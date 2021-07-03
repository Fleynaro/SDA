#include "SdaGoarBuilder.h"
#include <managers/TypeManager.h>

using namespace CE;
using namespace CE::Decompiler;
using namespace CE::Decompiler::ExprTree;
using namespace CE::Decompiler::Symbolization;

CE::Decompiler::Symbolization::SdaGoarBuilding::SdaGoarBuilding(UnknownLocation* unknownLocation, Project* project)
	: m_baseSdaNode(unknownLocation->getBaseSdaNode()), m_project(project), m_bitOffset(unknownLocation->getConstTermValue() * 0x8)
{
	for (auto term : unknownLocation->getArrTerms()) {
		m_sdaTerms.push_back(term.m_node);
	}
}

ISdaNode* CE::Decompiler::Symbolization::SdaGoarBuilding::create() {
	auto resultSdaNode = m_baseSdaNode;
	auto resultBitOffset = m_bitOffset;
	//building GOAR as long as it possible
	while (buildSingleGoar(resultSdaNode, resultBitOffset, m_sdaTerms));

	if (dynamic_cast<GoarNode*>(resultSdaNode)) {
		bool isPointer = m_baseSdaNode->getSrcDataType()->isPointer(); // getting a type without a cast (a pointer inside uint64_t)
		if (isPointer) {
			//if the base is a kind of pointer then remove & operation (and set it up later in the top of the built GOAR)
			if (auto addrGetting = dynamic_cast<IMappedToMemory*>(m_baseSdaNode)) {
				addrGetting->setAddrGetting(false); // (&player) + 0x10 -> &(player.pos.x)
			}
		}
		auto usedOffset = m_bitOffset - resultBitOffset;
		resultSdaNode = new GoarTopNode(resultSdaNode, usedOffset, isPointer);

		//if we have remaining either the offset or the array index terms
		if (resultBitOffset != 0x0 || !m_sdaTerms.empty()) {
			//remaining offset and terms (maybe only in case of node being as LinearExpr)
			auto numberType = m_project->getTypeManager()->getDefaultType(0x8);
			auto linearExpr = new LinearExpr(new SdaNumberLeaf(resultBitOffset / 0x8, numberType));
			linearExpr->addTerm(resultSdaNode);
			for (auto castTerm : m_sdaTerms) {
				linearExpr->addTerm(castTerm);
			}
			resultSdaNode = new UnknownLocation(linearExpr, 0);
		}
		return resultSdaNode;
	}
	return nullptr;
}

bool CE::Decompiler::Symbolization::SdaGoarBuilding::buildSingleGoar(ISdaNode*& sdaNode, int64_t& bitOffset, std::list<ISdaNode*>& terms) const
{
	auto dataType = sdaNode->getSrcDataType(); // getting a type without a cast (a pointer inside uint64_t)
	auto ptrLevels = dataType->getPointerLevels();
	auto baseDataType = dataType->getBaseType();

	//if it is a structure(Player, ...) or one-level pointer(Player*, float*, ...)
	if (ptrLevels.empty() || ptrLevels.size() == 1 && *ptrLevels.begin() == 1) {
		//try making a field
		if (auto structure = dynamic_cast<DataType::Structure*>(baseDataType)) {
			auto field = structure->getField((int)bitOffset);
			if (field->isDefault())
				return false;
			sdaNode = new GoarFieldNode(sdaNode, field);
			bitOffset -= field->getAbsBitOffset();
			return true;
		}

		//if no a pointer, a structure, an array(e.g. uint32_t) then END
		if (ptrLevels.empty())
			return false;
	}

	//if is a pointer(int*) or an array(int[2]) that supported addressing with [index] then try making an array
	//important: array is like a structure with stored items can be linearly addressed (an array item like a field)
	if (ptrLevels.size() >= 2 && *ptrLevels.begin() == 1 && *std::next(ptrLevels.begin()) != 1) {
		//in C++ NO declaration statements like this: int*[2][3] pArr;	(pointer to an array)
		ptrLevels.pop_front();
	}
	int arrItemsMaxCount = *ptrLevels.begin();
	ptrLevels.pop_front();
	auto arrItemDataType = DataType::GetUnit(baseDataType, ptrLevels);
	auto arrItemSize = arrItemDataType->getSize();

	// iterate over all {terms}
	ISdaNode* indexNode = nullptr;
	int indexSize = 0x4; //todo: long long(8 bytes) index?
	for (auto it = terms.begin(); it != terms.end(); it++) {
		auto sdaNode = *it;
		int64_t defMultiplier = 1;
		int64_t* multiplier = &defMultiplier;
		bool hasMultiplier = false;

		// try to get a multiplier 0x1000: players + idx * 0x1000 (arrItemSize = 0x1000)
		if (auto sdaGenTermNode = dynamic_cast<SdaGenericNode*>(sdaNode)) {
			if (auto opNode = dynamic_cast<OperationalNode*>(sdaGenTermNode->getNode())) {
				if (auto sdaNumberLeaf = dynamic_cast<SdaNumberLeaf*>(opNode->m_rightNode)) {
					if (opNode->m_operation == Mul) {
						multiplier = (int64_t*)&sdaNumberLeaf->m_value;
						hasMultiplier = true;
					}
				}
			}
		}

		if (*multiplier % arrItemSize == 0x0) {
			*multiplier /= arrItemSize;

			//optimization: remove operational node (add) (e.g. idx * 0x1 -> idx)
			if (*multiplier == 1 && hasMultiplier) {
				if (auto sdaGenTermNode = dynamic_cast<SdaGenericNode*>(sdaNode)) {
					if (auto opNode = dynamic_cast<OperationalNode*>(sdaGenTermNode->getNode())) {
						if (auto leftSdaNode = dynamic_cast<ISdaNode*>(opNode->m_leftNode)) {
							sdaGenTermNode->replaceWith(sdaNode = leftSdaNode);
							delete sdaGenTermNode;
						}
					}
				}
			}

			//players + idx * 0x1000 + idx2 * 0x2000 -> players[idx + idx2 * 2]
			if (indexNode) {
				auto indexNodeDataType = indexNode->getDataType();
				indexNode = new SdaGenericNode(new OperationalNode(indexNode, sdaNode, Add), indexNodeDataType); //todo: linear expr, another type
			}
			else {
				indexNode = sdaNode;
			}

			//remove the term because it in the index node
			terms.erase(it);
		}
	}

	//if we have some constant offset then try to insert it into the array indexer as index
	if (bitOffset != 0x0) {
		auto arrItemBitSize = arrItemSize * 0x8;
		auto constIndex = bitOffset / arrItemBitSize;
		if (constIndex != 0x0 || !indexNode) {
			bitOffset = bitOffset % arrItemBitSize;
			auto numberType = m_project->getTypeManager()->getDefaultType(indexSize); //need?
			auto constIndexNode = new SdaNumberLeaf(uint64_t(constIndex), numberType);

			//players[idx + idx2 * 2] -> players[idx + idx2 * 2 + 0x1000]
			if (indexNode) {
				auto indexNodeDataType = indexNode->getDataType();
				indexNode = new SdaGenericNode(new OperationalNode(indexNode, constIndexNode, Add), indexNodeDataType);
			}
			else {
				indexNode = constIndexNode;
			}
		}
	}

	// create an array node with the result index {idx + idx2 * 2 + 0x1000}
	if (indexNode) {
		sdaNode = new GoarArrayNode(sdaNode, indexNode, arrItemDataType, arrItemsMaxCount);
		return true;
	}
	return false;
}

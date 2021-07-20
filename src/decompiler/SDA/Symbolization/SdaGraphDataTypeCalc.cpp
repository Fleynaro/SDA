#include "SdaGraphDataTypeCalc.h"
#include "SdaGoarBuilder.h"
#include <managers/TypeManager.h>

using namespace CE;
using namespace Decompiler;
using namespace ExprTree;
using namespace Symbolization;

// calculate result data type for two operands

SdaDataTypesCalculater::SdaDataTypesCalculater(SdaCodeGraph* sdaCodeGraph, DataType::IFunctionSignature* signature, Project* project)
	: SdaGraphModification(sdaCodeGraph), m_signature(signature), m_project(project)
{}

void SdaDataTypesCalculater::start() {
	std::list<DecBlock::BlockTopNode*> allTopNodes;
	//gather all top nodes within the entire graph
	for (const auto decBlock : m_sdaCodeGraph->getDecGraph()->getDecompiledBlocks()) {
		auto list = decBlock->getAllTopNodes();
		allTopNodes.insert(allTopNodes.end(), list.begin(), list.end());
	}

	do {
		do {
			m_nextPassRequired = false;
			pass_up(allTopNodes);
		} while (m_nextPassRequired);
		pass_down(allTopNodes);
	} while (m_nextPassRequired);
}

//make a pass up through the specified top nodes

void SdaDataTypesCalculater::pass_up(const std::list<DecBlock::BlockTopNode*>& topNodes) {
	for (auto topNode : topNodes) {
		const auto node = topNode->getNode();
		UpdateDebugInfo(node);
		calculateDataTypes(node);

		//for return statement
		if (m_signature) {
			if (const auto returnTopNode = dynamic_cast<DecBlock::ReturnTopNode*>(topNode)) {
				if (const auto returnNode = dynamic_cast<ISdaNode*>(returnTopNode->getNode())) {
					const auto retDataType = m_signature->getReturnType();
					cast(returnNode, retDataType);
				}
			}
		}
	}
}

//make a pass down through the specified top nodes

void SdaDataTypesCalculater::pass_down(const std::list<DecBlock::BlockTopNode*>& topNodes) {
	for (auto topNode : topNodes) {
		const auto node = topNode->getNode();
		UpdateDebugInfo(node);
		moveExplicitCastsDown(node);
	}
}

void SdaDataTypesCalculater::moveExplicitCastsDown(INode* node) {
	auto sdaNode = dynamic_cast<ISdaNode*>(node);
	if (!sdaNode || !sdaNode->getCast()->hasExplicitCast())
		return;
	const auto castDataType = sdaNode->getCast()->getCastDataType();

	if (auto sdaGenNode = dynamic_cast<SdaGenericNode*>(sdaNode))
	{
		if (const auto opNode = dynamic_cast<OperationalNode*>(sdaGenNode->getNode())) {
			if (!IsOperationUnsupportedToCalculate(opNode->m_operation)
				&& opNode->m_operation != Concat && opNode->m_operation != Subpiece) {
				if (const auto sdaLeftSdaNode = dynamic_cast<ISdaNode*>(opNode->m_leftNode)) {
					if (const auto sdaRightSdaNode = dynamic_cast<ISdaNode*>(opNode->m_rightNode)) {
						cast(sdaLeftSdaNode, castDataType);
						cast(sdaRightSdaNode, castDataType);
						sdaGenNode->setDataType(castDataType);
						sdaNode->getCast()->clearCast();
					}
				}
			}
		}
	}

	// last iterate over all childs
	node->iterateChildNodes([&](INode* childNode) {
		moveExplicitCastsDown(childNode);
		});
}

void SdaDataTypesCalculater::calculateDataTypes(INode* node) {
	// first iterate over all childs
	node->iterateChildNodes([&](INode* childNode) {
		calculateDataTypes(childNode);
		});

	auto sdaNode = dynamic_cast<ISdaNode*>(node);
	if (!sdaNode)
		return;
	sdaNode->getCast()->clearCast();

	// method <cast> called for child nodes
	if (auto sdaGenNode = dynamic_cast<SdaGenericNode*>(sdaNode))
	{
		/*
		TODO: for operation >> and & of not-pointer object with size up to 8 bytes
		Create SUBPIECE node combined both >> and & operation to extract a field of a object
		Next call createGoar passing object and offset
		*/
		if (auto castNode = dynamic_cast<CastNode*>(sdaGenNode->getNode())) {
			if (auto srcSdaNode = dynamic_cast<ISdaNode*>(castNode->getNode())) {
				auto srcDataType = srcSdaNode->getDataType();
				auto srcBaseDataType = srcDataType->getBaseType();
				const auto castDataType = m_project->getTypeManager()->getDefaultType(castNode->getSize(), castNode->isSigned());
				sdaGenNode->setDataType(castDataType);
				if (srcDataType->isPointer() || castNode->isSigned() != srcBaseDataType->isSigned() || castNode->getSize() != srcBaseDataType->getSize()) {
					cast(srcSdaNode, castDataType);
				}
			}
		}
		else if (auto opNode = dynamic_cast<OperationalNode*>(sdaGenNode->getNode())) {
			if (!IsOperationUnsupportedToCalculate(opNode->m_operation)
				&& opNode->m_operation != Concat && opNode->m_operation != Subpiece) {
				if (auto sdaLeftSdaNode = dynamic_cast<ISdaNode*>(opNode->m_leftNode)) {
					if (auto sdaRightSdaNode = dynamic_cast<ISdaNode*>(opNode->m_rightNode)) {
						const DataTypePtr leftNodeDataType = sdaLeftSdaNode->getDataType();
						DataTypePtr rightNodeDataType;
						if (opNode->m_operation == Shr || opNode->m_operation == Shl) {
							rightNodeDataType = leftNodeDataType;
						}
						else {
							rightNodeDataType = sdaRightSdaNode->getDataType();
						}

						const auto opNodeSize = opNode->getSize();
						auto calcDataType = calcDataTypeForOperands(sdaLeftSdaNode->getDataType(), rightNodeDataType);
						if (opNode->isFloatingPoint()) { // floating operation used?
							calcDataType = calcDataTypeForOperands(calcDataType, m_project->getTypeManager()->getDefaultType(opNodeSize, true, true));
						}
						else {
							if (opNodeSize > calcDataType->getSize()) {
								// todo: print warning
								calcDataType = m_project->getTypeManager()->getDefaultType(opNodeSize);
							}
						}
						cast(sdaLeftSdaNode, calcDataType);
						cast(sdaRightSdaNode, calcDataType);
						sdaGenNode->setDataType(calcDataType);
					}
				}
			}
		}
		else if (auto linearExpr = dynamic_cast<LinearExpr*>(sdaGenNode->getNode())) { // or it is a pointer with offset, or it is some linear operation
			auto sdaConstTerm = dynamic_cast<SdaNumberLeaf*>(linearExpr->getConstTerm());
			DataTypePtr calcPointerDataType = sdaConstTerm->getDataType();

			/*
			 * Important info about {player + 0x10} where player has a {Player*} data type:
			 * Let access the field using 2 ways:
			 * 1) *(int*)(player + 0x10) - error!!! because {player + 0x10} == player[0x10]
			 * 2) *(int*)((uint64_t)player + 0x10) - correct
			 * So you need cast all terms into {uint64_t}, no {Player*}
			 */

			//finding a pointer among terms (base term)
			ISdaNode* sdaPointerNode = nullptr; // it is a pointer
			int sdaPointerNodeIdx = 0;
			int idx = 0;
			for (auto term : linearExpr->getTerms()) {
				if (auto sdaTermNode = dynamic_cast<ISdaNode*>(term)) {
					if (sdaTermNode->getDataType()->isPointer()) {
						sdaPointerNode = sdaTermNode;
						sdaPointerNodeIdx = idx;
						calcPointerDataType = m_project->getTypeManager()->getDefaultType(0x8);
						break;
					}
					calcPointerDataType = calcDataTypeForOperands(calcPointerDataType, sdaTermNode->getDataType());
				}
				idx++;
			}
			
			//set the default data type (usually size of 8 bytes) for all terms (including the base)
			cast(sdaConstTerm, calcPointerDataType);
			for (auto termNode : linearExpr->getTerms()) {
				if (const auto sdaTermNode = dynamic_cast<ISdaNode*>(termNode)) {
					cast(sdaTermNode, calcPointerDataType);
				}
			}

			//if we figure out a pointer then we guarantee it is always some unk location
			if (sdaPointerNode) {
				const auto unknownLocation = new UnknownLocation(linearExpr, sdaPointerNodeIdx); //wrap LinearExpr 
				linearExpr->addParentNode(unknownLocation);
				sdaGenNode->replaceWith(unknownLocation);
				delete sdaGenNode;

				// should be (float*)((uint64_t)param1 + 0x10)
				//cast(unknownLocation, sdaPointerNode->getDataType());
				//then build a goar or anything
				handleUnknownLocation(unknownLocation);
			}
			else {
				// not a pointer, just some linear operation
				sdaGenNode->setDataType(calcPointerDataType);
			}
		}
		else if (const auto assignmentNode = dynamic_cast<AssignmentNode*>(sdaGenNode->getNode())) {
			if (auto dstSdaNode = dynamic_cast<ISdaNode*>(assignmentNode->getDstNode())) {
				if (auto srcSdaNode = dynamic_cast<ISdaNode*>(assignmentNode->getSrcNode())) {
					auto dstNodeDataType = dstSdaNode->getDataType();
					auto srcNodeDataType = srcSdaNode->getDataType();

					if (dstNodeDataType->getSize() == srcNodeDataType->getSize() && dstNodeDataType->getPriority() < srcNodeDataType->getPriority()) {
						cast(dstSdaNode, srcNodeDataType);
						dstSdaNode->getCast()->clearCast();
						dstNodeDataType = dstSdaNode->getDataType();
					}

					cast(srcSdaNode, dstNodeDataType);
					sdaGenNode->setDataType(dstNodeDataType);
				}
			}
		}
		else if (auto condNode = dynamic_cast<AbstractCondition*>(sdaGenNode->getNode())) {
			// any condition returns BOOLEAN value
			const auto boolType = m_project->getTypeManager()->getType(DataType::SystemType::Bool);
			sdaGenNode->setDataType(boolType);
		}
	}
	else if (auto sdaReadValueNode = dynamic_cast<SdaReadValueNode*>(sdaNode)) {
		// example: *(float*)(&globalVar)
		auto addrSdaNode = sdaReadValueNode->getAddress();
		if (addrSdaNode->getDataType()->isPointer()) { // &globalVar is a pointer (type: float*)
			auto addrDataType = CloneUnit(addrSdaNode->getDataType());
			addrDataType->removePointerLevelOutOfFront(); // float*(8 bytes) -> float(4 bytes)
			if (sdaReadValueNode->getSize() == addrDataType->getSize()) {
				if (auto mappedToMemory = dynamic_cast<IMappedToMemory*>(addrSdaNode)) {
					if (mappedToMemory->isAddrGetting()) {
						// *(float*)(&globalVar) -> globalVar
						mappedToMemory->setAddrGetting(false); // &globalVar -> globalVar
						sdaReadValueNode->replaceWith(mappedToMemory);
						delete sdaReadValueNode;
						return;
					}
				}

				//*(float*)(&globalVar) have to return a value of <float> type
				sdaReadValueNode->setDataType(addrDataType);
				return;
			}
		}

		// cast &globalVar/stackVar/0x1000 to default type uint32_t* (because reading of 4 bytes)
		const auto defDataType = sdaReadValueNode->getDataType(); // any sda node have already had a default type
		auto defPtrDataType = CloneUnit(defDataType);
		defPtrDataType->addPointerLevelInFront();
		cast(addrSdaNode, defPtrDataType);
	}
	else if (const auto sdaFunctionNode = dynamic_cast<SdaFunctionNode*>(sdaNode)) {
		handleFunctionNode(sdaFunctionNode);
	}
	else if (auto sdaSymbolLeaf = dynamic_cast<SdaSymbolLeaf*>(sdaNode)) {
		// example: *(float*)(param1) where <param1> is <float*>
		if (sdaSymbolLeaf->getDataType()->isPointer()) {
			const auto g = sdaSymbolLeaf->getDataType()->getGroup();
			if (g == DataType::AbstractType::Structure || g == DataType::AbstractType::Class) {
				if (dynamic_cast<ReadValueNode*>(sdaSymbolLeaf->getParentNode())) {
					// just add offset: *(float*)(param1) -> *(float*)(param1 + 0x0)
					auto linearExpr = new LinearExpr(new SdaNumberLeaf(0));
					const auto unknownLocation = new UnknownLocation(linearExpr, 0);
					linearExpr->addParentNode(unknownLocation);
					sdaSymbolLeaf->replaceWith(unknownLocation);
					linearExpr->addTerm(sdaSymbolLeaf);
					//then build a goar or anything
					handleUnknownLocation(unknownLocation);
				}
			}
			// why a symbol only? Because no cases when (param1->field_1)->field_2 as memVar exists.
		}
	}
	else if (auto sdaNumberLeaf = dynamic_cast<SdaNumberLeaf*>(sdaNode)) {
		sdaNumberLeaf->setDataType(m_project->getTypeManager()->calcDataTypeForNumber(sdaNumberLeaf->m_value));
	}
	else if (auto goarNode = dynamic_cast<GoarNode*>(sdaNode)) {
		//...
		return;
	}
	else if (const auto unknownLocation = dynamic_cast<UnknownLocation*>(sdaNode)) {
		handleUnknownLocation(unknownLocation);
	}
}

void SdaDataTypesCalculater::handleFunctionNode(SdaFunctionNode* sdaFunctionNode) {
	auto funcSignature = sdaFunctionNode->getSignature();
	if (funcSignature)
	{
		// iterate over all params of the signature
		const auto paramsCount = static_cast<int>(std::min(funcSignature->getParameters().size(), sdaFunctionNode->getParamNodes().size()));
		for (int paramIdx = 0; paramIdx < paramsCount; paramIdx++) {
			const auto paramNode = sdaFunctionNode->getParamNodes()[paramIdx];
			if (auto paramSdaNode = dynamic_cast<ISdaNode*>(paramNode)) {
				auto funcParamSymbol = funcSignature->getParameters()[paramIdx];
				auto sigDataType = funcParamSymbol->getDataType();
				auto nodeDataType = paramSdaNode->getDataType();

				// or a type of the node, or a type of the sig parameter
				if (funcParamSymbol->isAutoSymbol() && nodeDataType->getPriority() > sigDataType->getPriority()) {
					// change a type of the parameter symbol
					funcParamSymbol->setDataType(nodeDataType);
					onDataTypeCasting(sigDataType, nodeDataType);
				}
				else {
					if (nodeDataType->getPriority() < sigDataType->getPriority()) {
						cast(paramSdaNode, sigDataType);
					}
				}
			}
			paramIdx++;
		}
	}
}

void SdaDataTypesCalculater::handleUnknownLocation(UnknownLocation* unknownLocation) {
	//if it is a pointer, see to make sure it could'be transformed to an array or a class field
	if (!dynamic_cast<GoarTopNode*>(unknownLocation->getBaseSdaNode())) {
		if (const auto goarNode = SdaGoarBuilding(unknownLocation, m_project).create()) {
			unknownLocation->replaceWith(goarNode);
			delete unknownLocation;
		}
	}
}

void SdaDataTypesCalculater::onDataTypeCasting(DataTypePtr fromDataType, DataTypePtr toDataType) {
}

// casting {sdaNode} to {toDataType}

void SdaDataTypesCalculater::cast(ISdaNode* sdaNode, DataTypePtr toDataType) {
	//exception case (better change number view between HEX and non-HEX than do the cast)
	if (auto sdaNumberLeaf = dynamic_cast<SdaNumberLeaf*>(sdaNode)) {
		if (!toDataType->isPointer()) {
			if (toDataType->getSize() >= sdaNumberLeaf->getDataType()->getSize()) {
				sdaNumberLeaf->setDataType(toDataType);
				return;
			}
		}
	}

	//CASTING
	const auto fromDataType = sdaNode->getSrcDataType();
	const auto explicitCast = isExplicitCast(fromDataType, toDataType);
	onDataTypeCasting(fromDataType, toDataType);
	sdaNode->getCast()->setCastDataType(toDataType, explicitCast);

	// for AUTO sda symbols that have to acquire a data type with the biggest priority (e.g. uint64_t -> Player*)
	if (auto sdaSymbolLeaf = dynamic_cast<SdaSymbolLeaf*>(sdaNode)) {
		if (sdaSymbolLeaf->getSdaSymbol()->isAutoSymbol()) {
			auto symbolDataType = sdaSymbolLeaf->getSdaSymbol()->getDataType();
			if (symbolDataType->getSize() == toDataType->getSize() && symbolDataType->getPriority() < toDataType->getPriority()) {
				sdaSymbolLeaf->setDataType(toDataType);
				m_nextPassRequired = true;
			}
		}
	}
	// *(uint32_t*)(p + 4) -> *(float*)(p + 4)
	else if (auto sdaReadValueNode = dynamic_cast<SdaReadValueNode*>(sdaNode)) {
		if (sdaReadValueNode->getSize() == toDataType->getSize()) {
			const auto addrSdaNode = sdaReadValueNode->getAddress();
			auto newAddrDataType = CloneUnit(toDataType);
			newAddrDataType->addPointerLevelInFront();

			cast(addrSdaNode, newAddrDataType);
			sdaNode->setDataType(toDataType);
		}
	}
}

// does it need explicit casting (e.g. (float)0x100024)

bool SdaDataTypesCalculater::isExplicitCast(DataTypePtr fromType, DataTypePtr toType) {
	auto fromBaseType = fromType->getBaseType();
	auto toBaseType = toType->getBaseType();
	if (auto fromSysType = dynamic_cast<DataType::SystemType*>(fromBaseType)) {
		if (auto toSysType = dynamic_cast<DataType::SystemType*>(toBaseType)) {
			if (fromSysType->isSigned() != toSysType->isSigned())
				return true;
			if (fromBaseType->getSize() > toBaseType->getSize())
				return true;
		}
	}
	const auto ptrList1 = fromType->getPointerLevels();
	const auto ptrList2 = toType->getPointerLevels();
	if (ptrList1.empty() && ptrList2.empty())
		return false;
	if (fromBaseType != toBaseType)
		return true;
	return !DataType::Unit::EqualPointerLvls(ptrList1, ptrList2);
}

DataTypePtr SdaDataTypesCalculater::calcDataTypeForOperands(DataTypePtr opType1, DataTypePtr opType2) const
{
	const auto priority1 = opType1->getConversionPriority();
	const auto priority2 = opType2->getConversionPriority();
	if (priority1 == 0 && priority2 == 0)
		return m_project->getTypeManager()->getType(DataType::SystemType::Int32);
	if (priority2 > priority1)
		return opType2;
	return opType1;
}

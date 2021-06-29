#pragma once
#include "ExprTreeSdaNode.h"

namespace CE::Decompiler::ExprTree
{
	// *(float*)&globalVar (where *(float*) is reader from the specified mem. location and &globalVar is named address)
	class SdaReadValueNode : public SdaNode, public INodeAgregator, public PCode::IRelatedToInstruction, public IMappedToMemory
	{
	public:
		ReadValueNode* m_readValueNode;
		DataTypePtr m_outDataType;

		SdaReadValueNode(ReadValueNode* readValueNode, DataTypePtr outDataType)
			: m_readValueNode(readValueNode), m_outDataType(outDataType)
		{}

		~SdaReadValueNode() {
			m_readValueNode->removeBy(this);
		}

		ISdaNode* getAddress() {
			return dynamic_cast<ISdaNode*>(m_readValueNode->getAddress());
		}

		void replaceNode(ExprTree::INode* node, ExprTree::INode* newNode) override {
			if (node == m_readValueNode)
				m_readValueNode = dynamic_cast<ReadValueNode*>(newNode);
		}

		std::list<INode*> getNodesList() override {
			return m_readValueNode->getNodesList();
		}

		std::list<PCode::Instruction*> getInstructionsRelatedTo() override {
			return m_readValueNode->getInstructionsRelatedTo();
		}

		int getSize() override {
			return m_readValueNode->getSize();
		}

		HS getHash() override {
			return m_readValueNode->getHash(); //todo: + term hashes
		}

		ISdaNode* cloneSdaNode(NodeCloneContext* ctx) override {
			auto clonedReadValueNode = dynamic_cast<ReadValueNode*>(m_readValueNode->clone(ctx));
			auto sdaReadValueNode = new SdaReadValueNode(clonedReadValueNode, CloneUnit(m_outDataType));
			clonedReadValueNode->addParentNode(sdaReadValueNode);
			return sdaReadValueNode;
		}

		DataTypePtr getSrcDataType() override {
			return m_outDataType;
		}

		void setDataType(DataTypePtr dataType) override {
			m_outDataType = dataType;
		}

		bool isAddrGetting() override {
			return false;
		}

		void setAddrGetting(bool toggle) override {
		}

		void getLocation(MemLocation& location) override {
			if (auto locatableAddrNode = dynamic_cast<ILocatable*>(getAddress())) {
				locatableAddrNode->getLocation(location);
				location.m_valueSize = getSize();
				return;
			}
			else {
				ISdaNode* sdaAddrNode = nullptr;
				int64_t offset = 0x0;
				if (auto symbolLeaf = dynamic_cast<SdaSymbolLeaf*>(getAddress())) {
					sdaAddrNode = symbolLeaf;
				}
				else if (auto sdaGenNode = dynamic_cast<SdaGenericNode*>(getAddress())) {
					if (auto linearExpr = dynamic_cast<LinearExpr*>(sdaGenNode->getNode())) {
						if (linearExpr->getTerms().size() == 1) {
							if(sdaAddrNode = dynamic_cast<ISdaNode*>(*linearExpr->getTerms().begin())) {
								offset = linearExpr->getConstTermValue();
							}
						}
					}
				}

				if (sdaAddrNode && sdaAddrNode->getSrcDataType()->getSize() == 0x8) {
					location.m_type = MemLocation::IMPLICIT;
					location.m_baseAddrHash = sdaAddrNode->getHash();
					location.m_offset = offset;
					location.m_valueSize = getSize();
					return;
				}
			}
			throw std::exception("impossible to determine the location");
		}

		std::string printSdaDebug() override {
			auto result = "*" + getAddress()->printDebug();
			return result;
		}
	};
};
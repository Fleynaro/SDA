#pragma once
#include "datatypes/SystemType.h"
#include "ExprTree/ExprTreeAssignmentNode.h"
#include "ExprTree/ExprTreeCondition.h"
#include "ExprTree/ExprTreeFunctionCall.h"
#include "ExprTree/ExprTreeLinearExpr.h"
#include "ExprTree/ExprTreeMirrorNode.h"
#include "ExprTree/ExprTreeNode.h"
#include "ExprTree/ExprTreeOperationalNode.h"
#include "LinearView/DecLinearViewBlocks.h"
#include "SDA/SdaCodeGraph.h"
#include "SDA/ExprTree/ExprTreeSdaFunction.h"
#include "SDA/ExprTree/ExprTreeSdaGenericNode.h"
#include "SDA/ExprTree/ExprTreeSdaGoar.h"
#include "SDA/ExprTree/ExprTreeSdaLeaf.h"
#include "SDA/ExprTree/ExprTreeSdaNode.h"
#include "SDA/ExprTree/ExprTreeSdaReadValue.h"
#include "SDA/ExprTree/ExprTreeSdaUnkLocation.h"

namespace CE::Decompiler
{
	namespace ExprTree
	{
		class ExprTreeViewGenerator
		{
		public:
			enum TokenType
			{
				TOKEN_OPERATION,		// +/*/%
				TOKEN_ROUND_BRACKET,	// ()
				TOKEN_FUNCTION_CALL,	// func()
				TOKEN_DATA_TYPE,		// float/int64_t
				TOKEN_DEC_SYMBOL,
				TOKEN_SDA_SYMBOL,
				TOKEN_NUMBER,
				TOKEN_DEBUG_INFO,
				TOKEN_OTHER
			};
			
			bool m_debugMode = false;
			bool m_markSdaNodes = false;

			virtual void generate(INode* node) {
				generateNode(node);
			}
			
			virtual void generateNode(INode* node) {
				if (const auto sdaNode = dynamic_cast<ISdaNode*>(node)) {
					if (m_debugMode && m_markSdaNodes) {
						generateToken("@", TOKEN_DEBUG_INFO);
					}
					
					if (sdaNode->hasCast() && sdaNode->getCast()->hasExplicitCast()) {
						const auto castDataType = sdaNode->getCast()->getCastDataType();
						const auto bracketId = reinterpret_cast<uint8_t*>(sdaNode) + 1;
						if (castDataType->isPointer() || sdaNode->getSrcDataType()->isPointer() && castDataType->isSystem()) {
							generateRoundBracket("(", bracketId);
							generateDataType(castDataType);
							generateRoundBracket(")", bracketId);
							generateSdaNodeWithAddrGetting(sdaNode);
						} else {
							generateToken("to", TOKEN_FUNCTION_CALL);
							generateToken("<", TOKEN_OTHER);
							generateDataType(castDataType);
							generateToken(">", TOKEN_OTHER);
							generateRoundBracket("(", bracketId);
							generateSdaNodeWithAddrGetting(sdaNode);
							generateRoundBracket(")", bracketId);
						}
					} else {
						generateSdaNodeWithAddrGetting(sdaNode);
					}
				}
				else if(const auto assignmentNode = dynamic_cast<AssignmentNode*>(node)) {
					generateAssignmentNode(assignmentNode);
				}
				else if (const auto opNode = dynamic_cast<OperationalNode*>(node)) {
					if (const auto readValueNode = dynamic_cast<ReadValueNode*>(node)) {
						generateReadValueNode(readValueNode);
					}
					else if (const auto castNode = dynamic_cast<CastNode*>(node)) {
						generateCastNode(castNode);
					}
					else if (const auto functionalNode = dynamic_cast<FunctionalNode*>(node)) {
						generateFunctionalNode(functionalNode);
					}
					else if (const auto floatFunctionalNode = dynamic_cast<FloatFunctionalNode*>(node)) {
						generateFloatFunctionalNode(floatFunctionalNode);
					}
					else {
						generateOperationalNode(opNode);
					}
				}
				else if (const auto functionCall = dynamic_cast<FunctionCall*>(node)) {
					generateFunctionCall(functionCall);
				}
				else if (const auto symbolLeaf = dynamic_cast<SymbolLeaf*>(node)) {
					generateSymbolLeaf(symbolLeaf);
				}
				else if (const auto numberLeaf = dynamic_cast<NumberLeaf*>(node)) {
					generateNumberLeaf(numberLeaf);
				}
				else if (const auto floatNanLeaf = dynamic_cast<FloatNanLeaf*>(node)) {
					generateFloatNanLeaf(floatNanLeaf);
				}
				else if (const auto linearExpr = dynamic_cast<LinearExpr*>(node)) {
					generateLinearExpr(linearExpr);
				}
				else if (const auto mirrorNode = dynamic_cast<MirrorNode*>(node)) {
					generateMirrorNode(mirrorNode);
				}
				else if (const auto condNode = dynamic_cast<AbstractCondition*>(node)) {
					if (const auto booleanValue = dynamic_cast<BooleanValue*>(node)) {
						generateBooleanValue(booleanValue);
					}
					else if (const auto simpleCondNode = dynamic_cast<Condition*>(node)) {
						generateSimpleCondition(simpleCondNode);
					}
					else if (const auto compositeCondNode = dynamic_cast<CompositeCondition*>(node)) {
						generateCompositeCondition(compositeCondNode);
					}
				}
			}

			virtual void generateSdaNodeWithAddrGetting(ISdaNode* node) {
				if (const auto memoryNode = dynamic_cast<IMappedToMemory*>(node)) {
					bool isAddrGetting = memoryNode->isAddrGetting();
					if (isAddrGetting) {
						if (const auto funcCallNode = dynamic_cast<FunctionCall*>(node->getParentNode()))
							isAddrGetting = funcCallNode->getDestination() != node;
					}
					if (isAddrGetting)
						generateToken("&", TOKEN_OPERATION);
				}
				generateSdaNode(node);
			}

			virtual void generateSdaNode(ISdaNode* node) {
				if (const auto sdaGenericNode = dynamic_cast<SdaGenericNode*>(node)) {
					INode* childNode = nullptr;
					if (const auto castNode = dynamic_cast<CastNode*>(sdaGenericNode->getNode())) {
						childNode = castNode->getNode();
					}
					else if (const auto floatFuncNode = dynamic_cast<FloatFunctionalNode*>(sdaGenericNode->getNode())) {
						if (floatFuncNode->m_funcId == FloatFunctionalNode::Id::TOINT || floatFuncNode->m_funcId == FloatFunctionalNode::Id::TOFLOAT) {
							childNode = floatFuncNode->getNode();
						}
					}
					
					if (childNode) {
						const auto bracketId = reinterpret_cast<uint8_t*>(node) + 1;
						generateDataType(sdaGenericNode->getSrcDataType());
						generateRoundBracket("(", bracketId);
						generateNode(childNode);
						generateRoundBracket(")", bracketId);
					} else {
						generateNode(sdaGenericNode->getNode());
					}
				}
				else if (const auto sdaReadValueNode = dynamic_cast<SdaReadValueNode*>(node)) {
					generateSdaReadValueNode(sdaReadValueNode);
				}
				else if (const auto sdaFunctionNode = dynamic_cast<SdaFunctionNode*>(node)) {
					generateSdaFunctionNode(sdaFunctionNode);
				}
				else if (const auto unknownLocation = dynamic_cast<UnknownLocation*>(node)) {
					generateUnknownLocation(unknownLocation);
				}
				else if (const auto goarNode = dynamic_cast<GoarNode*>(node)) {
					if (const auto goarArrayNode = dynamic_cast<GoarArrayNode*>(goarNode)) {
						generateGoarArrayNode(goarArrayNode);
					}
					else if (const auto goarFieldNode = dynamic_cast<GoarFieldNode*>(goarNode)) {
						generateGoarFieldNode(goarFieldNode);
					}
					else if (const auto goarTopNode = dynamic_cast<GoarTopNode*>(goarNode)) {
						generateGoarTopNode(goarTopNode);
					}
				}
				else if (const auto sdaSymbolLeaf = dynamic_cast<SdaSymbolLeaf*>(node)) {
					generateSdaSymbolLeaf(sdaSymbolLeaf);
				}
				else if (const auto sdaNumberLeaf = dynamic_cast<SdaNumberLeaf*>(node)) {
					generateSdaNumberLeaf(sdaNumberLeaf);
				}
			}

			virtual void generateSdaReadValueNode(SdaReadValueNode* node) {
				generateToken("*", TOKEN_DATA_TYPE);
				generateNode(node->getAddress());
			}

			virtual void generateSdaFunctionNode(SdaFunctionNode* node) {
				generateNode(node->m_funcCall);
			}
			
			virtual void generateUnknownLocation(UnknownLocation* node) {
				generateNode(node->m_linearExpr);
			}

			virtual void generateGoarArrayNode(GoarArrayNode* node) {
				generateNode(node->m_base);
				generateToken("[", TOKEN_OPERATION);
				generateNode(node->m_indexNode);
				generateToken("]", TOKEN_OPERATION);
			}

			virtual void generateGoarFieldNode(GoarFieldNode* node) {
				const auto isPointer = node->m_base->getDataType()->isPointer();
				generateNode(node->m_base);
				generateToken(isPointer ? "->" : ".", TOKEN_OPERATION);
				generateSdaSymbol(node->m_field);
			}

			virtual void generateGoarTopNode(GoarTopNode* node) {
				generateNode(node->m_base);
			}

			virtual void generateSdaSymbolLeaf(SdaSymbolLeaf* leaf) {
				generateSdaSymbol(leaf->getSdaSymbol());
			}

			virtual void generateSdaNumberLeaf(SdaNumberLeaf* leaf) {
				std::string number = "0x" + Helper::String::NumberToHex(leaf->m_value);
				if (leaf->getSrcDataType()->isFloatingPoint()) {
					if (leaf->getSrcDataType()->getSize() == 4)
						number = std::to_string(reinterpret_cast<float&>(leaf->m_value));
					else number = std::to_string(reinterpret_cast<double&>(leaf->m_value));
				}
				else if (auto sysType = dynamic_cast<DataType::SystemType*>(leaf->getSrcDataType()->getBaseType())) {
					if (sysType->isSigned()) {
						const auto size = leaf->getSrcDataType()->getSize();
						if (size <= 4)
							number = std::to_string(static_cast<int32_t>(leaf->m_value));
						else
							number = std::to_string(static_cast<int64_t>(leaf->m_value));
					}
				}
				generateToken(number, TOKEN_NUMBER);
			}

			virtual void generateAssignmentNode(const AssignmentNode* node) {
				if (!node->m_isSrcOnly) {
					generateNode(node->getDstNode());
					generateToken(" = ", TOKEN_OPERATION);
				}
				generateNode(node->getSrcNode());
			}

			static bool OperationPriority(OperationType opType, OperationType parentOpType) {
				/*
				 * let {opType} = +, {parentOpType} = *
				 * (X + 0.5f) * Y	- round brackets
				 * X + 0.5f * Y		- no brackets
				 */

				if (opType == parentOpType && IsOperationMoving(opType))
					return false;
				
				std::map priorities = {
					std::pair(Mul, 13), std::pair(Div, 13), std::pair(Mod, 13), std::pair(fMul, 13), std::pair(fDiv, 13),
					std::pair(Add, 12), std::pair(fAdd, 12),
					std::pair(Shr, 11), std::pair(Shl, 11),
					std::pair(And, 8),
					std::pair(Xor, 7),
					std::pair(Or, 6),
					std::pair(Functional, 0), std::pair(fFunctional, 0)
				};
				const auto it1 = priorities.find(opType);
				const auto it2 = priorities.find(parentOpType);
				if (it1 != priorities.end() && it2 != priorities.end()) {
					return it1->second <= it2->second;
				}
				return true;
			}

			static bool DoesNeedRoundBrackets(IOperation* node) {
				if (node->getParentNodes().size() != 1)
					return true;

				/*
				 * return X + 0.5f;		- no brackets
				 * Y = X + 0.5f;		- no brackets
				 */
				bool needBrackets = true;
				if (const auto sdaParent = dynamic_cast<SdaGenericNode*>(node->getParentNode())) {
					if (sdaParent->hasCast() && sdaParent->getCast()->hasExplicitCast())
						return true;
					if ((needBrackets = !dynamic_cast<DecBlock::BlockTopNode*>(sdaParent->getParentNode()))) {
						if ((needBrackets = !dynamic_cast<AssignmentNode*>(sdaParent->getParentNode()))) {
							if ((needBrackets = !dynamic_cast<FunctionCall*>(sdaParent->getParentNode()))) {
								if (const auto parentOpNode = dynamic_cast<IOperation*>(sdaParent->getParentNode())) {
									needBrackets &= OperationPriority(node->getOperation(), parentOpNode->getOperation());
								}
							}
						}
					}
				}
				return needBrackets;
			}

			virtual void generateOperationalNode(OperationalNode* node) {
				if (!node->m_leftNode || !node->m_rightNode)
					return;
				const auto opSize = GetOperationSize(node->getSize(), node->isFloatingPoint());

				if (node->m_operation == Concat) {
					generateToken("CONCAT", TOKEN_FUNCTION_CALL);
					if(m_debugMode) {
						generateToken("<" + opSize + ">", TOKEN_DEBUG_INFO);
					}
					generateRoundBracket("(", node);
					generateNode(node->m_leftNode);
					generateToken(", ", TOKEN_OTHER);
					generateNode(node->m_rightNode);
					generateToken(", ", TOKEN_OTHER);
					generateToken(std::to_string(node->m_rightNode->getSize() * 0x8), TOKEN_OTHER);
					generateRoundBracket(")", node);
					return;
				}

				if (node->m_operation == Xor) {
					if (const auto numberLeaf = dynamic_cast<INumberLeaf*>(node->m_rightNode)) {
						if (numberLeaf->getValue() == static_cast<uint64_t>(-1)) {
							generateToken("~", TOKEN_OTHER);
							generateNode(numberLeaf);
						}
					}
					return;
				}

				const auto operation = GetOperation(node->m_operation);
				const auto needBrackets = DoesNeedRoundBrackets(node);
				
				if (needBrackets)
					generateRoundBracket("(", node);
				generateNode(node->m_leftNode);
				generateToken(" ", TOKEN_OTHER);
				generateToken(operation, TOKEN_OPERATION);
				if(m_debugMode) {
					generateToken(opSize, TOKEN_DEBUG_INFO);
				}
				generateToken(" ", TOKEN_OTHER);
				generateNode(node->m_rightNode);
				if (needBrackets)
					generateRoundBracket(")", node);
			}

			virtual void generateReadValueNode(ReadValueNode* node) {
				if (!node->m_leftNode)
					return;
				const auto token = "*{uint_" + std::to_string(8 * node->getSize()) + "t*}";
				generateToken(token, TOKEN_OTHER);
				generateNode(node->m_leftNode);
			}

			virtual void generateCastNode(CastNode* node) {
				if (!node->m_leftNode)
					return;
				const auto token = "{" + std::string(!node->m_isSigned ? "u" : "") + "int_" + std::to_string(8 * node->getSize()) + "t}";
				generateToken(token, TOKEN_OTHER);
				generateNode(node->m_leftNode);
			}

			virtual void generateFunctionalNode(FunctionalNode* node) {
				if (!node->m_leftNode || !node->m_rightNode)
					return;
				generateToken(magic_enum::enum_name(node->m_funcId).data(), TOKEN_FUNCTION_CALL);
				generateRoundBracket("(", node);
				generateNode(node->m_leftNode);
				generateToken(", ", TOKEN_OTHER);
				generateNode(node->m_rightNode);
				generateRoundBracket(")", node);
			}

			virtual void generateFloatFunctionalNode(FloatFunctionalNode* node) {
				if (!node->m_leftNode)
					return;
				generateToken(magic_enum::enum_name(node->m_funcId).data(), TOKEN_FUNCTION_CALL);
				generateRoundBracket("(", node);
				generateNode(node->m_leftNode);
				generateRoundBracket(")", node);
			}

			virtual void generateLinearExpr(LinearExpr* node) {
				const auto operation = GetOperation(node->m_operation);
				const auto opSize = GetOperationSize(node->getSize(), node->isFloatingPoint());

				const auto needBrackets = DoesNeedRoundBrackets(node);
				if(needBrackets)
					generateRoundBracket("(", node);
				for (auto it = node->m_terms.begin(); it != node->m_terms.end(); ++it) {
					generateNode(*it);
					if (it != std::prev(node->m_terms.end()) || node->m_constTerm->getValue()) {
						generateToken(" ", TOKEN_OTHER);
						generateToken(operation, TOKEN_OPERATION);
						if (m_debugMode) {
							generateToken(opSize, TOKEN_DEBUG_INFO);
						}
						generateToken(" ", TOKEN_OTHER);
					}
				}

				if (node->m_constTerm->getValue()) {
					generateNode(node->m_constTerm);
				}
				if (needBrackets)
					generateRoundBracket(")", node);
			}

			virtual void generateFunctionCall(FunctionCall* node) {
				bool needBrackets = false;
				if (const auto sdaNode = dynamic_cast<ISdaNode*>(node->getDestination())) {
					// ((uint64_t)vt->f2)(5);
					if (sdaNode->hasCast() && sdaNode->getCast()->hasExplicitCast()) {
						needBrackets = true;
					}
				}

				if(needBrackets)
					generateRoundBracket("(", node);
				generateNode(node->getDestination());
				if (needBrackets)
					generateRoundBracket(")", node);
				
				generateRoundBracket("(", node);
				for (auto it = node->m_paramNodes.begin(); it != node->m_paramNodes.end(); ++it) {
					generateNode(*it);
					if(it != std::prev(node->m_paramNodes.end()))
						generateToken(", ", TOKEN_OTHER);
				}
				generateRoundBracket(")", node);
			}

			virtual void generateSymbolLeaf(SymbolLeaf* leaf) {
				generateDecSymbol(leaf->m_symbol);
			}

			virtual void generateNumberLeaf(NumberLeaf* leaf) {
				const auto token = "0x" + Helper::String::NumberToHex(leaf->m_value);
				generateToken(token, TOKEN_NUMBER);
				if (m_debugMode) {
					generateToken("{" + (std::to_string(static_cast<int>(leaf->m_value))) + "}", TOKEN_DEBUG_INFO);
				}
			}

			virtual void generateFloatNanLeaf(FloatNanLeaf* leaf) {
				generateToken("NaN", TOKEN_NUMBER);
			}

			virtual void generateMirrorNode(MirrorNode* node) {
				generateNode(node->m_node);
			}

			static bool DoesNeedRoundBrackets(AbstractCondition* node) {
				// todo: no good to count parents
				return node->getParentNodes().size() != 1 || dynamic_cast<DecBlock::JumpTopNode*>(node->getParentNode());
			}

			virtual void generateBooleanValue(BooleanValue* node) {
				const auto needBrackets = DoesNeedRoundBrackets(node);
				if (needBrackets)
					generateRoundBracket("(", node);
				generateToken(node->m_value ? "true" : "false", TOKEN_NUMBER);
				if (needBrackets)
					generateRoundBracket(")", node);
			}

			virtual void generateSimpleCondition(Condition* node) {
				if (!node->m_leftNode || !node->m_rightNode)
					return;
				const auto needBrackets = DoesNeedRoundBrackets(node);
				if (needBrackets)
					generateRoundBracket("(", node);
				generateNode(node->m_leftNode);
				generateToken(" ", TOKEN_OTHER);
				generateToken(GetConditionType(node->m_cond), TOKEN_OPERATION);
				generateToken(" ", TOKEN_OTHER);
				generateNode(node->m_rightNode);
				if (needBrackets)
					generateRoundBracket(")", node);
			}

			virtual void generateCompositeCondition(CompositeCondition* node) {
				if (!node->m_leftCond)
					return;

				const auto needBrackets = DoesNeedRoundBrackets(node);
				if (node->m_cond == CompositeCondition::None) {
					if(needBrackets)
						generateRoundBracket("(", node);
					generateNode(node->m_leftCond);
					if (needBrackets)
						generateRoundBracket(")", node);
					return;
				}
				
				if (node->m_cond == CompositeCondition::Not) {
					if (needBrackets)
						generateRoundBracket("(", node);
					generateToken("!", TOKEN_OTHER);
					generateRoundBracket("(", node);
					generateNode(node->m_leftCond);
					generateRoundBracket(")", node);
					if (needBrackets)
						generateRoundBracket(")", node);
					return;
				}

				generateRoundBracket("(", node);
				generateNode(node->m_leftCond);
				generateToken(" ", TOKEN_OTHER);
				generateToken(GetCompConditionType(node->m_cond), TOKEN_OPERATION);
				generateToken(" ", TOKEN_OTHER);
				generateNode(node->m_rightCond);
				generateRoundBracket(")", node);
			}

			virtual void generateDecSymbol(Symbol::Symbol* symbol) {
				if (const auto registerVariable = dynamic_cast<Symbol::RegisterVariable*>(symbol)) {
					const auto token = "[reg_" + PCode::InstructionViewGenerator::GenerateRegisterName(registerVariable->m_register) + "]";
					generateToken(token, TOKEN_DEC_SYMBOL);
				}
				else if (const auto variable = dynamic_cast<Symbol::AbstractVariable*>(symbol)) {
					if (const auto localVariable = dynamic_cast<Symbol::LocalVariable*>(variable)) {
						generateToken("[var_", TOKEN_DEC_SYMBOL);
					}
					else if (const auto memoryVariable = dynamic_cast<Symbol::MemoryVariable*>(variable)) {
						generateToken("[mem_", TOKEN_DEC_SYMBOL);
					}
					else if (const auto funcResultVar = dynamic_cast<Symbol::FunctionResultVar*>(variable)) {
						generateToken("[funcVar_", TOKEN_DEC_SYMBOL);
					}
					const auto endToken = Helper::String::NumberToHex(variable->getId()) + "_" + std::to_string(variable->getSize() * 8) + "]";
					generateToken(endToken, TOKEN_DEC_SYMBOL);
				}
			}

			virtual void generateSdaSymbol(CE::Symbol::ISymbol* symbol) {
				generateToken(symbol->getName(), TOKEN_SDA_SYMBOL);
			}

			virtual void generateDataType(DataTypePtr dataType) {
				generateToken(dataType->getDisplayName(), TOKEN_DATA_TYPE);
			}

			virtual void generateRoundBracket(const std::string& text, void* obj) {
				generateToken(text, TOKEN_ROUND_BRACKET);
			}

			virtual void generateToken(const std::string& text, TokenType tokenType) = 0;

		private:
			static std::string GetConditionType(Condition::ConditionType condType) {
				switch (condType)
				{
				case Condition::Eq: return "==";
				case Condition::Ne: return "!=";
				case Condition::Lt: return "<";
				case Condition::Le: return "<=";
				case Condition::Gt: return ">";
				case Condition::Ge: return ">=";
				}
				return "_";
			}
			
			static std::string GetCompConditionType(CompositeCondition::CompositeConditionType condType) {
				switch (condType)
				{
				case CompositeCondition::And: return "&&";
				case CompositeCondition::Or: return "||";
				}
				return "_";
			}
			
			static std::string GetOperationSize(int size, bool isFloat) {
				auto opSize = "." + std::to_string(size);
				if (isFloat) {
					opSize += "f";
				}
				return opSize;
			}
			
			static std::string GetOperation(OperationType opType) {
				switch (opType)
				{
				case Add: return "+";
				case Mul: return "*";
				case Div: return "/";
				case fAdd: return "+";
				case fMul: return "*";
				case fDiv: return "/";
				case Mod: return "%";
				case And: return "&";
				case Or: return "|";
				case Xor: return "^";
				case Shr: return ">>";
				case Shl: return "<<";
				case ReadValue: return "&";
				}
				return "_";
			}
		};

		class ExprTreeTextGenerator : public ExprTreeViewGenerator
		{
		public:
			std::string m_text;

			void generate(INode* node) override {
				m_text.clear();
				ExprTreeViewGenerator::generate(node);
			}

			void generateToken(const std::string& text, TokenType tokenType) override {
				m_text += text;
			}

			std::string getText(INode* node) {
				generate(node);
				return m_text;
			}
		};
	};


	class CodeViewGenerator
	{
		ExprTree::ExprTreeViewGenerator* m_exprTreeViewGenerator;
		std::set<LinearView::CodeBlock*> m_blocksToGoTo;

	protected:
		int m_tabsCount = 0;
	
	public:
		enum TokenType
		{
			TOKEN_TAB,
			TOKEN_CURLY_BRACKET,	// {}
			TOKEN_OPERATOR,			// if/while/return
			TOKEN_LABEL,
			TOKEN_SEMICOLON,		// ;
			TOKEN_END_LINE,
			TOKEN_COMMENT,
			TOKEN_DEBUG_INFO,
			TOKEN_OTHER
		};
		
		bool m_SHOW_ALL_COMMENTS = true;
		bool m_SHOW_ALL_GOTO = true;
		bool m_SHOW_LINEAR_LEVEL_EXT = true;
		bool m_SHOW_BLOCK_HEADER = true;
		bool m_SHOW_COMMENTS_IN_HEADER = true;

		CodeViewGenerator(ExprTree::ExprTreeViewGenerator* exprTreeViewGenerator)
			: m_exprTreeViewGenerator(exprTreeViewGenerator)
		{}

		void setMinInfoToShow() {
			m_SHOW_ALL_GOTO = false;
			m_SHOW_LINEAR_LEVEL_EXT = false;
		}

		virtual void generate(LinearView::BlockList* blockList) {
			m_blocksToGoTo.clear();
			generateBlockList(blockList, false);
		}

		virtual void generateHeader(const std::list<CE::Symbol::ISymbol*>& symbols) {
			for (auto symbol : symbols) {
				generateTabs();
				m_exprTreeViewGenerator->generateDataType(symbol->getDataType());
				generateToken(" ", TOKEN_OTHER);
				m_exprTreeViewGenerator->generateSdaSymbol(symbol);
				generateSemicolon();
				
				if (m_SHOW_ALL_COMMENTS && m_SHOW_COMMENTS_IN_HEADER) {
					std::string comment = "//priority: " + std::to_string(symbol->getDataType()->getPriority());
					//size
					if (symbol->getDataType()->isArray())
						comment += ", size: " + std::to_string(symbol->getDataType()->getSize());
					//offsets
					if (auto localInstrSymbol = dynamic_cast<CE::Symbol::LocalInstrVarSymbol*>(symbol)) {
						if (!localInstrSymbol->m_instrOffsets.empty()) {
							comment += ", offsets: ";
							for (auto off : localInstrSymbol->m_instrOffsets) {
								comment += std::to_string(off) + ", ";
							}
							comment.pop_back();
							comment.pop_back();
						}
					}
					generateToken(" ", TOKEN_OTHER);
					generateToken(comment, TOKEN_COMMENT);
				}
				generateEndLine();
			}
		}
		
		virtual void generateBlockList(LinearView::BlockList* blockList, bool generatingBraces = true) {
			if (generatingBraces) {
				generateCurlyBracket("{", blockList);
			}
			for (auto block : blockList->getBlocks()) {
				if (m_SHOW_ALL_COMMENTS && m_SHOW_BLOCK_HEADER) {
					generateCommentAboutBlock(block);
				}
				
				if (const auto conditionBlock = dynamic_cast<LinearView::ConditionBlock*>(block)) {
					generateConditionBlock(conditionBlock);
				}
				else if (const auto whileCycleBlock = dynamic_cast<LinearView::WhileCycleBlock*>(block)) {
					generateWhileCycleBlock(whileCycleBlock);
				}
				else if (const auto codeBlock = dynamic_cast<LinearView::CodeBlock*>(block)) {
					generateBlock(codeBlock);
					if (const auto endBlock = dynamic_cast<EndDecBlock*>(codeBlock->m_decBlock)) {
						if (endBlock->getReturnTopNode()->getNode() != nullptr)
							generateReturnStatement(endBlock->getReturnTopNode());
					}
				}
			}
			generateGotoStatement(blockList);
			if (generatingBraces) {
				generateCurlyBracket("}", blockList);
			}
		}

		void generateCommentAboutBlock(LinearView::AbstractBlock* block) {
			std::string comment;
			const auto levelInfo = "backOrderId: " + std::to_string(block->m_backOrderId) + ", linearLevel: " + std::to_string(block->m_linearLevel);
			if (const auto codeBlock = dynamic_cast<LinearView::CodeBlock*>(block)) {
				const auto decBlock = codeBlock->m_decBlock;
				comment = "//code block " + decBlock->m_pcodeBlock->getName() + " (" + levelInfo + ", level: " + std::to_string(decBlock->m_level) +
					", maxHeight: " + std::to_string(decBlock->m_maxHeight) + ", refCount: " +
					std::to_string(decBlock->getRefBlocksCount()) + ")";
			}
			else if (const auto whileCycleBlock = dynamic_cast<LinearView::WhileCycleBlock*>(block)) {
				comment = "//while block (enterBackOrderId: " + std::to_string(whileCycleBlock->m_enterBackOrderId) + ", " + levelInfo + ")";
			}
			else {
				comment = "//condition block (" + levelInfo + ")";
			}
			generateTabs();
			generateToken(comment, TOKEN_COMMENT);
			generateEndLine();
		}

		virtual void generateBlock(LinearView::CodeBlock* block) {
			if (m_blocksToGoTo.find(block) != m_blocksToGoTo.end()) {
				generateTabs();
				generateLabelName(block->m_decBlock->m_pcodeBlock);
				generateToken(":", TOKEN_OTHER);
				generateEndLine();
			}
			generateCode(block->m_decBlock);
		}

		virtual void generateConditionBlock(LinearView::ConditionBlock* block) {
			generateTabs();
			generateToken("if", TOKEN_OPERATOR);
			{
				generateBlockTopNode(block->m_jmpTopNode, block->m_cond);
			}
			generateToken(" ", TOKEN_OTHER);
			{
				generateBlockList(block->m_mainBranch);
			}
			if (m_SHOW_ALL_GOTO || !block->m_elseBranch->isEmpty()) {
				generateToken(" ", TOKEN_OTHER);
				generateToken("else", TOKEN_OPERATOR);
				generateToken(" ", TOKEN_OTHER);
				generateBlockList(block->m_elseBranch);
			}
			generateEndLine();
		}

		virtual void generateWhileCycleBlock(LinearView::WhileCycleBlock* block) {
			if (!block->m_isDoWhileCycle) {
				generateTabs();
				generateToken("while", TOKEN_OPERATOR);
				generateToken(" ", TOKEN_OTHER);
				{
					generateBlockTopNode(block->m_jmpTopNode, block->m_cond);
				}
				generateToken(" ", TOKEN_OTHER);
				{
					generateBlockList(block->m_mainBranch);
				}
				generateEndLine();
			}
			else {
				generateTabs();
				generateToken("do", TOKEN_OPERATOR);
				generateToken(" ", TOKEN_OTHER);
				generateBlockList(block->m_mainBranch);
				generateToken(" ", TOKEN_OTHER);
				generateToken("while", TOKEN_OPERATOR);
				generateToken(" ", TOKEN_OTHER);
				{
					generateBlockTopNode(block->m_jmpTopNode, block->m_cond);
				}
				generateSemicolon();
				generateEndLine();
			}
		}

		virtual void generateCode(DecBlock* decBlock) {
			for (auto line : decBlock->m_seqLines) {
				generateTabs();
				generateBlockTopNode(line);
				generateSemicolon();
				generateEndLine();
			}
			if (!decBlock->m_symbolParallelAssignmentLines.empty()) {
				generateTabs();
				generateToken("//Symbol parallel assignments:", TOKEN_COMMENT);
				generateEndLine();
				for (auto line : decBlock->m_symbolParallelAssignmentLines) {
					generateTabs();
					m_exprTreeViewGenerator->generateNode(line->getNode());
					generateEndLine();
				}
			}
		}

		virtual void generateBlockTopNode(DecBlock::BlockTopNode* blockTopNode, ExprTree::INode* node = nullptr) {
			m_exprTreeViewGenerator->generateNode(node ? node : blockTopNode->getNode());
		}

		virtual void generateReturnStatement(DecBlock::ReturnTopNode* returnTopNode) {
			generateTabs();
			generateToken("return", TOKEN_OPERATOR);
			generateToken(" ", TOKEN_OTHER);
			generateBlockTopNode(returnTopNode);
			generateSemicolon();
			generateEndLine();
		}

		virtual void generateGotoStatement(LinearView::BlockList* blockList) {
			std::string blockName = "none";
			bool hasGotoOperator = false;
			if (blockList->m_goto != nullptr) {
				const auto gotoType = blockList->getGotoType();
				blockName = blockList->m_goto->m_decBlock->m_pcodeBlock->getName();
				if (gotoType != LinearView::GotoType::None) {
					hasGotoOperator = true;
					generateTabs();
					if (gotoType == LinearView::GotoType::Normal) {
						generateToken("goto", TOKEN_OPERATOR);
						generateToken(" ", TOKEN_OTHER);
						generateLabelName(blockList->m_goto->m_decBlock->m_pcodeBlock);
						m_blocksToGoTo.insert(blockList->m_goto);
					}
					else if (gotoType == LinearView::GotoType::Break) {
						generateToken("break", TOKEN_OPERATOR);
					}
					else if (gotoType == LinearView::GotoType::Continue) {
						generateToken("continue", TOKEN_OPERATOR);
					}
					generateSemicolon();
					if (m_SHOW_ALL_COMMENTS && m_SHOW_ALL_GOTO)
						generateToken(" ", TOKEN_OTHER);
					else generateEndLine();
				}
			}

			if (m_SHOW_ALL_COMMENTS && m_SHOW_ALL_GOTO) {
				const auto comment = "//goto to block " + blockName;
				if (!hasGotoOperator)
					generateTabs();
				generateToken(comment, TOKEN_COMMENT);
				generateEndLine();
			}

			if (m_SHOW_ALL_COMMENTS && m_SHOW_LINEAR_LEVEL_EXT) {
				auto const comment2 = "//backOrderId: " + std::to_string(blockList->m_backOrderId) + "; minLinLevel: " +
					std::to_string(blockList->m_minLinearLevel) + ", maxLinLevel: " + std::to_string(blockList->m_maxLinearLevel);
				generateTabs();
				generateToken(comment2, TOKEN_COMMENT);
				generateEndLine();
			}
		}

		virtual void generateCurlyBracket(const std::string& text, LinearView::BlockList* blockList) {
			if (text == "}") {
				m_tabsCount--;
				generateTabs();
			}
			generateToken(text, TOKEN_CURLY_BRACKET);
			if (text == "{") {
				generateEndLine();
				m_tabsCount++;
			}
		}
		
		virtual void generateToken(const std::string& text, TokenType tokenType) = 0;

		void generateLabelName(PCodeBlock* pcodeBlock) {
			const auto labelName = "label_" + pcodeBlock->getName();
			generateToken(labelName, TOKEN_LABEL);
		}

		void generateTab() {
			generateToken("\t", TOKEN_TAB);
		}
		
		void generateTabs() {
			for (int i = 0; i < m_tabsCount; i++)
				generateTab();
		}

		void generateEndLine() {
			generateToken("\n", TOKEN_END_LINE);
		}

		void generateSemicolon() {
			generateToken(";", TOKEN_SEMICOLON);
		}
	};

	class CodeTextGenerator : public CodeViewGenerator
	{
		class ExprTreeGenerator : public ExprTree::ExprTreeViewGenerator
		{
			CodeTextGenerator* m_parentGen;
		public:
			ExprTreeGenerator(CodeTextGenerator* parentGen)
				: m_parentGen(parentGen)
			{}
			
			void generateToken(const std::string& text, TokenType tokenType) override {
				m_parentGen->m_text += text;
			}
		};
		
		ExprTreeGenerator m_exprTreeGenerator;
	public:
		std::string m_text;
		
		CodeTextGenerator()
			: m_exprTreeGenerator(this), CodeViewGenerator(&m_exprTreeGenerator)
		{}

		void generateToken(const std::string& text, TokenType tokenType) override {
			m_text += text;
		}

		void print(LinearView::BlockList* blockList, SdaCodeGraph* sdaCodeGraph = nullptr) {
			m_text.clear();
			if (sdaCodeGraph) {
				generateHeader(sdaCodeGraph->getSdaSymbols());
				generateEndLine();
				generateEndLine();
			}
			generate(blockList);
			printf("%s", m_text.c_str());
		}
	};
};
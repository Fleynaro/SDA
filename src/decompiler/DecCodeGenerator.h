#pragma once
#include "ExprTree/ExprTreeAssignmentNode.h"
#include "ExprTree/ExprTreeCondition.h"
#include "ExprTree/ExprTreeFunctionCall.h"
#include "ExprTree/ExprTreeLinearExpr.h"
#include "ExprTree/ExprTreeMirrorNode.h"
#include "ExprTree/ExprTreeNode.h"
#include "ExprTree/ExprTreeOperationalNode.h"
#include "SDA/ExprTree/ExprTreeSdaNode.h"

namespace CE::Decompiler
{
	namespace Symbol
	{
		class SymbolViewGenerator
		{
		public:
			void generate(Symbol* symbol) {
				if (const auto registerVariable = dynamic_cast<RegisterVariable*>(symbol)) {
					const auto token = "[reg_" + PCode::InstructionViewGenerator::GenerateRegisterName(registerVariable->m_register) + "]";
					generateToken(token);
				} else if (const auto variable = dynamic_cast<AbstractVariable*>(symbol)) {
					if (const auto localVariable = dynamic_cast<LocalVariable*>(variable)) {
						generateToken("[var_");
					}
					else if (const auto memoryVariable = dynamic_cast<MemoryVariable*>(variable)) {
						generateToken("[mem_");
					}
					else if (const auto funcResultVar = dynamic_cast<FunctionResultVar*>(variable)) {
						generateToken("[funcVar_");
					}
					generateToken(Helper::String::NumberToHex(variable->getId()) + "_" + std::to_string(variable->getSize() * 8) + "]");
				}
			}

			virtual void generateToken(const std::string& text) = 0;
		};
	};
	
	namespace ExprTree
	{
		class ExprTreeViewGenerator
		{
			Symbol::SymbolViewGenerator* m_symbolViewGenerator;
			bool m_debugMode = false;
		protected:
			enum TokenType
			{
				TOKEN_OPERATION,
				TOKEN_OPERATOR,
				TOKEN_FUNCTION_CALL,
				TOKEN_DATA_TYPE,
				TOKEN_SYMBOL,
				TOKEN_NUMBER,
				TOKEN_DEBUG_INFO,
				TOKEN_OTHER
			};
		
		public:
			ExprTreeViewGenerator(Symbol::SymbolViewGenerator* symbolViewGenerator, bool debugMode)
				: m_symbolViewGenerator(symbolViewGenerator), m_debugMode(debugMode)
			{}
			
			virtual void generate(INode* node) {
				if (const auto sdaNode = dynamic_cast<ISdaNode*>(node)) {
					generateSdaNode(sdaNode);
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

			virtual void generateSdaNode(ISdaNode* node) {
				
			}

			virtual void generateAssignmentNode(const AssignmentNode* node) {
				generate(node->getDstNode());
				generateToken(" = ", TOKEN_OPERATION);
				generate(node->getSrcNode());
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
					generateToken("(", TOKEN_OTHER);
					generate(node->m_leftNode);
					generateToken(", ", TOKEN_OTHER);
					generate(node->m_rightNode);
					generateToken(", ", TOKEN_OTHER);
					generateToken(std::to_string(node->m_rightNode->getSize() * 0x8), TOKEN_OTHER);
					generateToken(")", TOKEN_OTHER);
					return;
				}

				if (node->m_operation == Xor) {
					if (const auto numberLeaf = dynamic_cast<INumberLeaf*>(node->m_rightNode)) {
						if (numberLeaf->getValue() == static_cast<uint64_t>(-1)) {
							generateToken("~", TOKEN_OTHER);
							generate(numberLeaf);
						}
					}
					return;
				}

				const auto operation = GetOperation(node->m_operation);
				generateToken("(", TOKEN_OTHER);
				generate(node->m_leftNode);
				generateToken(" ", TOKEN_OTHER);
				generateToken(operation, TOKEN_OPERATION);
				if(m_debugMode) {
					generateToken(opSize, TOKEN_DEBUG_INFO);
				}
				generateToken(" ", TOKEN_OTHER);
				generate(node->m_rightNode);
				generateToken(")", TOKEN_OTHER);
			}

			virtual void generateReadValueNode(ReadValueNode* node) {
				if (!node->m_leftNode)
					return;
				const auto token = "*{uint_" + std::to_string(8 * node->getSize()) + "t*}";
				generateToken(token, TOKEN_OTHER);
				generate(node->m_rightNode);
			}

			virtual void generateCastNode(CastNode* node) {
				if (!node->m_leftNode)
					return;
				const auto token = "{" + std::string(!node->m_isSigned ? "u" : "") + "int_" + std::to_string(8 * node->getSize()) + "t}";
				generateToken(token, TOKEN_OTHER);
				generate(node->m_rightNode);
			}

			virtual void generateFunctionalNode(FunctionalNode* node) {
				if (!node->m_leftNode || !node->m_rightNode)
					return;
				generateToken(magic_enum::enum_name(node->m_funcId).data(), TOKEN_FUNCTION_CALL);
				generateToken("(", TOKEN_OTHER);
				generate(node->m_leftNode);
				generateToken(", ", TOKEN_OTHER);
				generate(node->m_rightNode);
				generateToken(")", TOKEN_OTHER);
			}

			virtual void generateFloatFunctionalNode(FloatFunctionalNode* node) {
				if (!node->m_leftNode)
					return;
				generateToken(magic_enum::enum_name(node->m_funcId).data(), TOKEN_FUNCTION_CALL);
				generateToken("(", TOKEN_OTHER);
				generate(node->m_leftNode);
				generateToken(")", TOKEN_OTHER);
			}

			virtual void generateFunctionCall(FunctionCall* node) {
				generateToken("(", TOKEN_OTHER);
				generate(node->getDestination());
				generateToken(")", TOKEN_OTHER);
				
				generateToken("(", TOKEN_OTHER);
				for (auto it = node->m_paramNodes.begin(); it != node->m_paramNodes.end(); ++it) {
					generate(*it);
					if(it != std::prev(node->m_paramNodes.end()))
						generateToken(", ", TOKEN_OTHER);
				}
				generateToken(")", TOKEN_OTHER);
			}

			virtual void generateSymbolLeaf(SymbolLeaf* leaf) {
				m_symbolViewGenerator->generate(leaf->m_symbol);
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

			virtual void generateLinearExpr(LinearExpr* node) {
				const auto operation = GetOperation(node->m_operation);
				const auto opSize = GetOperationSize(node->getSize(), node->isFloatingPoint());

				generateToken("(", TOKEN_OTHER);
				for (auto it = node->m_terms.begin(); it != node->m_terms.end(); ++it) {
					generate(*it);
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
					generate(node->m_constTerm);
				}
				generateToken(")", TOKEN_OTHER);
			}

			virtual void generateMirrorNode(MirrorNode* node) {
				generate(node->m_node);
			}

			virtual void generateBooleanValue(BooleanValue* node) {
				generateToken(node->m_value ? "true" : "false", TOKEN_NUMBER);
			}

			virtual void generateSimpleCondition(Condition* node) {
				if (!node->m_leftNode || !node->m_rightNode)
					return;
				generateToken("(", TOKEN_OTHER);
				generate(node->m_leftNode);
				generateToken(" ", TOKEN_OTHER);
				generateToken(GetConditionType(node->m_cond), TOKEN_OPERATION);
				generateToken(" ", TOKEN_OTHER);
				generate(node->m_rightNode);
				generateToken(")", TOKEN_OTHER);
			}

			virtual void generateCompositeCondition(CompositeCondition* node) {
				if (!node->m_leftCond)
					return;
				
				if (node->m_cond == CompositeCondition::None) {
					generate(node->m_leftCond);
					return;
				}
				
				if (node->m_cond == CompositeCondition::Not) {
					generateToken("!(", TOKEN_OTHER);
					generate(node->m_leftCond);
					generateToken(")", TOKEN_OTHER);
					return;
				}
				
				generateToken("(", TOKEN_OTHER);
				generate(node->m_leftCond);
				generateToken(" ", TOKEN_OTHER);
				generateToken(GetConditionType(node->m_cond), TOKEN_OPERATION);
				generateToken(" ", TOKEN_OTHER);
				generate(node->m_rightCond);
				generateToken(")", TOKEN_OTHER);
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
			
			static std::string GetConditionType(CompositeCondition::CompositeConditionType condType) {
				switch (condType)
				{
				case And: return "&&";
				case Or: return "||";
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
	};
};
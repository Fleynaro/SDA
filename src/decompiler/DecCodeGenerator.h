#pragma once
#include "datatypes/SystemType.h"
#include "ExprTree/ExprTreeAssignmentNode.h"
#include "ExprTree/ExprTreeCondition.h"
#include "ExprTree/ExprTreeFunctionCall.h"
#include "ExprTree/ExprTreeLinearExpr.h"
#include "ExprTree/ExprTreeMirrorNode.h"
#include "ExprTree/ExprTreeNode.h"
#include "ExprTree/ExprTreeOperationalNode.h"
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
		protected:
			enum TokenType
			{
				TOKEN_OPERATION,
				TOKEN_OPERATOR,
				TOKEN_FUNCTION_CALL,
				TOKEN_DATA_TYPE,
				TOKEN_DEC_SYMBOL,
				TOKEN_SDA_SYMBOL,
				TOKEN_NUMBER,
				TOKEN_DEBUG_INFO,
				TOKEN_OTHER
			};
		
		public:
			bool m_debugMode = false;
			bool m_markSdaNodes = false;
			
			virtual void generate(INode* node) {
				if (const auto sdaNode = dynamic_cast<ISdaNode*>(node)) {
					if (m_debugMode && m_markSdaNodes) {
						generateToken("@", TOKEN_DEBUG_INFO);
					}
					if (sdaNode->hasCast() && sdaNode->getCast()->hasExplicitCast()) {
						generateCast(sdaNode);
					}
					if (auto addressGetting = dynamic_cast<IMappedToMemory*>(this)) {
						if (addressGetting->isAddrGetting())
							generateToken("&", TOKEN_OPERATION);
					}
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

			virtual void generateCast(ISdaNode* node) {
				const auto dataTypeName = node->getCast()->getCastDataType()->getDisplayName();
				generateToken("(", TOKEN_OTHER);
				generateToken(dataTypeName, TOKEN_DATA_TYPE);
				generateToken(")", TOKEN_OTHER);
			}

			virtual void generateSdaNode(ISdaNode* node) {
				if (const auto sdaGenericNode = dynamic_cast<SdaGenericNode*>(node)) {
					generate(sdaGenericNode->getNode());
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
				generateToken("*", TOKEN_OPERATION);
				generate(node->getAddress());
			}

			virtual void generateSdaFunctionNode(SdaFunctionNode* node) {
				generate(node->m_funcCall);
			}
			
			virtual void generateUnknownLocation(UnknownLocation* node) {
				generate(node->m_linearExpr);
			}

			virtual void generateGoarArrayNode(GoarArrayNode* node) {
				generate(node->m_base);
				generateToken("[", TOKEN_OPERATION);
				generate(node->m_indexNode);
				generateToken("]", TOKEN_OPERATION);
			}

			virtual void generateGoarFieldNode(GoarFieldNode* node) {
				const auto isPointer = node->m_base->getDataType()->isPointer();
				generate(node->m_base);
				generateToken(isPointer ? "->" : ".", TOKEN_OPERATION);
				generateToken(node->m_field->getName(), TOKEN_OTHER);
			}

			virtual void generateGoarTopNode(GoarTopNode* node) {
				generate(node->m_base);
			}

			virtual void generateSdaSymbolLeaf(SdaSymbolLeaf* node) {
				generateSdaSymbol(node->getSdaSymbol());
			}

			virtual void generateSdaNumberLeaf(SdaNumberLeaf* node) {
				std::string number = "0x" + Helper::String::NumberToHex(node->m_value);
				if (node->getSrcDataType()->isFloatingPoint()) {
					if (node->getSrcDataType()->getSize() == 4)
						number = std::to_string(reinterpret_cast<float&>(node->m_value));
					else number = std::to_string(reinterpret_cast<double&>(node->m_value));
				}
				if (auto sysType = dynamic_cast<DataType::SystemType*>(node->getSrcDataType()->getBaseType())) {
					if (sysType->isSigned()) {
						const auto size = node->getSrcDataType()->getSize();
						if (size <= 4)
							number = std::to_string(static_cast<int32_t>(node->m_value));
						else
							number = std::to_string(static_cast<int64_t>(node->m_value));
					}
				}
				generateToken(number, TOKEN_NUMBER);
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

		class ExprTreeTextGenerator : public ExprTreeViewGenerator
		{
		public:
			std::string m_text;

			void generateToken(const std::string& text, TokenType tokenType) override {
				m_text += text;
			}
		};
	};


	
};
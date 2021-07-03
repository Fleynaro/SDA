#pragma once
#include <Project.h>
#include <decompiler/SDA/SdaGraphModification.h>

namespace CE::Decompiler::Symbolization
{
	//Calculating data types for all nodes and building GOAR structures
	class SdaDataTypesCalculater : public SdaGraphModification
	{
		DataType::IFunctionSignature* m_signature;
		Project* m_project;
	public:
		SdaDataTypesCalculater(SdaCodeGraph* sdaCodeGraph, DataType::IFunctionSignature* signature, Project* project);

		void start() override;

	private:
		//make a pass up through the specified top nodes
		void pass_up(const std::list<DecBlock::BlockTopNode*>& topNodes);

		//make a pass down through the specified top nodes
		void pass_down(const std::list<DecBlock::BlockTopNode*>& topNodes);

		void moveExplicitCastsDown(INode* node);

	protected:
		//used to proceed passing
		bool m_nextPassRequired = false;

		virtual void calculateDataTypes(INode* node);

		virtual void handleFunctionNode(SdaFunctionNode* sdaFunctionNode);

		virtual void handleUnknownLocation(UnknownLocation* unknownLocation);

		virtual void onDataTypeCasting(DataTypePtr fromDataType, DataTypePtr toDataType);

		// casting {sdaNode} to {toDataType}
		void cast(ISdaNode* sdaNode, DataTypePtr toDataType);

		// does it need explicit casting (e.g. (float)0x100024)
		bool isExplicitCast(DataTypePtr fromType, DataTypePtr toType);

		// calculate result data type for two operands
		DataTypePtr calcDataTypeForOperands(DataTypePtr opType1, DataTypePtr opType2) const;
	};
};
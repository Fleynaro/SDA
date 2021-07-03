#pragma once
#include <datatypes/TypeUnit.h>
#include <decompiler/ExprTree/ExprTree.h>
#include <decompiler/DecTopNode.h>
#include "../MemLocation.h"

namespace CE::Decompiler::ExprTree
{
	static bool g_MARK_SDA_NODES = false;

	// used for all sda nodes to cast them from their types to another in some cases
	class DataTypeCast
	{
		DataTypePtr m_castDataType;
		bool m_explicitCast = false;
	public:
		DataTypeCast() = default;

		DataTypePtr getCastDataType() const;

		bool hasExplicitCast() const;

		void setCastDataType(DataTypePtr dataType, bool isExplicit = false);

		void clearCast();
	};

	class ISdaNode : public virtual INode
	{
	public:
		DataTypePtr getDataType();

		virtual DataTypePtr getSrcDataType() = 0;

		virtual void setDataType(DataTypePtr dataType) = 0;

		bool hasCast();

		virtual DataTypeCast* getCast() = 0;

		virtual std::string printSdaDebug();
	};

	// means that the class addresses some memory location (not give a value!)
	class ILocatable : public virtual ISdaNode
	{
	public:
		virtual void getLocation(MemLocation& location) = 0;
	};

	// means that the class can give a value(or not) associated with some memory location
	class IMappedToMemory : public ILocatable
	{
	public:
		// check if does it give NAMED address, not value on this address: &globalVar
		virtual bool isAddrGetting() = 0;

		// globalVar -> &globalVar
		virtual void setAddrGetting(bool toggle) = 0;
	};

	// base class for most sda nodes
	class SdaNode : public Node, public virtual ISdaNode
	{
		DataTypeCast m_dataTypeCast;
	public:
		DataTypeCast* getCast() override sealed;

		INode* clone(NodeCloneContext* ctx) override sealed;

		std::string printDebug() override sealed;

	protected:
		virtual ISdaNode* cloneSdaNode(NodeCloneContext* ctx) = 0;
	};

	// for sda graph
	class SdaTopNode : public TopNode
	{
	public:
		SdaTopNode(ISdaNode* node);

		ISdaNode* getSdaNode();
	};
};
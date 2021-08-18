#pragma once
#include "AbstractSymbol.h"

namespace CE::DataType {
	class IStructure;
};

namespace CE::Symbol
{
	class StructFieldSymbol : public AbstractSymbol
	{
		int m_bitSize;
		int m_absBitOffset = -1;
	public:
		bool m_isDefault = false;
		DataType::IStructure* m_structure = nullptr;
		
		StructFieldSymbol(SymbolManager* manager = nullptr, DataTypePtr type = nullptr, int bitSize = 0, const std::string& name = "", const std::string& comment = "")
			: m_bitSize(bitSize), AbstractSymbol(manager, type, name, comment)
		{}

		Type getType() override;

		Decompiler::Storage getStorage() override;

		int getBitSize() const;

		void setBitSize(int size);

		int getAbsBitOffset() const;

		void setAbsBitOffset(int offset);

		int getBitOffset();

		int getOffset();

		bool isBitField();

		bool isDefault() const;

		StructFieldSymbol* clone() {
			const auto field = new StructFieldSymbol(getManager(), getDataType(), m_bitSize, getName(), getComment());
			field->setMapper(getMapper());
			return field;
		}
	};
};
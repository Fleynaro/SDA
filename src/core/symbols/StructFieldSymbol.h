#pragma once
#include "AbstractSymbol.h"

namespace CE::DataType {
	class IStructure;
};

namespace CE::Symbol
{
	class StructFieldSymbol : public AbstractSymbol
	{
	public:
		bool m_isDefault = false;
		
		StructFieldSymbol(SymbolManager* manager, DataType::IStructure* structure, DataTypePtr type, int absBitOffset, int bitSize, const std::string& name, const std::string& comment = "")
			: m_structure(structure), m_absBitOffset(absBitOffset), m_bitSize(bitSize), AbstractSymbol(manager, type, name, comment)
		{}

		Type getType() override;

		int getBitSize() const;

		int& getAbsBitOffset();

		void setAbsBitOffset(int offset);

		int getBitOffset();

		int getOffset();

		bool isBitField() const;

		void setStructure(DataType::IStructure* structure);

		bool isDefault() const;
	private:
		int m_bitSize;
		int m_absBitOffset;
		DataType::IStructure* m_structure;
	};
};
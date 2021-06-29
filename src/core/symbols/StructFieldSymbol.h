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
		StructFieldSymbol(SymbolManager* manager, DataType::IStructure* structure, DataTypePtr type, int absBitOffset, int bitSize, const std::string& name, const std::string& comment = "", bool isDefault = false)
			: m_structure(structure), m_absBitOffset(absBitOffset), m_bitSize(bitSize), m_isDefault(isDefault), AbstractSymbol(manager, type, name, comment)
		{}

		Type getType() override;

		int getBitSize();

		int& getAbsBitOffset();

		int getBitOffset();

		int getOffset();

		bool isBitField();

		void setStructure(DataType::IStructure* structure);

		bool isDefault();
	private:
		int m_bitSize;
		int m_absBitOffset;
		DataType::IStructure* m_structure;
		bool m_isDefault;
	};
};
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

		int getBitSize() const;

		int& getAbsBitOffset();

		int getBitOffset();

		int getOffset();

		bool isBitField() const;

		void setStructure(DataType::IStructure* structure);

		bool isDefault() const;
	private:
		int m_bitSize;
		int m_absBitOffset;
		DataType::IStructure* m_structure;
		bool m_isDefault;
	};
};
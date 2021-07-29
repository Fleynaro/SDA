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
		
		StructFieldSymbol(SymbolManager* manager = nullptr, DataType::IStructure* structure = nullptr, DataTypePtr type = nullptr, int absBitOffset = 0, int bitSize = 0, const std::string& name = "", const std::string& comment = "")
			: m_structure(structure), m_absBitOffset(absBitOffset), m_bitSize(bitSize), AbstractSymbol(manager, type, name, comment)
		{}

		Type getType() override;

		int getBitSize() const;

		void setBitSize(int size);

		int& getAbsBitOffset();

		void setAbsBitOffset(int offset);

		int getBitOffset();

		int getOffset();

		bool isBitField();

		void setStructure(DataType::IStructure* structure);

		bool isDefault() const;
	private:
		int m_bitSize;
		int m_absBitOffset;
		DataType::IStructure* m_structure;
	};
};
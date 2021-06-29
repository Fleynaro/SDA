#pragma once
#include <main.h>
#include <Utils/HashSerialization.h>

namespace CE::Decompiler
{
	//Means some place in the memory of the process
	class MemLocation
	{
		struct ArrayDim {
			int m_itemSize;
			int m_itemsMaxCount;
		};
	public:
		//A location type often represented as stack, global space or pointer to some stuff(stack, global space, pile)
		enum LOCATION_TYPE {
			STACK,
			GLOBAL,
			IMPLICIT, // when it's not clear what location does it mean, therefore let it be a pile 
			ALL
		};

		LOCATION_TYPE m_type;
		HS m_baseAddrHash = 0x0;
		int64_t m_offset = 0x0;
		std::list<ArrayDim> m_arrDims;
		int m_valueSize = 0x0;

		// -1 is an infinite size (method getLocSize() will return a very big value)
		void addArrayDim(int itemSize, int itemsMaxCount = -1);

		bool intersect(const MemLocation& location) const;

		bool equal(const MemLocation& location) const;
		
		int getLocSize() const;
	};
};
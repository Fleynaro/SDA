#pragma once
#include "../ExprTree/ExprTreeSda.h"
#include <Project.h>

namespace CE::Decompiler::Symbolization
{
	using namespace ExprTree;
	//Creating complex memory data structures based on given location that is linearly calculated, or based on raw bytes(when class fields packed in a register) sized up to 8.
	//Fields of class objects or Array
	class SdaGoarBuilding
	{
		Project* m_project;
		ISdaNode* m_baseSdaNode;
		int64_t m_bitOffset;
		std::list<ISdaNode*> m_sdaTerms; // the array index terms: players + {idx * 0x1000} + {idx2 * 0x2000}
	public:
		SdaGoarBuilding(UnknownLocation* unknownLocation, Project* project);
		
		ISdaNode* create();

	private:
		bool buildSingleGoar(ISdaNode*& sdaNode, int64_t& bitOffset, std::list<ISdaNode*>& terms) const;
	};
};
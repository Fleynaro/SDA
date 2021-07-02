#pragma once
#include <stdint.h>

namespace CE::Ghidra
{
	using Id = int64_t;

	class IMapper;
	class IObject
	{
	public:
		virtual Id getGhidraId() = 0;
		virtual bool doesSyncWithGhidra() = 0;
		virtual void makeSyncWithGhidra(bool toggle) = 0;
		virtual IMapper* getGhidraMapper() { return nullptr; }
		virtual void setGhidraMapper(IMapper* mapper) {}
	};
	
	class Object : virtual public IObject
	{
	public:
		bool doesSyncWithGhidra() override;

		void makeSyncWithGhidra(bool toggle) override;

		IMapper* getGhidraMapper() override;

		void setGhidraMapper(IMapper* mapper) override;
	private:
		bool m_sync = true;
		IMapper* m_mapper = nullptr;
	};
};
#pragma once
#include "GhidraAbstractMapper.h"
#include "GhidraClient.h"

namespace CE {
	class Project;
};

namespace CE::Ghidra
{
	using namespace ghidra;

	class Sync
	{
	public:
		Sync(CE::Project* programModule);

		~Sync();

		Project* getProject() const;

		Client* getClient() const;

		packet::DataSyncPacketManagerServiceClient* getDataSyncPacketManagerServiceClient() const;
	private:
		CE::Project* m_project;
		Client* m_client;
		packet::DataSyncPacketManagerServiceClient* m_dataSyncPacketManagerServiceClient;
	};
};
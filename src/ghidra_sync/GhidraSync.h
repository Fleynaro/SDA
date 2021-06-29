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

		Project* getProject();

		Client* getClient();

		packet::DataSyncPacketManagerServiceClient* getDataSyncPacketManagerServiceClient();
	private:
		CE::Project* m_project;
		Client* m_client;
		packet::DataSyncPacketManagerServiceClient* m_dataSyncPacketManagerServiceClient;
	};
};
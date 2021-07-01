#include "GhidraSync.h"
#include <managers/FunctionManager.h>
#include <managers/TypeManager.h>

using namespace CE;
using namespace CE::Ghidra;

Sync::Sync(CE::Project* programModule)
	: m_project(programModule)
{
	m_client = new Client;
	m_dataSyncPacketManagerServiceClient = new packet::DataSyncPacketManagerServiceClient(
		std::shared_ptr<TMultiplexedProtocol>(new TMultiplexedProtocol(m_client->m_protocol, "DataSyncPacketManager")));
}

Sync::~Sync() {
	delete m_client;
	delete m_dataSyncPacketManagerServiceClient;
}

Project* Sync::getProject() {
	return m_project;
}

Client* Sync::getClient() {
	return m_client;
}

packet::DataSyncPacketManagerServiceClient* Sync::getDataSyncPacketManagerServiceClient() {
	return m_dataSyncPacketManagerServiceClient;
}
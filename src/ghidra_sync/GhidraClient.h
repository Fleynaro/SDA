#pragma once
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TMultiplexedProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

using namespace apache::thrift;
using namespace protocol;
using namespace transport;

namespace CE::Ghidra
{
	class Client;
	class Transport
	{
	public:
		Transport(Client* client);

		~Transport();
	private:
		Client* m_client;
	};

	class Client
	{
	public:
		friend class Transport;
		std::shared_ptr<TProtocol> m_protocol;

		Client();
	private:
		std::shared_ptr<TTransport> m_socket;
		std::shared_ptr<TTransport> m_transport;
		int m_port = 9090;
	};
};
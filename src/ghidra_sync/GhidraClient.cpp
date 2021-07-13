#include "GhidraClient.h"

using namespace CE;
using namespace Ghidra;

Transport::~Transport()
{
	m_client->m_transport->close();
}

Transport::Transport(Client* client)
	: m_client(client)
{
	m_client->m_transport->open();
}

Client::Client() {
	m_socket = std::shared_ptr<TTransport>(new TSocket("localhost", m_port));
	m_transport = std::shared_ptr<TTransport>(new TBufferedTransport(m_socket));
	m_protocol = std::shared_ptr<TProtocol>(new TBinaryProtocol(m_transport));
}

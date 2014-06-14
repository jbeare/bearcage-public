/******************************************************************************
**                                                                           **
** Jonathan Beare (2014) - (2014)                                            **
** All Rights Reserved.                                                      **
**                                                                           **
** NOTICE:  All information contained herein is, and remains the property of **
** Jonathan Beare and his suppliers, if any.  The intellectual and technical **
** concepts contained herein are proprietary to Jonathan Beare and his       **
** suppliers and may be covered by U.S. and Foreign Patents, patents in      **
** process, and are protected by trade secret or copyright law.              **
** Dissemination of this information or reproduction of this material is     **
** strictly forbidden unless prior written permission is obtained from       **
** Jonathan Beare.                                                           **
**                                                                           **
******************************************************************************/

#include "SimpleServer.h"

void SimpleServer::Start() {
	boost::lock_guard<boost::recursive_mutex> lock(m_mutex);

	if(m_started) {
		return;
	}

	m_started = true;

	SimpleConnectionManager::Start();
}

void SimpleServer::Stop() {
	boost::lock_guard<boost::recursive_mutex> lock(m_mutex);

	if(!m_started) {
		return;
	}

	m_started = false;

	m_acceptor.cancel();

	SimpleConnectionManager::Stop();
}

void SimpleServer::AcceptConnection() {
	if(!m_started) {
		return;
	}

	boost::shared_ptr<SimpleConnection> connection = SimpleConnection::Create(IoService(), GetShared());

	m_acceptor.async_accept(connection->Socket(),
		boost::bind(&SimpleServer::HandleAcceptConnection, this, connection,
		boost::asio::placeholders::error));
}

void SimpleServer::HandleAcceptConnection(boost::shared_ptr<SimpleConnection> const &Connection, boost::system::error_code const &Error) {
	if(!Error) {
		auto connectionEvent = SimpleConnectionEvent::Create(SimpleConnectionEvent::Connected, Connection, std::vector<char>(), 0);
		HandleEvent(connectionEvent);
	}

	AcceptConnection();
}

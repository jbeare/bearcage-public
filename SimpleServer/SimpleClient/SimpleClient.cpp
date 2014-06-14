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

#include "SimpleClient.h"

void SimpleClient::Start() {
	boost::lock_guard<boost::recursive_mutex> lock(m_mutex);

	if(m_started) {
		return;
	}

	m_started = true;

	SimpleConnectionManager::Start();
}

void SimpleClient::Stop() {
	boost::lock_guard<boost::recursive_mutex> lock(m_mutex);

	if(!m_started) {
		return;
	}

	m_started = false;

	m_resolver.cancel();

	SimpleConnectionManager::Stop();
}

void SimpleClient::HandleResolve(boost::system::error_code const &Error, boost::asio::ip::tcp::resolver::iterator Iterator) {
	if(!Error) {
		boost::shared_ptr<SimpleConnection> connection = SimpleConnection::Create(IoService(), GetShared());
		connection->Socket().async_connect(*Iterator,
			boost::bind(&SimpleClient::HandleConnect, this, connection,
			boost::asio::placeholders::error));
	}
}

void SimpleClient::HandleConnect(boost::shared_ptr<SimpleConnection> const &Connection, boost::system::error_code const &Error) {
	if(!Error) {
		boost::shared_ptr<SimpleConnectionEvent> connectionEvent =
			SimpleConnectionEvent::Create(SimpleConnectionEvent::Connected, Connection, std::vector<char>(), 0);
		HandleEvent(connectionEvent);
	}
}

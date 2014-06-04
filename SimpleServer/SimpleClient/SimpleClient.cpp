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
	{
		boost::lock_guard<boost::recursive_mutex> lock(m_mutex);

		if(m_started) {
			return;
		}

		m_started = true;
	}

	m_thread = boost::thread(boost::bind(&SimpleClient::StartThread, this));
}

void SimpleClient::Stop() {
	boost::lock_guard<boost::recursive_mutex> lock(m_mutex);

	m_started = false;

	if(m_connection) {
		m_connection->Stop();
		m_connection.reset();
	}

	m_resolver.cancel();
	m_ioService.stop();
	m_thread.join();
	m_ioService.reset();
}

void SimpleClient::HandleResolve(const boost::system::error_code& Error, boost::asio::ip::tcp::resolver::iterator Iterator) {
	if(!Error) {
		boost::shared_ptr<SimpleConnection> connection = SimpleConnection::Create(m_ioService, m_callback);
		connection->Socket().async_connect(*Iterator,
			boost::bind(&SimpleClient::HandleConnect, this, connection,
			boost::asio::placeholders::error));
	}
}

void SimpleClient::HandleConnect(boost::shared_ptr<SimpleConnection> Connection, const boost::system::error_code& Error) {
	if(!Error) {
		boost::shared_ptr<SimpleConnectionEvent> connectionEvent =
			SimpleConnectionEvent::Create(SimpleConnectionEvent::Connected, Connection, std::vector<char>(), 0);
		Callback(connectionEvent);
		{
			boost::lock_guard<boost::recursive_mutex> lock(m_mutex);
			m_connection = Connection;
		}
		m_connection->Start();
	}
}

void SimpleClient::StartThread() {
	size_t result = 0;

	m_resolver.async_resolve(m_query,
		boost::bind(&SimpleClient::HandleResolve, this,
		boost::asio::placeholders::error,
		boost::asio::placeholders::iterator));
	result = m_ioService.run();
}

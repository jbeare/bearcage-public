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
	m_thread = boost::thread(boost::bind(&SimpleClient::StartThread, this));
}

void SimpleClient::Stop() {
	m_ioService.stop();
	m_thread.join();
}

void SimpleClient::HandleResolve(const boost::system::error_code& Error, boost::asio::ip::tcp::resolver::iterator Iterator) {
	if(!Error) {
		std::cout << "Resolved host." << std::endl;
		boost::shared_ptr<SimpleConnection> connection = SimpleConnection::Create(m_ioService, m_callback);
		connection->Socket().async_connect(*Iterator,
			boost::bind(&SimpleClient::HandleConnect, this, connection,
			boost::asio::placeholders::error));
	} else {
		std::cout << "Encountered an error resolving host." << std::endl;
	}
}

void SimpleClient::HandleConnect(boost::shared_ptr<SimpleConnection> Connection, const boost::system::error_code& Error) {
	if(!Error) {
		boost::shared_ptr<SimpleConnectionEvent> connectionEvent =
			SimpleConnectionEvent::Create(SimpleConnectionEvent::Connected, Connection, std::vector<char>(), 0);
		Callback(connectionEvent);
		Connection->Start();
	} else {
		std::cout << "Encountered an error establishing a connection: " << Error.message() << std::endl;
	}
}

void SimpleClient::StartThread() {
	m_resolver.async_resolve(m_query,
		boost::bind(&SimpleClient::HandleResolve, this,
		boost::asio::placeholders::error,
		boost::asio::placeholders::iterator));
	m_ioService.run();
}

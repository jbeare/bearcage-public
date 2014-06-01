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
	AcceptConnection();
	m_ioService.run();
}

void SimpleServer::Stop() {

}

void SimpleServer::AcceptConnection() {
	boost::shared_ptr<SimpleConnection> connection = SimpleConnection::create(m_ioService);

	m_acceptor.async_accept(connection->Socket(),
		boost::bind(&SimpleServer::HandleAcceptConnection, this, connection,
		boost::asio::placeholders::error));
}

void SimpleServer::HandleAcceptConnection(boost::shared_ptr<SimpleConnection> Connection, const boost::system::error_code& Error) {
	if(!Error) {
		std::cout << "Accepted a connection." << std::endl;
		Connection->Start();
		Connection->Write("Hello", 5);
	}

	AcceptConnection();
}

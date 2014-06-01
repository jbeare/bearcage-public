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

#pragma once

#include "SimpleConnection.h"

class SimpleServer {
public:
	SimpleServer(unsigned short Port) : 
		m_ioService(),
		m_acceptor(m_ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), Port)) {};

	void Start();

	void Stop();

private:
	void AcceptConnection();

	void HandleAcceptConnection(boost::shared_ptr<SimpleConnection> Connection, const boost::system::error_code& Error);

	boost::asio::io_service m_ioService;
	boost::asio::ip::tcp::acceptor m_acceptor;
};

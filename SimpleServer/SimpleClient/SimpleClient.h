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

class SimpleClient {
public:
	SimpleClient(char* Host, unsigned short Port) :
		m_ioService(),
		m_query(boost::asio::ip::tcp::v4(), std::string(Host), std::to_string(Port)),
		m_resolver(m_ioService) {};

	void Start();

	void Stop();

private:
	void HandleResolve(const boost::system::error_code& Error, boost::asio::ip::tcp::resolver::iterator Iterator);

	void HandleConnect(boost::shared_ptr<SimpleConnection> Connection, const boost::system::error_code& Error);

	boost::asio::io_service m_ioService;
	boost::asio::ip::tcp::resolver::query m_query;
	boost::asio::ip::tcp::resolver m_resolver;
};

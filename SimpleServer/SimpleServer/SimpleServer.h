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

#include <boost/thread.hpp>
#include "SimpleConnection.h"

class SimpleServer {
public:
	SimpleServer(unsigned short Port, void(*Callback)(boost::shared_ptr<SimpleConnectionEvent>)) :
		m_ioService(),
		m_acceptor(m_ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), Port)),
		m_callback(Callback),
		m_started(false) {
	
		UT_STAT_INCREMENT("SimpleServer");
	};

	~SimpleServer() {
		UT_STAT_DECREMENT("SimpleServer");
	};

	void Start();

	void Stop();

private:
	SimpleServer& operator=(const SimpleServer&) = delete;
	SimpleServer(const SimpleServer&) = delete;

	void AcceptConnection();

	void Callback(boost::shared_ptr<SimpleConnectionEvent> ConnectionEvent) {
		if(m_callback) {
			m_callback(ConnectionEvent);
		}
	}

	void HandleAcceptConnection(boost::shared_ptr<SimpleConnection> Connection, const boost::system::error_code& Error);

	void StartThread();

	boost::asio::io_service m_ioService;
	boost::asio::ip::tcp::acceptor m_acceptor;
	boost::thread m_thread;
	void (*m_callback)(boost::shared_ptr<SimpleConnectionEvent>);
	boost::recursive_mutex m_mutex;
	boost::atomic<bool> m_started;
	std::vector<boost::shared_ptr<SimpleConnection>> m_connections;
};

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

#include <string>
#include <boost/thread.hpp>
#include "SimpleConnection.h"

class SimpleClient {
public:
	SimpleClient(char* Host, unsigned short Port, void(*Callback)(boost::shared_ptr<SimpleConnectionEvent>)) :
		m_ioService(),
		m_query(boost::asio::ip::tcp::v4(), std::string(Host), std::to_string(Port)),
		m_resolver(m_ioService),
		m_callback(Callback),
		m_started(false),
		m_connection(NULL) {
	
		UT_CLASS_CONSTRUCTED("SimpleClient");
	};

	~SimpleClient() {
		UT_CLASS_DESTROYED("SimpleClient");
	}

	void Start();

	void Stop();

private:
	SimpleClient& operator=(const SimpleClient&) = delete;
	SimpleClient(const SimpleClient&) = delete;

	void Callback(boost::shared_ptr<SimpleConnectionEvent> ConnectionEvent) {
		if(m_callback) {
			m_callback(ConnectionEvent);
		}
	}

	void HandleResolve(const boost::system::error_code& Error, boost::asio::ip::tcp::resolver::iterator Iterator);

	void HandleConnect(boost::shared_ptr<SimpleConnection> Connection, const boost::system::error_code& Error);

	void StartThread();

	boost::asio::io_service m_ioService;
	boost::asio::ip::tcp::resolver::query m_query;
	boost::asio::ip::tcp::resolver m_resolver;
	boost::thread m_thread;
	void(*m_callback)(boost::shared_ptr<SimpleConnectionEvent>);
	boost::recursive_mutex m_mutex;
	boost::atomic<bool> m_started;
	boost::shared_ptr<SimpleConnection> m_connection;
};

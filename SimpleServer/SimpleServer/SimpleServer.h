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

#include "SimpleConnectionManager.h"

class SimpleServer : public SimpleConnectionManager {
public:
	static boost::shared_ptr<SimpleServer> Create(unsigned short Port,
		boost::shared_ptr<SimpleObject> const &Parent) {

		return boost::shared_ptr<SimpleServer>(new SimpleServer(Port, Parent));
	}

	boost::shared_ptr<SimpleServer> GetShared() {
		return boost::dynamic_pointer_cast<SimpleServer>(shared_from_this());
	}

	virtual void Start();

	virtual void Stop();

	~SimpleServer() {
		UT_STAT_DECREMENT("SimpleServer");
	};

protected:
	SimpleServer(unsigned short Port,
		boost::shared_ptr<SimpleObject> const &Parent) :
		m_acceptor(IoService(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), Port)),
		m_started(false),
		SimpleConnectionManager(Parent) {

		UT_STAT_INCREMENT("SimpleServer");
	};

private:
	SimpleServer& operator=(SimpleServer const &) = delete;
	SimpleServer(SimpleServer const &) = delete;

	void AcceptConnection();

	void HandleAcceptConnection(boost::shared_ptr<SimpleConnection> Connection, boost::system::error_code const &Error);

	virtual void IoServiceThreadEntry() {
		AcceptConnection();
		SimpleConnectionManager::IoServiceThreadEntry();
	}

	boost::asio::ip::tcp::acceptor m_acceptor;
	boost::recursive_mutex m_mutex;
	bool m_started;
};

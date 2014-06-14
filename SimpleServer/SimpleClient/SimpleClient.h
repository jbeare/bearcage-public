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

class SimpleClient : public SimpleConnectionManager {
public:
	static boost::shared_ptr<SimpleClient> Create(std::string const &Host, unsigned short Port,
		boost::shared_ptr<SimpleObject> const &Parent) {

		return boost::shared_ptr<SimpleClient>(new SimpleClient(Host, Port, Parent));
	}

	boost::shared_ptr<SimpleClient> GetShared() {
		return boost::dynamic_pointer_cast<SimpleClient>(shared_from_this());
	}

	virtual void Start();

	virtual void Stop();

	void Write(std::vector<char> const &Buffer) {
		auto connection = GetConnection();
		if(connection) {
			connection->Write(Buffer);
		}
	}

	~SimpleClient() {
		UT_STAT_DECREMENT("SimpleClient");
	}

protected:
	SimpleClient(std::string const &Host, unsigned short Port,
		boost::shared_ptr<SimpleObject> const &Parent) :
		m_query(boost::asio::ip::tcp::v4(), Host, std::to_string(Port)),
		m_resolver(IoService()),
		m_started(false),
		SimpleConnectionManager(Parent) {

		UT_STAT_INCREMENT("SimpleClient");
	};

private:
	SimpleClient& operator=(SimpleClient const &) = delete;
	SimpleClient(SimpleClient const &) = delete;

	void HandleResolve(boost::system::error_code const &Error, boost::asio::ip::tcp::resolver::iterator Iterator);

	void HandleConnect(boost::shared_ptr<SimpleConnection> const &Connection, boost::system::error_code const &Error);

	virtual void IoServiceThreadEntry() {
		m_resolver.async_resolve(m_query,
			boost::bind(&SimpleClient::HandleResolve, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::iterator));

		SimpleConnectionManager::IoServiceThreadEntry();
	}

	boost::asio::ip::tcp::resolver::query m_query;
	boost::asio::ip::tcp::resolver m_resolver;
	boost::recursive_mutex m_mutex;
	bool m_started;
};

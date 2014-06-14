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

#include <vector>
#include <boost/asio.hpp>
#include "SimpleTypes.h"

#define MAX_BUFFER_LENGTH 1000
#define DEFAULT_PORT 13

class SimpleConnection;

class SimpleConnectionEvent : public SimpleEvent {
public:
	static boost::shared_ptr<SimpleConnectionEvent> Create(EventType EventType,
		boost::shared_ptr<SimpleConnection> const &Connection,
		std::vector<char> const &Data, unsigned int Length) {

		return boost::shared_ptr<SimpleConnectionEvent>(new SimpleConnectionEvent(EventType, Connection, Data, Length));
	}

	boost::shared_ptr<SimpleConnection> Connection() {
		return m_connection;
	}

	std::vector<char> const &Data() {
		return m_data;
	}

	~SimpleConnectionEvent() {
		UT_STAT_DECREMENT("SimpleConnectionEvent");
	}

protected:
	SimpleConnectionEvent(EventType EventType,
		boost::shared_ptr<SimpleConnection> const &Connection, std::vector<char> const &Data, unsigned int Length) :
		m_connection(Connection),
		SimpleEvent(EventType) {
	
		if(Length > MAX_BUFFER_LENGTH || Length > Data.size()) {
			return;
		}

		m_data.resize(Length);
		memcpy_s(m_data.data(), m_data.size(), Data.data(), Length);
		UT_STAT_INCREMENT("SimpleConnectionEvent");
	};

private:
	SimpleConnectionEvent &operator=(SimpleConnectionEvent const &) = delete;
	SimpleConnectionEvent(SimpleConnectionEvent const &) = delete;

	boost::shared_ptr<SimpleConnection> m_connection;
	std::vector<char> m_data;
};

class SimpleConnection : public SimpleObject {
public:
	static boost::shared_ptr<SimpleConnection> Create(boost::asio::io_service &IoService,
		boost::shared_ptr<SimpleObject> const &Parent) {

		return boost::shared_ptr<SimpleConnection>(new SimpleConnection(IoService, Parent));
	}

	boost::shared_ptr<SimpleConnection> GetShared() {
		return boost::dynamic_pointer_cast<SimpleConnection>(shared_from_this());
	}

	boost::asio::ip::tcp::socket &Socket() {
		return m_socket;
	}

	void Start();

	void Stop();

	void Write(std::vector<char> const &Buffer);

	~SimpleConnection() {
		UT_STAT_DECREMENT("SimpleConnection");
	}

protected:
	SimpleConnection(boost::asio::io_service &IoService,
		boost::shared_ptr<SimpleObject> const &Parent) :
		m_socket(IoService),
		m_started(false),
		SimpleObject(Parent) {

		m_readBuffer.resize(MAX_BUFFER_LENGTH);
		UT_STAT_INCREMENT("SimpleConnection");
	};

private:
	SimpleConnection &operator=(SimpleConnection const &) = delete;
	SimpleConnection(SimpleConnection const &) = delete;

	void HandleRead(boost::system::error_code const &Error, size_t BytesTransferred);

	void HandleWrite(boost::system::error_code const &Error, size_t BytesTransferred);

	boost::asio::ip::tcp::socket m_socket;
	std::vector<char> m_readBuffer;
	std::vector<char> m_writeBuffer;
	bool m_started;
};

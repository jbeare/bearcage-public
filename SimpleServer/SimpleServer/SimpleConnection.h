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
#include <boost/atomic.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "SimpleObject.h"

#define MAX_BUFFER_LENGTH 1000
#define DEFAULT_PORT 13

class SimpleConnection;
class SimpleConnectionManager;

class SimpleConnectionEvent {
public:
	enum SimpleConnectionEventType{
		Connected,
		Disconnected,
		Read
	};

	static boost::shared_ptr<SimpleConnectionEvent> Create(SimpleConnectionEventType EventType,
		boost::shared_ptr<SimpleConnection> const &Connection, std::vector<char> const &Data, unsigned int Length) {

		return boost::shared_ptr<SimpleConnectionEvent>(new SimpleConnectionEvent(EventType, Connection, Data, Length));
	}

	SimpleConnectionEventType EventType() {
		return m_eventType;
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

private:
	SimpleConnectionEvent(SimpleConnectionEventType EventType,
		boost::shared_ptr<SimpleConnection> const &Connection, std::vector<char> const &Data, unsigned int Length) :
		m_eventType(EventType),
		m_connection(Connection) {
	
		if(Length > MAX_BUFFER_LENGTH || Length > Data.size()) {
			return;
		}

		m_data.resize(Length);
		memcpy_s(m_data.data(), m_data.size(), Data.data(), Length);
		UT_STAT_INCREMENT("SimpleConnectionEvent");
	};

	SimpleConnectionEvent &operator=(SimpleConnectionEvent const &) = delete;
	SimpleConnectionEvent(SimpleConnectionEvent const &) = delete;

	SimpleConnectionEventType m_eventType;
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

private:
	SimpleConnection(boost::asio::io_service &IoService,
		boost::shared_ptr<SimpleObject> const &Parent) :
		m_socket(IoService),
		m_started(false),
		SimpleObject(Parent) {

		m_readBuffer.resize(MAX_BUFFER_LENGTH);
		UT_STAT_INCREMENT("SimpleConnection");
	};

	SimpleConnection &operator=(SimpleConnection const &) = delete;
	SimpleConnection(SimpleConnection const &) = delete;

	void HandleRead(boost::system::error_code const &Error, size_t BytesTransferred);

	void HandleWrite(boost::system::error_code const &Error, size_t BytesTransferred);

	boost::asio::ip::tcp::socket m_socket;
	std::vector<char> m_readBuffer;
	std::vector<char> m_writeBuffer;
	boost::atomic<bool> m_started;
};

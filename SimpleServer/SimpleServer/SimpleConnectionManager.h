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

#include <boost/lockfree/queue.hpp>
#include <boost/thread.hpp>
#include "SimpleConnection.h"

class SimpleConnectionManager {
	friend class SimpleConnection;

public:
	SimpleConnectionManager(void(*Callback)(boost::shared_ptr<SimpleConnectionEvent>)) :
		m_ioService(),
		m_connectionEventCallback(Callback),
		m_connectionManagerStarted(false) {
	
		UT_STAT_INCREMENT("SimpleConnectionManager");
	};

	virtual ~SimpleConnectionManager() {
		UT_STAT_DECREMENT("SimpleConnectionManager");
	};

	virtual void Start();

	virtual void Stop();

protected:
	void HandleConnectionEvent(boost::shared_ptr<SimpleConnectionEvent> const &ConnectionEvent);

	void AddConnection(boost::shared_ptr<SimpleConnection> const &Connection);

	void RemoveConnection(boost::shared_ptr<SimpleConnection> const &Connection);

	boost::shared_ptr<SimpleConnection> GetConnection(unsigned int Index = 0);

	void ConnectionEventCallback(boost::shared_ptr<SimpleConnectionEvent> const &ConnectionEvent) {
		if(m_connectionEventCallback) {
			m_connectionEventCallback(ConnectionEvent);
		}
	}

	boost::asio::io_service &IoService() {
		return m_ioService;
	}

	virtual void IoServiceThreadEntry() {
		m_ioService.run();
	}

	void(*m_connectionEventCallback)(boost::shared_ptr<SimpleConnectionEvent>);

private:
	SimpleConnectionManager& operator=(SimpleConnectionManager const &) = delete;
	SimpleConnectionManager(SimpleConnectionManager const &) = delete;

	boost::asio::io_service m_ioService;
	boost::thread m_ioServiceThread;
	boost::atomic<bool> m_connectionManagerStarted;
	std::vector<boost::shared_ptr<SimpleConnection>> m_connections;
	boost::recursive_mutex m_connectionManagerMutex;
};

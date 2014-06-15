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

#define THREAD_POOL_SIZE 4

class SimpleConnectionManager : public SimpleObject {
public:
	static boost::shared_ptr<SimpleConnectionManager> Create(boost::shared_ptr<SimpleObject> const &Parent) {
		return boost::shared_ptr<SimpleConnectionManager>(new SimpleConnectionManager(Parent));
	}

	boost::shared_ptr<SimpleConnectionManager> GetShared() {
		return boost::dynamic_pointer_cast<SimpleConnectionManager>(shared_from_this());
	}

	virtual void Start();

	virtual void Stop();

	virtual ~SimpleConnectionManager() {
		UT_STAT_DECREMENT("SimpleConnectionManager");
	};

protected:
	SimpleConnectionManager(boost::shared_ptr<SimpleObject> const &Parent) :
		m_ioService(),
		m_connectionManagerStarted(false),
		SimpleObject(Parent) {

		UT_STAT_INCREMENT("SimpleConnectionManager");
	};

	virtual void HandleEvent(boost::shared_ptr<SimpleEvent> const &Event);

	void AddConnection(boost::shared_ptr<SimpleConnection> const &Connection);

	void RemoveConnection(boost::shared_ptr<SimpleConnection> const &Connection);

	boost::shared_ptr<SimpleConnection> GetConnection(unsigned int Index = 0);

	boost::asio::io_service &IoService() {
		return m_ioService;
	}

	virtual void IoServiceThreadEntry() {
		for(int i = 0; i < THREAD_POOL_SIZE; i++) {
			boost::shared_ptr<boost::thread> thread(new boost::thread(
				boost::bind(&boost::asio::io_service::run, &m_ioService)));

			m_ioServiceThreads.push_back(thread);
		}
	}

private:
	SimpleConnectionManager& operator=(SimpleConnectionManager const &) = delete;
	SimpleConnectionManager(SimpleConnectionManager const &) = delete;

	boost::asio::io_service m_ioService;
	std::vector<boost::shared_ptr<boost::thread>> m_ioServiceThreads;
	bool m_connectionManagerStarted;
	std::vector<boost::shared_ptr<SimpleConnection>> m_connections;
	boost::recursive_mutex m_connectionsMutex;
	boost::recursive_mutex m_connectionManagerMutex;
};

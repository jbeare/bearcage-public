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

#include "SimpleConnectionManager.h"

void SimpleConnectionManager::Start() {
	boost::lock_guard<boost::recursive_mutex> lock(m_connectionManagerMutex);

	if(m_connectionManagerStarted) {
		return;
	}

	m_connectionManagerStarted = true;

	m_ioServiceThread = boost::thread(boost::bind(&SimpleConnectionManager::IoServiceThreadEntry, this));
}

void SimpleConnectionManager::Stop() {
	boost::lock_guard<boost::recursive_mutex> lock(m_connectionManagerMutex);

	if(!m_connectionManagerStarted) {
		return;
	}

	m_connectionManagerStarted = false;

	for(auto &connection : m_connections) {
		connection->Stop();
	}

	m_connections.clear();
	m_ioService.stop();
	m_ioServiceThread.join();
	m_ioService.reset();
}

void SimpleConnectionManager::HandleConnectionEvent(boost::shared_ptr<SimpleConnectionEvent> const &ConnectionEvent) {
	switch(ConnectionEvent->EventType()) {
	case SimpleConnectionEvent::Connected:
		AddConnection(ConnectionEvent->Connection());
		break;
	case SimpleConnectionEvent::Disconnected:
		RemoveConnection(ConnectionEvent->Connection());
		break;
	case SimpleConnectionEvent::Read:
		break;
	default:
		break;
	}
	
	ConnectionEventCallback(ConnectionEvent);
}

void SimpleConnectionManager::AddConnection(boost::shared_ptr<SimpleConnection> const &Connection) {
	boost::lock_guard<boost::recursive_mutex> lock(m_connectionManagerMutex);

	if(!m_connectionManagerStarted) {
		return;
	}

	for(auto iter = m_connections.begin(); iter != m_connections.end(); iter++) {
		if(*iter == Connection) {
			return;
		}
	}

	m_connections.push_back(Connection);
	Connection->Start();
}

void SimpleConnectionManager::RemoveConnection(boost::shared_ptr<SimpleConnection> const &Connection) {
	boost::lock_guard<boost::recursive_mutex> lock(m_connectionManagerMutex);

	for(auto iter = m_connections.begin(); iter != m_connections.end(); iter++) {
		if(*iter == Connection) {
			(*iter)->Stop();
			m_connections.erase(iter);
			return;
		}
	}

	Connection->Stop();
}

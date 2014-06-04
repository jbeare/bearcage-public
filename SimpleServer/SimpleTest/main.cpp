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

#include "SimpleServer.h"
#include "SimpleClient.h"
#include "Utility.h"
#include <boost/thread/condition.hpp>

#define BOOST_TEST_MODULE SimpleTest
#include <boost/test/included/unit_test.hpp>

#define UT_TIMEOUT_MS 50
boost::mutex g_mutex;
boost::condition g_condition;

void ResetConnectionTimeout() {
	boost::lock_guard<boost::mutex> lock(g_mutex);
	g_condition.notify_all();
}

void WaitForConnections() {
	boost::mutex::scoped_lock lock(g_mutex);
	while(g_condition.wait_for(g_mutex, boost::chrono::milliseconds(UT_TIMEOUT_MS)) != boost::cv_status::timeout);
}

void ServerConnectionEventCallback(boost::shared_ptr<SimpleConnectionEvent> ConnectionEvent) {
	switch(ConnectionEvent->EventType()) {
	case SimpleConnectionEvent::Connected:
		ConnectionEvent->Connection()->Write(std::vector < char > {'h', 'e', 'l', 'l', 'o'});
		ResetConnectionTimeout();
		break;
	case SimpleConnectionEvent::Disconnected:
		ResetConnectionTimeout();
		break;
	case SimpleConnectionEvent::Read:
		ConnectionEvent->Connection()->Write(ConnectionEvent->Data());
		break;
	default:
		break;
	}
}

void ClientConnectionEventCallback(boost::shared_ptr<SimpleConnectionEvent> ConnectionEvent) {
	switch(ConnectionEvent->EventType()) {
	case SimpleConnectionEvent::Connected:
		ResetConnectionTimeout();
		break;
	case SimpleConnectionEvent::Disconnected:
		ResetConnectionTimeout();
		break;
	case SimpleConnectionEvent::Read:
		ConnectionEvent->Connection()->Write(ConnectionEvent->Data());
		break;
	default:
		break;
	}
}

BOOST_AUTO_TEST_SUITE(SimpleServerTestSuite)

BOOST_AUTO_TEST_CASE(SimpleServerTestCase1) {
	BOOST_CHECK_EQUAL(UT_CLASS_COUNT("SimpleServer"), 0);
	BOOST_CHECK_EQUAL(UT_CLASS_COUNT("SimpleClient"), 0);
	BOOST_CHECK_EQUAL(UT_CLASS_COUNT("SimpleConnection"), 0);
	BOOST_CHECK_EQUAL(UT_CLASS_COUNT("SimpleConnectionEvent"), 0);

	try {
		SimpleServer server(DEFAULT_PORT, &ServerConnectionEventCallback);
		SimpleClient client1("localhost", DEFAULT_PORT, &ClientConnectionEventCallback);
		SimpleClient client2("localhost", DEFAULT_PORT, &ClientConnectionEventCallback);

		for(int i = 0; i < 3; i++) {
			BOOST_CHECK_EQUAL(UT_CLASS_COUNT("SimpleServer"), 1);
			BOOST_CHECK_EQUAL(UT_CLASS_COUNT("SimpleClient"), 2);
			BOOST_CHECK_EQUAL(UT_CLASS_COUNT("SimpleConnection"), 0);
			BOOST_CHECK_EQUAL(UT_CLASS_COUNT("SimpleConnectionEvent"), 0);

			if(i % 3 == 0) {
				server.Start();
				client1.Start();
				client2.Start();
			} else if(i % 3 == 1) {
				client1.Start();
				server.Start();
				client2.Start();
			} else {
				client1.Start();
				client2.Start();
				server.Start();
			}

			WaitForConnections();

			BOOST_CHECK_EQUAL(UT_CLASS_COUNT("SimpleConnection"), 5);

			if(i % 3 == 0) {
				server.Stop();
				client1.Stop();
				client2.Stop();
			} else if(i % 3 == 1) {
				client1.Stop();
				server.Stop();
				client2.Stop();
			} else {
				client1.Stop();
				client2.Stop();
				server.Stop();
			}

			WaitForConnections();

			BOOST_CHECK_EQUAL(UT_CLASS_COUNT("SimpleServer"), 1);
			BOOST_CHECK_EQUAL(UT_CLASS_COUNT("SimpleClient"), 2);
			BOOST_CHECK_EQUAL(UT_CLASS_COUNT("SimpleConnection"), 0);
			BOOST_CHECK_EQUAL(UT_CLASS_COUNT("SimpleConnectionEvent"), 0);
		}
	} catch(std::exception& e) {
		BOOST_CHECK_MESSAGE(false, e.what());
	}

	BOOST_CHECK_EQUAL(UT_CLASS_COUNT("SimpleServer"), 0);
	BOOST_CHECK_EQUAL(UT_CLASS_COUNT("SimpleClient"), 0);
	BOOST_CHECK_EQUAL(UT_CLASS_COUNT("SimpleConnection"), 0);
	BOOST_CHECK_EQUAL(UT_CLASS_COUNT("SimpleConnectionEvent"), 0);
}

BOOST_AUTO_TEST_SUITE_END()

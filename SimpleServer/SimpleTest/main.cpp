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

#define UT_TIMEOUT_MS 100
boost::mutex g_mutex;
boost::condition g_condition;
boost::atomic<bool> g_serverLoopback(false);
boost::atomic<bool> g_clientLoopback(false);

void ResetCallbackConfig() {
	g_serverLoopback = false;
	g_clientLoopback = false;
}

void ResetActivityTimeout() {
	boost::lock_guard<boost::mutex> lock(g_mutex);
	g_condition.notify_all();
}

void WaitForActivity() {
	boost::mutex::scoped_lock lock(g_mutex);
	while(g_condition.wait_for(g_mutex, boost::chrono::milliseconds(UT_TIMEOUT_MS)) != boost::cv_status::timeout);
}

void ServerConnectionEventCallback(boost::shared_ptr<SimpleConnectionEvent> ConnectionEvent) {
	ResetActivityTimeout();
	switch(ConnectionEvent->EventType()) {
	case SimpleConnectionEvent::Connected:
		break;
	case SimpleConnectionEvent::Disconnected:
		break;
	case SimpleConnectionEvent::Read:
		UT_STAT_ADD("ServerReadBytes", ConnectionEvent->Data().size());
		if(g_serverLoopback) {
			ConnectionEvent->Connection()->Write(ConnectionEvent->Data());
		}
		break;
	default:
		break;
	}
	ResetActivityTimeout();
}

void ClientConnectionEventCallback(boost::shared_ptr<SimpleConnectionEvent> ConnectionEvent) {
	ResetActivityTimeout();
	switch(ConnectionEvent->EventType()) {
	case SimpleConnectionEvent::Connected:
		break;
	case SimpleConnectionEvent::Disconnected:
		break;
	case SimpleConnectionEvent::Read:
		UT_STAT_ADD("ClientReadBytes", ConnectionEvent->Data().size());
		if(g_clientLoopback) {
			ConnectionEvent->Connection()->Write(ConnectionEvent->Data());
		}
		break;
	default:
		break;
	}
	ResetActivityTimeout();
}

BOOST_AUTO_TEST_SUITE(SimpleServerTestSuite)

//Tests basic client/server construction/destruction and connectability
BOOST_AUTO_TEST_CASE(SimpleServerTestCase1) {
	UT_STAT_RESET_ALL();
	ResetCallbackConfig();

	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleServer"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleClient"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);

	try {
		SimpleServer server(DEFAULT_PORT, &ServerConnectionEventCallback);
		SimpleClient client1("localhost", DEFAULT_PORT, &ClientConnectionEventCallback);
		SimpleClient client2("localhost", DEFAULT_PORT, &ClientConnectionEventCallback);

		for(int i = 0; i < 3; i++) {
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleServer"), 1);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleClient"), 2);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);

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

			WaitForActivity();

			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 5);

			g_serverLoopback = true;
			g_clientLoopback = true;

			client1.Write(std::vector<char>{'t', 'e', 's', 't'});
			client2.Write(std::vector<char>{'t', 'e', 's', 't'});

			boost::this_thread::sleep_for(boost::chrono::milliseconds(50));

			g_serverLoopback = false;
			g_clientLoopback = false;

			WaitForActivity();

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

			WaitForActivity();

			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleServer"), 1);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleClient"), 2);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);
		}
	} catch(std::exception& e) {
		BOOST_CHECK_MESSAGE(false, e.what());
	}

	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleServer"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleClient"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);
}

//Tests server connected to a single client with a data stream
BOOST_AUTO_TEST_CASE(SimpleServerTestCase2) {
	UT_STAT_RESET_ALL();
	ResetCallbackConfig();

	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleServer"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleClient"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);

	try {
		SimpleServer server(DEFAULT_PORT, &ServerConnectionEventCallback);
		SimpleClient client("localhost", DEFAULT_PORT, &ClientConnectionEventCallback);

		for(int i = 0; i < 2; i++) {
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleServer"), 1);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleClient"), 1);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);

			if(i % 2 == 0) {
				server.Start();
				client.Start();
			} else {
				client.Start();
				server.Start();
			}

			WaitForActivity();

			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 3);

			UT_STAT_RESET("ServerReadBytes");
			UT_STAT_RESET("ClientReadBytes");
			g_serverLoopback = true;

			for(int j = 0; j < 100; j++) {
				client.Write(std::vector<char>{'t', 'e', 's', 't'});
			}

			WaitForActivity();

			BOOST_CHECK_EQUAL(UT_STAT_COUNT("ServerReadBytes"), 400);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("ClientReadBytes"), 400);
			g_serverLoopback = false;

			if(i % 2 == 0) {
				server.Stop();
				client.Stop();
			} else {
				client.Stop();
				server.Stop();
			}

			WaitForActivity();
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleServer"), 1);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleClient"), 1);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);
		}
	} catch(std::exception& e) {
		BOOST_CHECK_MESSAGE(false, e.what());
	}

	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleServer"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleClient"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);
}

//Tests persistant server connecting/disconnecting to many clients with large data streams
BOOST_AUTO_TEST_CASE(SimpleServerTestCase3) {
	UT_STAT_RESET_ALL();
	ResetCallbackConfig();

	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleServer"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleClient"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);

	try {
		SimpleServer server(DEFAULT_PORT, &ServerConnectionEventCallback);
		std::vector<boost::shared_ptr<SimpleClient>> clients;

		for(int i = 0; i < 100; i++) {
			clients.push_back(boost::shared_ptr<SimpleClient>(new SimpleClient("localhost", DEFAULT_PORT, &ClientConnectionEventCallback)));
		}

		BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleServer"), 1);
		BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleClient"), 100);
		BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
		BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);

		server.Start();
		for(boost::shared_ptr<SimpleClient> client : clients) {
			client->Start();
		}

		WaitForActivity();

		BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 201);

		g_serverLoopback = true;

		for(int j = 0; j < 10; j++) {
			UT_STAT_RESET("ServerReadBytes");
			UT_STAT_RESET("ClientReadBytes");

			for(boost::shared_ptr<SimpleClient> client : clients) {
				client->Write(std::vector<char>(MAX_BUFFER_LENGTH, 'a'));
			}

			WaitForActivity();

			BOOST_CHECK_EQUAL(UT_STAT_COUNT("ServerReadBytes"), 100000);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("ClientReadBytes"), 100000);

			for(int k = 0; k < 10; k++) {
				boost::shared_ptr<SimpleClient> rem = clients.back();
				clients.pop_back();
				rem->Stop();
			}

			for(int k = 0; k < 10; k++) {
				boost::shared_ptr<SimpleClient> add(new SimpleClient("localhost", DEFAULT_PORT, &ClientConnectionEventCallback));
				clients.push_back(add);
				add->Start();
			}

			WaitForActivity();

			BOOST_CHECK_EQUAL(UT_STAT_COUNT("ServerReadBytes"), 100000);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("ClientReadBytes"), 100000);
		}

		g_serverLoopback = false;

		for(boost::shared_ptr<SimpleClient> client : clients) {
			client->Stop();
		}
		server.Stop();

		WaitForActivity();
		BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleServer"), 1);
		BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleClient"), 100);
		BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
		BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);
	} catch(std::exception& e) {
		BOOST_CHECK_MESSAGE(false, e.what());
	}

	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleServer"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleClient"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);
}

BOOST_AUTO_TEST_SUITE_END()

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
#include <boost/atomic/atomic.hpp>
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

void ServerConnectionEventCallback(boost::shared_ptr<SimpleEvent> ConnectionEvent) {
	ResetActivityTimeout();
	switch(boost::dynamic_pointer_cast<SimpleConnectionEvent>(ConnectionEvent)->GetEventType()) {
	case SimpleConnectionEvent::Connected:
		break;
	case SimpleConnectionEvent::Disconnected:
		break;
	case SimpleConnectionEvent::Read_Completed:
		UT_STAT_ADD("ServerReadBytes", boost::dynamic_pointer_cast<SimpleConnectionEvent>(ConnectionEvent)->Data().size());
		if(g_serverLoopback) {
			boost::dynamic_pointer_cast<SimpleConnectionEvent>(ConnectionEvent)->Connection()->Write(
				boost::dynamic_pointer_cast<SimpleConnectionEvent>(ConnectionEvent)->Data());
		}
		break;
	default:
		break;
	}
	ResetActivityTimeout();
}

void ClientConnectionEventCallback(boost::shared_ptr<SimpleEvent> ConnectionEvent) {
	ResetActivityTimeout();
	switch(boost::dynamic_pointer_cast<SimpleConnectionEvent>(ConnectionEvent)->GetEventType()) {
	case SimpleConnectionEvent::Connected:
		break;
	case SimpleConnectionEvent::Disconnected:
		break;
	case SimpleConnectionEvent::Read_Completed:
		UT_STAT_ADD("ClientReadBytes", boost::dynamic_pointer_cast<SimpleConnectionEvent>(ConnectionEvent)->Data().size());
		if(g_clientLoopback) {
			boost::dynamic_pointer_cast<SimpleConnectionEvent>(ConnectionEvent)->Connection()->Write(
				boost::dynamic_pointer_cast<SimpleConnectionEvent>(ConnectionEvent)->Data());
		}
		break;
	default:
		break;
	}
	ResetActivityTimeout();
}

class TestServer : public SimpleServer {
public:
	static boost::shared_ptr<TestServer> Create(unsigned short Port,
		boost::shared_ptr<SimpleObject> const &Parent) {

		return boost::shared_ptr<TestServer>(new TestServer(Port, Parent));
	}

	boost::shared_ptr<TestServer> GetShared() {
		return boost::dynamic_pointer_cast<TestServer>(shared_from_this());
	}

	~TestServer() {
		UT_STAT_DECREMENT("TestServer");
	};

protected:
	TestServer(unsigned short Port,
		boost::shared_ptr<SimpleObject> const &Parent) :
		SimpleServer(Port, Parent) {

		UT_STAT_INCREMENT("TestServer");
	};

	virtual void HandleEvent(boost::shared_ptr<SimpleEvent> const &Event) {
		SimpleServer::HandleEvent(Event);
		ServerConnectionEventCallback(Event);
	}

private:
	TestServer& operator=(TestServer const &) = delete;
	TestServer(TestServer const &) = delete;
};

class TestClient : public SimpleClient {
public:
	static boost::shared_ptr<TestClient> Create(std::string const &Host, unsigned short Port,
		boost::shared_ptr<SimpleObject> const &Parent) {

		return boost::shared_ptr<TestClient>(new TestClient(Host, Port, Parent));
	}

	boost::shared_ptr<TestClient> GetShared() {
		return boost::dynamic_pointer_cast<TestClient>(shared_from_this());
	}

	~TestClient() {
		UT_STAT_DECREMENT("TestClient");
	};

protected:
	TestClient(std::string const &Host, unsigned short Port,
		boost::shared_ptr<SimpleObject> const &Parent) :
		SimpleClient(Host, Port, Parent) {

		UT_STAT_INCREMENT("TestClient");
	};

	virtual void HandleEvent(boost::shared_ptr<SimpleEvent> const &Event) {
		SimpleClient::HandleEvent(Event);
		ClientConnectionEventCallback(Event);
	}

private:
	TestClient& operator=(TestClient const &) = delete;
	TestClient(TestClient const &) = delete;
};

BOOST_AUTO_TEST_SUITE(TestServerTestSuite)

//Tests basic client/server construction/destruction and connectability
BOOST_AUTO_TEST_CASE(TestServerTestCase1) {
	UT_STAT_RESET_ALL();
	ResetCallbackConfig();

	BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestServer"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestClient"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);

	try {
		boost::shared_ptr<TestServer> server = TestServer::Create(DEFAULT_PORT, NULL);
		boost::shared_ptr<TestClient> client1 = TestClient::Create("localhost", DEFAULT_PORT, NULL);
		boost::shared_ptr<TestClient> client2 = TestClient::Create("localhost", DEFAULT_PORT, NULL);

		for(int i = 0; i < 3; i++) {
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestServer"), 1);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestClient"), 2);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);

			if(i % 3 == 0) {
				server->Start();
				client1->Start();
				client2->Start();
			} else if(i % 3 == 1) {
				client1->Start();
				server->Start();
				client2->Start();
			} else {
				client1->Start();
				client2->Start();
				server->Start();
			}

			WaitForActivity();

			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 5);

			g_serverLoopback = true;
			g_clientLoopback = true;

			client1->Write(std::vector<char>{'t', 'e', 's', 't'});
			client2->Write(std::vector<char>{'t', 'e', 's', 't'});

			boost::this_thread::sleep_for(boost::chrono::milliseconds(50));

			g_serverLoopback = false;
			g_clientLoopback = false;

			WaitForActivity();

			if(i % 3 == 0) {
				server->Stop();
				client1->Stop();
				client2->Stop();
			} else if(i % 3 == 1) {
				client1->Stop();
				server->Stop();
				client2->Stop();
			} else {
				client1->Stop();
				client2->Stop();
				server->Stop();
			}

			WaitForActivity();

			BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestServer"), 1);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestClient"), 2);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);
		}
	} catch(std::exception& e) {
		BOOST_CHECK_MESSAGE(false, e.what());
	}

	BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestServer"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestClient"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);
}

//Tests server connected to a single client with a data stream
BOOST_AUTO_TEST_CASE(TestServerTestCase2) {
	UT_STAT_RESET_ALL();
	ResetCallbackConfig();

	BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestServer"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestClient"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);

	try {
		boost::shared_ptr<TestServer> server = TestServer::Create(DEFAULT_PORT, NULL);
		boost::shared_ptr<TestClient> client = TestClient::Create("localhost", DEFAULT_PORT, NULL);

		for(int i = 0; i < 2; i++) {
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestServer"), 1);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestClient"), 1);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);

			if(i % 2 == 0) {
				server->Start();
				client->Start();
			} else {
				client->Start();
				server->Start();
			}

			WaitForActivity();

			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 3);

			UT_STAT_RESET("ServerReadBytes");
			UT_STAT_RESET("ClientReadBytes");
			g_serverLoopback = true;

			for(int j = 0; j < 100; j++) {
				client->Write(std::vector<char>{'t', 'e', 's', 't'});
			}

			WaitForActivity();

			BOOST_CHECK_EQUAL(UT_STAT_COUNT("ServerReadBytes"), 400);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("ClientReadBytes"), 400);
			g_serverLoopback = false;

			if(i % 2 == 0) {
				server->Stop();
				client->Stop();
			} else {
				client->Stop();
				server->Stop();
			}

			WaitForActivity();
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestServer"), 1);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestClient"), 1);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);
		}
	} catch(std::exception& e) {
		BOOST_CHECK_MESSAGE(false, e.what());
	}

	BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestServer"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestClient"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);
}

//Tests persistant server connecting/disconnecting to many clients with large data streams
BOOST_AUTO_TEST_CASE(TestServerTestCase3) {
	UT_STAT_RESET_ALL();
	ResetCallbackConfig();

	BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestServer"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestClient"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);

	try {
		boost::shared_ptr<TestServer> server = TestServer::Create(DEFAULT_PORT, NULL);
		std::vector<boost::shared_ptr<TestClient>> clients;

		for(int i = 0; i < 100; i++) {
			clients.push_back(TestClient::Create("localhost", DEFAULT_PORT, NULL));
		}

		BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestServer"), 1);
		BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestClient"), 100);
		BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
		BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);

		server->Start();
		for(boost::shared_ptr<TestClient> client : clients) {
			client->Start();
		}

		WaitForActivity();

		BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 201);

		g_serverLoopback = true;

		for(int j = 0; j < 10; j++) {
			UT_STAT_RESET("ServerReadBytes");
			UT_STAT_RESET("ClientReadBytes");

			for(boost::shared_ptr<TestClient> client : clients) {
				client->Write(std::vector<char>(MAX_BUFFER_LENGTH, 'a'));
			}

			WaitForActivity();

			BOOST_CHECK_EQUAL(UT_STAT_COUNT("ServerReadBytes"), 100000);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("ClientReadBytes"), 100000);

			for(int k = 0; k < 10; k++) {
				boost::shared_ptr<TestClient> rem = clients.back();
				clients.pop_back();
				rem->Stop();
			}

			for(int k = 0; k < 10; k++) {
				boost::shared_ptr<TestClient> add(TestClient::Create("localhost", DEFAULT_PORT, NULL));
				clients.push_back(add);
				add->Start();
			}

			WaitForActivity();

			BOOST_CHECK_EQUAL(UT_STAT_COUNT("ServerReadBytes"), 100000);
			BOOST_CHECK_EQUAL(UT_STAT_COUNT("ClientReadBytes"), 100000);
		}

		g_serverLoopback = false;

		for(boost::shared_ptr<TestClient> client : clients) {
			client->Stop();
		}
		server->Stop();

		WaitForActivity();
		BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestServer"), 1);
		BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestClient"), 100);
		BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
		BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);
	} catch(std::exception& e) {
		BOOST_CHECK_MESSAGE(false, e.what());
	}

	BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestServer"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("TestClient"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnection"), 0);
	BOOST_CHECK_EQUAL(UT_STAT_COUNT("SimpleConnectionEvent"), 0);
}

BOOST_AUTO_TEST_SUITE_END()

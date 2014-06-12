#include "stdafx.h"
#include "CppUnitTest.h"

#include "SimpleObject.h"
#include "SimpleServer.h"
#include "SimpleClient.h"
#include "Utility.h"
#include <boost/thread/condition.hpp>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


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

namespace SimpleTest2
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			UT_STAT_RESET_ALL();
			ResetCallbackConfig();

			Assert::AreEqual(UT_STAT_COUNT("SimpleServer"), 0);
			Assert::AreEqual(UT_STAT_COUNT("SimpleClient"), 0);
			Assert::AreEqual(UT_STAT_COUNT("SimpleConnection"), 0);
			Assert::AreEqual(UT_STAT_COUNT("SimpleConnectionEvent"), 0);

			try {
				boost::shared_ptr<SimpleServer> server = SimpleServer::Create(DEFAULT_PORT, &ServerConnectionEventCallback, NULL);
				boost::shared_ptr<SimpleClient> client1 = SimpleClient::Create("localhost", DEFAULT_PORT, &ClientConnectionEventCallback, NULL);
				boost::shared_ptr<SimpleClient> client2 = SimpleClient::Create("localhost", DEFAULT_PORT, &ClientConnectionEventCallback, NULL);

				for(int i = 0; i < 3; i++) {
					Assert::AreEqual(UT_STAT_COUNT("SimpleServer"), 1);
					Assert::AreEqual(UT_STAT_COUNT("SimpleClient"), 2);
					Assert::AreEqual(UT_STAT_COUNT("SimpleConnection"), 0);
					Assert::AreEqual(UT_STAT_COUNT("SimpleConnectionEvent"), 0);

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

					Assert::AreEqual(UT_STAT_COUNT("SimpleConnection"), 5);

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

					Assert::AreEqual(UT_STAT_COUNT("SimpleServer"), 1);
					Assert::AreEqual(UT_STAT_COUNT("SimpleClient"), 2);
					Assert::AreEqual(UT_STAT_COUNT("SimpleConnection"), 0);
					Assert::AreEqual(UT_STAT_COUNT("SimpleConnectionEvent"), 0);
				}
			} catch(std::exception& e) {
				Logger::WriteMessage(e.what());
				Assert::Fail(NULL);
			}

			Assert::AreEqual(UT_STAT_COUNT("SimpleServer"), 0);
			Assert::AreEqual(UT_STAT_COUNT("SimpleClient"), 0);
			Assert::AreEqual(UT_STAT_COUNT("SimpleConnection"), 0);
			Assert::AreEqual(UT_STAT_COUNT("SimpleConnectionEvent"), 0);
		}

		TEST_METHOD(TestMethod2)
		{
			UT_STAT_RESET_ALL();
			ResetCallbackConfig();

			Assert::AreEqual(UT_STAT_COUNT("SimpleServer"), 0);
			Assert::AreEqual(UT_STAT_COUNT("SimpleClient"), 0);
			Assert::AreEqual(UT_STAT_COUNT("SimpleConnection"), 0);
			Assert::AreEqual(UT_STAT_COUNT("SimpleConnectionEvent"), 0);

			try {
				boost::shared_ptr<SimpleServer> server = SimpleServer::Create(DEFAULT_PORT, &ServerConnectionEventCallback, NULL);
				boost::shared_ptr<SimpleClient> client = SimpleClient::Create("localhost", DEFAULT_PORT, &ClientConnectionEventCallback, NULL);

				for(int i = 0; i < 2; i++) {
					Assert::AreEqual(UT_STAT_COUNT("SimpleServer"), 1);
					Assert::AreEqual(UT_STAT_COUNT("SimpleClient"), 1);
					Assert::AreEqual(UT_STAT_COUNT("SimpleConnection"), 0);
					Assert::AreEqual(UT_STAT_COUNT("SimpleConnectionEvent"), 0);

					if(i % 2 == 0) {
						server->Start();
						client->Start();
					} else {
						client->Start();
						server->Start();
					}

					WaitForActivity();

					Assert::AreEqual(UT_STAT_COUNT("SimpleConnection"), 3);

					UT_STAT_RESET("ServerReadBytes");
					UT_STAT_RESET("ClientReadBytes");
					g_serverLoopback = true;

					for(int j = 0; j < 100; j++) {
						client->Write(std::vector<char>{'t', 'e', 's', 't'});
					}

					WaitForActivity();

					Assert::AreEqual(UT_STAT_COUNT("ServerReadBytes"), 400);
					Assert::AreEqual(UT_STAT_COUNT("ClientReadBytes"), 400);
					g_serverLoopback = false;

					if(i % 2 == 0) {
						server->Stop();
						client->Stop();
					} else {
						client->Stop();
						server->Stop();
					}

					WaitForActivity();
					Assert::AreEqual(UT_STAT_COUNT("SimpleServer"), 1);
					Assert::AreEqual(UT_STAT_COUNT("SimpleClient"), 1);
					Assert::AreEqual(UT_STAT_COUNT("SimpleConnection"), 0);
					Assert::AreEqual(UT_STAT_COUNT("SimpleConnectionEvent"), 0);
				}
			} catch(std::exception& e) {
				Logger::WriteMessage(e.what());
				Assert::Fail(NULL);
			}

			Assert::AreEqual(UT_STAT_COUNT("SimpleServer"), 0);
			Assert::AreEqual(UT_STAT_COUNT("SimpleClient"), 0);
			Assert::AreEqual(UT_STAT_COUNT("SimpleConnection"), 0);
			Assert::AreEqual(UT_STAT_COUNT("SimpleConnectionEvent"), 0);
		}

		TEST_METHOD(TestMethod3)
		{
			UT_STAT_RESET_ALL();
			ResetCallbackConfig();

			Assert::AreEqual(UT_STAT_COUNT("SimpleServer"), 0);
			Assert::AreEqual(UT_STAT_COUNT("SimpleClient"), 0);
			Assert::AreEqual(UT_STAT_COUNT("SimpleConnection"), 0);
			Assert::AreEqual(UT_STAT_COUNT("SimpleConnectionEvent"), 0);

			try {
				boost::shared_ptr<SimpleServer> server = SimpleServer::Create(DEFAULT_PORT, &ServerConnectionEventCallback, NULL);
				std::vector<boost::shared_ptr<SimpleClient>> clients;

				for(int i = 0; i < 100; i++) {
					clients.push_back(SimpleClient::Create("localhost", DEFAULT_PORT, &ClientConnectionEventCallback, NULL));
				}

				Assert::AreEqual(UT_STAT_COUNT("SimpleServer"), 1);
				Assert::AreEqual(UT_STAT_COUNT("SimpleClient"), 100);
				Assert::AreEqual(UT_STAT_COUNT("SimpleConnection"), 0);
				Assert::AreEqual(UT_STAT_COUNT("SimpleConnectionEvent"), 0);

				server->Start();
				for(boost::shared_ptr<SimpleClient> client : clients) {
					client->Start();
				}

				WaitForActivity();

				Assert::AreEqual(UT_STAT_COUNT("SimpleConnection"), 201);

				g_serverLoopback = true;

				for(int j = 0; j < 10; j++) {
					UT_STAT_RESET("ServerReadBytes");
					UT_STAT_RESET("ClientReadBytes");

					for(boost::shared_ptr<SimpleClient> client : clients) {
						client->Write(std::vector<char>(MAX_BUFFER_LENGTH, 'a'));
					}

					WaitForActivity();

					Assert::AreEqual(UT_STAT_COUNT("ServerReadBytes"), 100000);
					Assert::AreEqual(UT_STAT_COUNT("ClientReadBytes"), 100000);

					for(int k = 0; k < 10; k++) {
						boost::shared_ptr<SimpleClient> rem = clients.back();
						clients.pop_back();
						rem->Stop();
					}

					for(int k = 0; k < 10; k++) {
						boost::shared_ptr<SimpleClient> add(SimpleClient::Create("localhost", DEFAULT_PORT, &ClientConnectionEventCallback, NULL));
						clients.push_back(add);
						add->Start();
					}

					WaitForActivity();

					Assert::AreEqual(UT_STAT_COUNT("ServerReadBytes"), 100000);
					Assert::AreEqual(UT_STAT_COUNT("ClientReadBytes"), 100000);
				}

				g_serverLoopback = false;

				for(boost::shared_ptr<SimpleClient> client : clients) {
					client->Stop();
				}
				server->Stop();

				WaitForActivity();
				Assert::AreEqual(UT_STAT_COUNT("SimpleServer"), 1);
				Assert::AreEqual(UT_STAT_COUNT("SimpleClient"), 100);
				Assert::AreEqual(UT_STAT_COUNT("SimpleConnection"), 0);
				Assert::AreEqual(UT_STAT_COUNT("SimpleConnectionEvent"), 0);
			} catch(std::exception& e) {
				Logger::WriteMessage(e.what());
				Assert::Fail(NULL);
			}

			Assert::AreEqual(UT_STAT_COUNT("SimpleServer"), 0);
			Assert::AreEqual(UT_STAT_COUNT("SimpleClient"), 0);
			Assert::AreEqual(UT_STAT_COUNT("SimpleConnection"), 0);
			Assert::AreEqual(UT_STAT_COUNT("SimpleConnectionEvent"), 0);
		}
	};
}
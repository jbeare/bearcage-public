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

#include "SimpleClient.h"

boost::shared_ptr<SimpleConnection> g_connection;

void ConnectionEventCallback(boost::shared_ptr<SimpleEvent> ConnectionEvent) {
	if(!ConnectionEvent) {
		return;
	}

	switch(boost::dynamic_pointer_cast<SimpleConnectionEvent>(ConnectionEvent)->GetEventType()) {
	case SimpleConnectionEvent::Connected:
		std::cout << "ConnectionEventCallback: Connected" << std::endl;
		g_connection = boost::dynamic_pointer_cast<SimpleConnectionEvent>(ConnectionEvent)->Connection();
		break;
	case SimpleConnectionEvent::Disconnected:
		std::cout << "ConnectionEventCallback: Disconnected" << std::endl;
		g_connection.reset();
		break;
	case SimpleConnectionEvent::Read_Completed:
		std::cout << "ConnectionEventCallback: Read" << std::endl;
		std::cout.write(boost::dynamic_pointer_cast<SimpleConnectionEvent>(ConnectionEvent)->Data().data(),
			boost::dynamic_pointer_cast<SimpleConnectionEvent>(ConnectionEvent)->Data().size());
		std::cout << std::endl;
		break;
	default:
		break;
	}
}

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
		ConnectionEventCallback(Event);
	}

private:
	TestClient& operator=(TestClient const &) = delete;
	TestClient(TestClient const &) = delete;
};

int main(int argc, char* argv[]) {
	try {
		std::vector<char> data;
		data.resize(5);
		memcpy_s(data.data(), data.size(), "Hello", 5);

		boost::shared_ptr<TestClient> client = TestClient::Create("localhost", DEFAULT_PORT, NULL);
		if(client) {
			client->Start();

			for(;;) {
				boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));

				if(g_connection) {
					g_connection->Write(data);
				}
			}

			client->Stop();
		}
	} catch(std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	system("pause");

	return 0;
}

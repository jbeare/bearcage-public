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

void ConnectionEventCallback(boost::shared_ptr<SimpleEvent> ConnectionEvent) {
	switch(boost::dynamic_pointer_cast<SimpleConnectionEvent>(ConnectionEvent)->GetEventType()) {
	case SimpleConnectionEvent::Connected:
		std::cout << "ConnectionEventCallback: Connected" << std::endl;
		break;
	case SimpleConnectionEvent::Disconnected:
		std::cout << "ConnectionEventCallback: Disconnected" << std::endl;
		break;
	case SimpleConnectionEvent::Read_Completed:
		std::cout << "ConnectionEventCallback: Read_Completed" << std::endl;
		std::cout.write(boost::dynamic_pointer_cast<SimpleConnectionEvent>(ConnectionEvent)->Data().data(),
			boost::dynamic_pointer_cast<SimpleConnectionEvent>(ConnectionEvent)->Data().size());
		std::cout << std::endl;
		boost::dynamic_pointer_cast<SimpleConnectionEvent>(ConnectionEvent)->Connection()->Write(
			boost::dynamic_pointer_cast<SimpleConnectionEvent>(ConnectionEvent)->Data());
		break;
	default:
		break;
	}
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
		ConnectionEventCallback(Event);
	}

private:
	TestServer& operator=(TestServer const &) = delete;
	TestServer(TestServer const &) = delete;
};

int main() {
	try {
		std::vector<char> data;
		data.resize(5);
		memcpy_s(data.data(), data.size(), "Hello", 5);

		boost::shared_ptr<TestServer> server = TestServer::Create(DEFAULT_PORT, NULL);
		if(server) {
			server->Start();

			for(;;) {
				boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
			}

			server->Stop();
		}
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	system("pause");

	return 0;
}

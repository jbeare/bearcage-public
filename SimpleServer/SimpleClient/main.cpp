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

void ConnectionEventCallback(boost::shared_ptr<SimpleConnectionEvent> ConnectionEvent) {
	switch(ConnectionEvent->EventType()) {
	case SimpleConnectionEvent::Connected:
		std::cout << "ConnectionEventCallback: Connected" << std::endl;
		g_connection = ConnectionEvent->Connection();
		break;
	case SimpleConnectionEvent::Disconnected:
		std::cout << "ConnectionEventCallback: Disconnected" << std::endl;
		g_connection.reset();
		break;
	case SimpleConnectionEvent::Read:
		std::cout << "ConnectionEventCallback: Read" << std::endl;
		std::cout.write(ConnectionEvent->Data().data(), ConnectionEvent->Data().size());
		std::cout << std::endl;
		break;
	default:
		break;
	}
}

int main(int argc, char* argv[]) {
	try {
		std::vector<char> data;
		data.resize(5);
		memcpy_s(data.data(), data.size(), "Hello", 5);

		SimpleClient client("localhost", DEFAULT_PORT, &ConnectionEventCallback);
		client.Start();

		for(;;) {
			boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));

			if(g_connection) {
				g_connection->Write(data);
			}
		}

		client.Stop();
	} catch(std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	system("pause");

	return 0;
}

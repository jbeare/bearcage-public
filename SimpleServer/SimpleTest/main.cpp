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
#include "Utility.h"

ConfigurationManager G_CONFIG;

#define BOOST_TEST_MODULE SimpleTest
#include <boost/test/included/unit_test.hpp>

void ConnectionEventCallback(boost::shared_ptr<SimpleConnectionEvent> ConnectionEvent) {
	switch(ConnectionEvent->EventType()) {
	case SimpleConnectionEvent::Connected:
		std::cout << "ConnectionEventCallback: Connected" << std::endl;
		break;
	case SimpleConnectionEvent::Disconnected:
		std::cout << "ConnectionEventCallback: Disconnected" << std::endl;
		break;
	case SimpleConnectionEvent::Read:
		std::cout << "ConnectionEventCallback: Read" << std::endl;
		std::cout.write(ConnectionEvent->Data().data(), ConnectionEvent->Data().size());
		std::cout << std::endl;
		ConnectionEvent->Connection()->Write(ConnectionEvent->Data());
		break;
	default:
		break;
	}
}

BOOST_AUTO_TEST_SUITE(SimpleServerTestSuite)

BOOST_AUTO_TEST_CASE(SimpleServerTestCase1) {
	BOOST_CHECK_EQUAL(UT_CLASS_COUNT("SimpleServer"), 0);

	try {
		SimpleServer server(13, &ConnectionEventCallback);
		BOOST_CHECK_EQUAL(UT_CLASS_COUNT("SimpleServer"), 1);
		server.Start();
		server.Stop();
		BOOST_CHECK_EQUAL(UT_CLASS_COUNT("SimpleServer"), 1);
	} catch(std::exception& e) {
		BOOST_CHECK_MESSAGE(false, e.what());
	}

	BOOST_CHECK_EQUAL(UT_CLASS_COUNT("SimpleServer"), 0);
}

BOOST_AUTO_TEST_SUITE_END()

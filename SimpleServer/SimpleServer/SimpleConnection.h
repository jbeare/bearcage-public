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

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#define MAX_BUFFER_LENGTH 1024
#define DEFAULT_PORT 13

class SimpleConnection : public boost::enable_shared_from_this<SimpleConnection> {
public:
	static boost::shared_ptr<SimpleConnection> create(boost::asio::io_service& IoService) {
		return boost::shared_ptr<SimpleConnection>(new SimpleConnection(IoService));
	}

	boost::asio::ip::tcp::socket& Socket() {
		return m_socket;
	}

	void Start();

	void Stop();

	void Write(char* Buffer, unsigned int Length);

private:
	SimpleConnection(boost::asio::io_service& IoService) : m_socket(IoService) {};

	void HandleRead(const boost::system::error_code& Error, size_t BytesTransferred);

	void HandleWrite(const boost::system::error_code& Error, size_t BytesTransferred);

	boost::asio::ip::tcp::socket m_socket;
	std::string m_message;
	char m_readBuffer[MAX_BUFFER_LENGTH];
	char m_writeBuffer[MAX_BUFFER_LENGTH];
};

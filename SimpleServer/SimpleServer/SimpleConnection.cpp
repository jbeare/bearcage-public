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

#include "SimpleConnection.h"

void SimpleConnection::Start() {
	Socket().async_read_some(boost::asio::buffer(m_readBuffer),
		boost::bind(&SimpleConnection::HandleRead, shared_from_this(),
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}

void SimpleConnection::Stop() {

}

void SimpleConnection::Write(std::vector<char>& Buffer) {
	if(Buffer.size() > MAX_BUFFER_LENGTH) {
		return;
	}

	m_writeBuffer = Buffer;
	boost::asio::async_write(m_socket, boost::asio::buffer(m_writeBuffer),
		boost::bind(&SimpleConnection::HandleWrite, shared_from_this(),
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}

void SimpleConnection::HandleRead(const boost::system::error_code& Error, size_t BytesTransferred) {
	if(Error.value() == boost::system::errc::success) {
		if(BytesTransferred) {
			boost::shared_ptr<SimpleConnectionEvent> connectionEvent =
				SimpleConnectionEvent::Create(SimpleConnectionEvent::Read, shared_from_this(), m_readBuffer, BytesTransferred);
			Callback(connectionEvent);
		}

		Socket().async_read_some(boost::asio::buffer(m_readBuffer),
			boost::bind(&SimpleConnection::HandleRead, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	} else {
		boost::shared_ptr<SimpleConnectionEvent> connectionEvent = 
			SimpleConnectionEvent::Create(SimpleConnectionEvent::Disconnected, shared_from_this(), std::vector<char>(), 0);
		Callback(connectionEvent);
	}
}

void SimpleConnection::HandleWrite(const boost::system::error_code& Error, size_t BytesTransferred) {
	if(Error.value() == boost::system::errc::success) {
		std::cout << "Wrote bytes: " << BytesTransferred << std::endl;
	} else {
		std::cout << "Encountered an error writing: " << Error.message() << std::endl;
	}
}
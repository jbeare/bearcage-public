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
	Socket().async_read_some(boost::asio::buffer(m_readBuffer, MAX_BUFFER_LENGTH),
		boost::bind(&SimpleConnection::HandleRead, shared_from_this(),
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}

void SimpleConnection::Stop() {

}

void SimpleConnection::Write(char* Buffer, unsigned int Length) {
	if(!Buffer || !Length || Length > MAX_BUFFER_LENGTH) {
		return;
	}

	if(memcpy_s(m_writeBuffer, MAX_BUFFER_LENGTH, Buffer, Length)) {
		return;
	}

	boost::asio::async_write(m_socket, boost::asio::buffer(Buffer, Length),
		boost::bind(&SimpleConnection::HandleWrite, shared_from_this(),
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}

void SimpleConnection::HandleRead(const boost::system::error_code& Error, size_t BytesTransferred) {
	if(Error.value() == boost::system::errc::success) {
		std::cout.write(m_readBuffer, BytesTransferred);
		std::cout << std::endl;

		Socket().async_read_some(boost::asio::buffer(m_readBuffer, MAX_BUFFER_LENGTH),
			boost::bind(&SimpleConnection::HandleRead, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));

		Write(m_readBuffer, BytesTransferred);
	} else {
		std::cout << "Read failure" << std::endl;
	}
}

void SimpleConnection::HandleWrite(const boost::system::error_code& Error, size_t BytesTransferred) {
	UNREFERENCED_PARAMETER(Error);
	UNREFERENCED_PARAMETER(BytesTransferred);
}

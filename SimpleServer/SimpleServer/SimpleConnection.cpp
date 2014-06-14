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

#include "SimpleConnectionManager.h"

void SimpleConnection::Start() {
	if(m_started) {
		return;
	}

	m_started = true;

	m_socket.async_read_some(boost::asio::buffer(m_readBuffer),
		boost::bind(&SimpleConnection::HandleRead, GetShared(),
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}

void SimpleConnection::Stop() {
	m_started = false;
	m_socket.close();
}

void SimpleConnection::Write(std::vector<char> const &Buffer) {
	if(!m_started) {
		return;
	}

	if(Buffer.size() > MAX_BUFFER_LENGTH) {
		return;
	}

	m_writeBuffer = Buffer;
	//TODO: Need to add a thread and buffer queue to implement write_async
	boost::asio::write(m_socket, boost::asio::buffer(m_writeBuffer));
}

void SimpleConnection::HandleRead(boost::system::error_code const &Error, size_t BytesTransferred) {
	if(Error.value() == boost::system::errc::success) {
		if(BytesTransferred) {
			auto connectionEvent = SimpleConnectionEvent::Create(SimpleConnectionEvent::Read_Completed, GetShared(), m_readBuffer, BytesTransferred);
			HandleEvent(connectionEvent);
		}

		m_socket.async_read_some(boost::asio::buffer(m_readBuffer),
			boost::bind(&SimpleConnection::HandleRead, GetShared(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	} else {
		auto connectionEvent = SimpleConnectionEvent::Create(SimpleConnectionEvent::Disconnected, GetShared(), std::vector<char>(), 0);
		HandleEvent(connectionEvent);
	}
}

void SimpleConnection::HandleWrite(boost::system::error_code const &Error, size_t BytesTransferred) {
	UNREFERENCED_PARAMETER(Error);
	UNREFERENCED_PARAMETER(BytesTransferred);
}

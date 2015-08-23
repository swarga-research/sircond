/*
 * $DateTime: 2015/08/23 13:00:12 $                                                               
 * $Id: //depot/sircond/client.cpp#3 $
 * $Change: 2154 $
 *
 * Copyright (C) 2015 Swarga Research (http://www.swarga-research.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#include "pch.h"
#include "client.h"

//!
//! \brief Add data to the transmit buffer.
//!
//! \param[in] buf The data to be sent
//! \param[in] len The number of octets to send
//!
//! \retval bool Returns true if the data was successfully queued, or
//! false if an error occured.
//!
//========================================================================
bool CLIENT::Send (const uint8_t* buf, uint32_t len)
{

	if (len <= m_txbuf.GetWriteLen())
	{
		memcpy(m_txbuf.GetWritePtr(), buf, len);
		m_txbuf.MarkWritten(len);
		return true;
	}
	return false;
}

//!
//! \brief Disconnect a client
//!
//! Closes the client's socket and discards any unsent data.
//!
//========================================================================
void CLIENT::Drop ()
{

    if (m_sock != -1)
    {
        LogWrite(LEVEL_INFO, "Closing client connection...");
        closesocket(m_sock);
        m_sock = -1;
    }

	// DISCARD ANY PENDING DATA
	m_txbuf.Clear();
}

//!
//! \internal
//! \brief Transmit data from the transmit staging buffer.
//!
//! \retval bool Returns true if the operation was successful, or false
//! if an error occurred.
//!
//========================================================================
bool CLIENT::TransmitData ()
{
	bool result = false;

	// OUR SOCKET IS WRITABLE - TRANSMIT STAGED DATA
	if (m_txbuf.GetReadLen() > 0u)
	{
		int n = send(m_sock, 
			reinterpret_cast<char*>(m_txbuf.GetReadPtr()), 
			m_txbuf.GetReadLen(), 
			0);
		if (n >= 0)
		{
			m_txbuf.MarkRead(n);
			result = true;
		}
		else
		{
			LogWrite(LEVEL_ERROR, "send() error %d on client socket.", errno);
		}
	}
	return result;
}

//!
//! \internal
//! \brief Read data from the client socket into the receive staging buffer.
//!
//! \retval bool Returns true if the operation was successful, or false
//! if an error occurred.
//!
//========================================================================
bool CLIENT::ReceiveData ()
{
	bool result = false;

	if (m_rxbuf.GetWriteLen() == 0u)
	{
		LogWrite(LEVEL_ERROR, "Buffer full!");
		m_rxbuf.Clear();	// PUNT
	}

    int n = recv(m_sock, 
		reinterpret_cast<char*>(m_rxbuf.GetWritePtr()), 
		m_rxbuf.GetWriteLen(), 
		0);
    if (n > 0)
    {
        m_rxbuf.MarkWritten(n);
        m_rxbuf.MarkRead(ProcessData(m_rxbuf.GetReadPtr(), m_rxbuf.GetReadLen()));
		result = true;
    }
	else if (n < 0)
    {
        LogWrite(LEVEL_ERROR, "recv() error %d on client socket.", errno);
	}
	else
	{
		LogWrite(LEVEL_INFO, "Client closed connection.");
	}

	return result;
}

//!
//! \internal
//! \brief Process data from the given buffer
//! Any significant processing is left to the derived class.
//!
//! \param[in] buf The data to process
//! \param[in] len The number of octets in the buffer
//!
//! \retval uint32_t Returns the number of octets consumed from the buffer
//!
//========================================================================
uint32_t CLIENT::ProcessData (uint8_t* buf, uint32_t len)
{

	LogWrite(LEVEL_DEBUG, "CLIENT::ProcessData(): Processing %u bytes...", len);
	return len;		// TOSS IT INTO THE BIT BUCKET
}

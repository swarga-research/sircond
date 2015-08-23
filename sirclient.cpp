/*
 * $DateTime: 2015/08/23 13:00:12 $                                                               
 * $Id: //depot/sircond/sirclient.cpp#1 $
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
#include "sirserver.h"
#include "sirclient.h"


//!
//! \brief Constructor
//!
//! \param[in] server A reference to the server object which manages this
//! client.
//! \param[in] socket The socket descriptor for this client.
//! \param[in] bufsize The size (in octets) of the RX and TX staging 
//! buffers.
//!
//========================================================================
CSirClient::CSirClient (CSirServer& server, SOCKET socket, uint32_t bufsize) : 
	CLIENT(socket, bufsize), m_server(server)
{ 

}

//!
//! \internal
//! \brief Extract and process lines of text from the given buffer.
//!
//! \param[in] buf The data to process.
//! \param[in] len The number of octets in the buffer.
//!
//! \retval uint32_t Returns the number of octets consumed from the buffer.
//!
//========================================================================
uint32_t CSirClient::ProcessData (uint8_t* buf, uint32_t len)
{
	uint32_t offset = 0u;

	// EXTRACT LINES OF TEXT STARTING WITH '!' AND ENDING WITH '\n'
	while (offset < len)
	{
		// SCAN UNTIL EOL
		string line;
		for (uint32_t i = 0u; (offset + i) < len; ++i)
		{
			if (buf[offset + i] == '\n')
			{
				m_server.ProcessCommand(this, line);
				offset += (i + 1);
				break;
			}
			line += buf[offset + i];
		}
	}

	return offset;
}


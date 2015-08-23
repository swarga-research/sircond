/**
 * $DateTime: 2015/08/23 13:00:12 $                                                                
 * $Id: //depot/sircond/client.h#3 $
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

#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "pch.h"
#include "sobuf.h"

class SERVER;

//!
//! \brief Base class for clients of a SERVER object
//!
//! This base class contains a socket descriptor and I/O 
//! buffers (and methods to deal with the above). Anything
//! more than this is left to the derived class.
//!
class CLIENT
{
public:
	CLIENT() = delete;
    CLIENT (SOCKET socket, uint32_t bufsize) : m_sock(socket), m_rxbuf(bufsize), m_txbuf(bufsize) { };
	virtual ~CLIENT () { }
	bool IsDead () { return (m_sock == -1); }
    SOCKET GetSock () { return m_sock; };
	bool TxPending () { return (m_txbuf.GetReadLen() > 0u); }
    bool Send (const uint8_t* buf, uint32_t len);
    void Drop ();
	bool TransmitData ();
    bool ReceiveData ();

protected:
    virtual uint32_t ProcessData (uint8_t* buf, uint32_t len);

private:
    SOCKET m_sock;
	sr::SOBuffer m_rxbuf;
	sr::SOBuffer m_txbuf;
};

#endif

/**
 * $DateTime: 2015/08/23 13:00:12 $                                                                
 * $Id: //depot/sircond/server.h#3 $
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

#ifndef _SERVER_H_
#define _SERVER_H_

#include "pch.h"
#include <list>
#include <mutex>
#include "ctask.h"
#include "client.h"

using std::list;

//!
//! \brief Base class for a server object
//!
//! This base class provides basic server functionality such as listening 
//! for TCP connections and managing a list of connected clients. Any
//! application-specific processing will be implemented in a derived
//! class.
//!
class SERVER : public sr::CTask
{
public:
	SERVER() : m_bufsize(0u), m_port(0u), m_sock(-1) { FD_ZERO(&m_fds);  }
	virtual ~SERVER () { }
	void SetBufferSize (uint32_t bufsize) { m_bufsize = bufsize; }
	void SetPort (uint16_t port) { m_port = port; }
	void SetLocalAddress (string addr) { m_addr = addr; };
	bool OnStart ();
	void OnRun ();
	void OnExit ();

protected:
	bool Broadcast (const uint8_t* buf, uint8_t len);
	virtual void OnNewClient(CLIENT* client);
	virtual void OnDrop(CLIENT* client);
	virtual void Drop (CLIENT* client);
	uint32_t m_bufsize;			//!< Size of RX/TX buffers (bytes)

private:
	virtual CLIENT* NewClient (SOCKET s) { return new CLIENT(s, m_bufsize); }
	bool OnConnect();

	string m_addr;				//!< Address of the local interface
	uint16_t m_port;			//!< Local port number on which we listen
	SOCKET m_sock;				//!< Listening socket
	fd_set m_fds;				//!< Set of active file descriptors
	SOCKET m_highsock;			//!< Highest socket in m_fds (needed for select())
	list<CLIENT*> m_clients;	//!< List of currently attached clients
	std::mutex m_mutex;			//!< Serializes access to the client list
};

#endif

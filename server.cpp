/*
 * $DateTime: 2015/08/23 13:00:12 $                                                               
 * $Id: //depot/sircond/server.cpp#3 $
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
#include <algorithm>
#include "server.h"


//!
//! \brief Perform some initialization and then start listening on the
//! master socket.
//!
//! \retval bool Returns true if the initialization was successful, or 
//! false if an error occurred.
//!
//========================================================================
bool SERVER::OnStart ()
{
    struct sockaddr_in localaddr;   // LOCAL ADDRESS TO WHICH TO BIND
    list<CLIENT*>::iterator i;

    m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_sock == -1)
    {
        LogWrite(LEVEL_ERROR, "Error %d allocating listening socket.", errno);
        return false;
    }

	FD_SET(m_sock, &m_fds);
	m_highsock = m_sock;

    int on = 1;
    setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&on), sizeof(on));

	// BIND TO THE SELECTED LOCAL INTERFACE
    memset(&localaddr, '\0', sizeof(localaddr));
    localaddr.sin_family = AF_INET;
	if (m_addr.empty())
	{
		// DEFAULT TO ALL INTERFACES
		localaddr.sin_addr.s_addr = INADDR_ANY;
	}
	else
	{
		Resolve(m_addr.c_str(), &localaddr.sin_addr);
	}
    localaddr.sin_port = htons(m_port);
    if (bind(m_sock, (sockaddr*)&localaddr, sizeof(localaddr)) != 0)
    {
        LogWrite(LEVEL_ERROR, "Error %d binding to local address.", errno);
        return false;
    }

    if (listen(m_sock, 1) != 0)
    {
        LogWrite(LEVEL_ERROR, "Error %d listening on socket.", errno);
        return false;
    }

	return true;
}

//!
//! \brief Server main loop.
//! 
//! \note Based on select() for portability purposes. As long as the 
//! number of simultaneous clients is not too large this isn't a problem.
//!
//========================================================================
void SERVER::OnRun ()
{
    list<CLIENT*>::iterator i;

    while (!IsShutdown())
    {
        fd_set readfds, exceptfds;
        timeval ts;

        // COPY ACTIVE DESCRIPTORS FROM THE MASTER SET
		readfds = m_fds;
		exceptfds = m_fds;

        ts.tv_sec = 1;
        ts.tv_usec = 0;
        int n = select(m_highsock + 1, &readfds, 0, &exceptfds, &ts);
		if (n < 0)
		{
			LogWrite(LEVEL_ERROR, "Error %d in select()", errno);
		}
        
        // CHECK FOR AN INCOMING CONNECTION REQUEST
        if (FD_ISSET(m_sock, &readfds))
        {
            OnConnect();
        }

        // PROCESS CLIENT SOCKETS
        m_mutex.lock();
		i = m_clients.begin();
        while (i != m_clients.end())
        {
			bool drop = false;

			// DID AN ERROR OCCUR?
			if (!(*i)->IsDead() && FD_ISSET((*i)->GetSock(), &exceptfds))
			{
				drop = true;
			}

			// IS THERE DATA WAITING TO BE READ?
			if (!(*i)->IsDead() && FD_ISSET((*i)->GetSock(), &readfds))
			{
				if (!(*i)->ReceiveData())
				{
					drop = true;
				}
			}

			// IS THERE DATA WAITING TO BE TRANSMITTED?
			if (!drop && !(*i)->IsDead() && (*i)->TxPending())
			{
				if (!(*i)->TransmitData())
				{
					drop = true;
				}
			}

			// IS OUR TIME TOGETHER FINISHED?
			if (drop)
			{
				Drop(*i);
				i = m_clients.erase(i);
			}
			else
			{
				i++;
			}
        }
        m_mutex.unlock();
    }
}

//!
//! \brief Perform cleanup prior to server termination.
//!
//! This function drops all the clients and closes the master socket.
//!
//========================================================================
void SERVER::OnExit ()
{
	std::lock_guard<std::mutex> lk(m_mutex);

	if (m_sock != -1)
	{
		// CLOSE THE LISTENING SOCKET
		closesocket(m_sock);
		m_sock = -1;
	}


    // DROP ALL CLIENTS
    list<CLIENT*>::iterator i = m_clients.begin();
    while (i != m_clients.end())
    {
    	Drop(*i);
		delete *i;
    	i = m_clients.erase(i);
    }
}

//!
//! \brief Called when a new client attempts to connect.
//! Accepts the connection and adds a new client object to the client list.
//!
//! \retval bool Returns true if the connection was established.
//!
//========================================================================
bool SERVER::OnConnect ()
{
    struct sockaddr_in peeraddr;
    socklen_t addrlen;
    SOCKET s;

    addrlen = sizeof(peeraddr);
    s = accept(m_sock, (sockaddr*)&peeraddr, &addrlen);

    LogWrite(LEVEL_INFO, 
             "Incoming connection from %s.", 
             inet_ntoa(peeraddr.sin_addr));

    // ENABLE KEEPALIVES
    int optval = 1;
    int optlen = sizeof(optval);
    if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&optval), optlen) < 0) 
    {
        LogWrite(LEVEL_ERROR, "Cannot enable keepalive on socket.");
        closesocket(s);
        return false;
    }

    // ALLOCATE A CLIENT RECORD AND ADD IT TO THE LIST
    CLIENT* clientptr = NewClient(s);
    if (clientptr != 0)
    {
		std::lock_guard<std::mutex> lk(m_mutex);
        m_clients.push_back(clientptr);
    }
    else
    {
        LogWrite(LEVEL_ERROR, "Failed to allocate client record.");
        closesocket(s);
        return false;
    }

	// PERFORM NEW CLIENT ACTIONS
	OnNewClient(clientptr);

    return true;
}

//!
//! \brief Called when a new client is added to the client list.
//!
//! \param[in] client The newly-added client record.
//!
//========================================================================
void SERVER::OnNewClient(CLIENT* client)
{

	FD_SET(client->GetSock(), &m_fds); 
	if (client->GetSock() > m_highsock)
	{
		m_highsock = client->GetSock();
	}
}

//!
//! \brief Called when a client is disconnected from the server.
//!
//! \param[in] client The dearly-departed client record
//!
//========================================================================
void SERVER::OnDrop(CLIENT* client)
{

	FD_CLR(client->GetSock(), &m_fds);
}

//!
//! \brief Disconnect a client.
//! This function closes the client's socket and performs other cleanup.
//!
//! \param[in] client The client to disconnect.
//!
//========================================================================
void SERVER::Drop (CLIENT* client)
{

	OnDrop(client);
	client->Drop();
}

//!
//! \brief Send a message to all attached clients.
//!
//! \param[in] buf The message to send.
//! \param[in] len The number of bytes in the message.
//!
//! \retval bool Returns true if the operation was successful.
//!
//========================================================================
bool SERVER::Broadcast (const uint8_t* buf, uint8_t len)
{
	std::lock_guard<std::mutex> lk(m_mutex);
	bool result = true;

	// ITERATE THROUGH THE LIST OF CONNECTED CLIENTS, SENDING A COPY OF
	// THE DATA TO EACH ONE
	std::for_each(m_clients.begin(), 
		m_clients.end(), 
		[&buf, len, &result](CLIENT* c) { if (!c->Send(buf, len)) result = false; });

	return result;
}


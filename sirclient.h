/**
 * $DateTime: 2015/08/23 13:00:12 $                                                               
 * $Id: //depot/sircond/sirclient.h#1 $
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

#ifndef _SIRCLIENT_H_
#define _SIRCLIENT_H_

#include "pch.h"
#include "client.h"

class CSirServer;

//!
//! \brief A client object for a CSirServer
//!
//! This derived class provides client functionality specific to the 
//! SiriusConnect Daemon application.
//!
class CSirClient : public CLIENT
{
public:
	CSirClient (CSirServer& server, SOCKET socket, uint32_t bufsize);

protected:
    virtual uint32_t ProcessData (uint8_t* buf, uint32_t len);

private:
	CSirClient ();

	CSirServer& m_server;
};

#endif


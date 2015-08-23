/*
 * $DateTime: 2015/08/23 13:00:12 $                                                                
 * $Id: //depot/sircond/timetrax.cpp#12 $
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

//!
//! \file timetrax.cpp
//!
//! \brief Definitions for the CTTS100 class.
//!

#include "pch.h"
#include "timetrax.h"
#include "sircon.h"
#include "serial.h"

static const uint32_t MAX_AUTH_ATTEMPTS = 5;

//========================================================================
//!
//! \brief Query the attached TTS-100 for its version information
//!
//! \param[out] major Major version number
//! \param[out] minor Minor version number
//!
//! \retval bool Returns TRUE on success, or FALSE if an error occurred.
//!
//========================================================================
bool CTTS100::QueryVersion (uint32_t& major, uint32_t& minor)
{
    uint32_t buflen = 0;
    uint8_t buf[256];
    int32_t rc;

    major = minor = 0;

    SetDataRate(9600);

    // V IS FOR VERSION...
    uint8_t v = 'V';
    if (m_port->Send(&v, 1, 1000U) < 0)
    {
        return false;
    }

    LogWrite(LEVEL_DEBUG, "Version req sent...");

    // READ THE VERSION STRING
    buflen = 0;
    while ((buflen == 0) || (buf[buflen - 1] != '\n'))
    {
        LogWrite(LEVEL_DEBUG, "Reading %u bytes...", 256 - buflen);
        if ((rc = m_port->Recv(buf + buflen, 256 - buflen, 1000U)) < 0)
        {
			if (rc == sr::CSerialPort::ErrorTimeout)
			{
				LogWrite(LEVEL_ERROR, "No response to version request.");
			}
			else
			{
				LogWrite(LEVEL_ERROR, "Error %d reading from serial port!", rc);
			}
            break;
        }
        buflen += rc;
        LogWrite(LEVEL_DEBUG, "%d bytes read, %u staged.", rc, buflen);
        if (rc == 0U)
        {
        	break;
        }
    }

    LogWrite(LEVEL_DEBUG, "Response len = %u", buflen);

    // ATTEMPT TO PARSE THE RESPONSE
    if ((buflen < 9) || 
        !strstr(reinterpret_cast<const char*>(buf), "Time Trax"))
    {
       // NOT A TTS100?
        return false;  
    }

    // FIND VERSION
    char* p = strstr(reinterpret_cast<char*>(buf), "Version");
    if (!p)
    {
        // NOT A TTS100?
        return false;
    }

    // PARSE VERSION INFO
    p += 7;                 // SKIP PAST THE "Version"
    p = strtok(p, ". \r\n");
    if (p)
    {
        major = atoi(p);
        p = strtok(NULL, " \r\n");
        if (p)
        {
            minor = atoi(p);
        }
    }

    LogWrite(LEVEL_INFO, "TTS-100 version %u.%u detected.", major, minor);

    return true;
}

//========================================================================
//!
//! \internal
//! \brief Authenticate to the TTS-100
//!
//! The TTS-100 requires a special challenge-response dialog before the 
//! radio control interface is accessible. This was a lame attempt to
//! prevent third-party software from operating with the TTS-100.
//!
//! \retval bool Returns TRUE if the authentication process was successful.
//!
//========================================================================
bool CTTS100::Authenticate ()
{
	uint32_t buflen = 0;	// NUMBER OF BYTES CURRENTLY IN BUF
	uint8_t buf[256];		// A STAGING BUFFER FOR INCOMING DATA
	int32_t rc;				// A RESULT CODE FOR SERIAL PORT I/O

	// AUTHENTICATION TAKES PLACE AT 9600BPS
	SetDataRate(9600);

	// A IS FOR AUTHENTICATE...
	uint8_t a = 'A';
	if (m_port->Send(&a, 1, 1000U) < 0)
	{
		LogWrite(LEVEL_ERROR, "CTTS100::Authenticate(): Send auth request failed.");
		return false;
	}

	// READ THE CHALLENGE DATA
	buflen = 0;
	while (buflen < 15)
	{
		if ((rc = m_port->Recv(buf + buflen, 256 - buflen, 1000U)) <= 0)
		{
			break;
		}
		buflen += rc;
	}

	// VERIFY buf[0] == buf[1] == 0x3e
	if ((buflen < 15) || (buf[0] != 0x3e) || (buf[1] != 0x3e))
	{
		LogWrite(LEVEL_ERROR, "CTTS100::Authenticate(): Unexpected response from TTS100.");
		return false;
	}

	// START WITH A CANNED CHALLENGE RESPONSE
	uint8_t respbuf[21] = { 0x22, 0xF1, 0x59, 0x37, 0xF6, 0xA7, 0xFA, 0x3F,
					        0xD8, 0x5E, 0x27, 0x06, 0x0E, 0x39, 0xFA, 0xE8,
					        0x75, 0x29, 0x2E, 0x16, 0x50 };

	// REPLACE THE IMPORTANT BYTES BASED ON THE CHALLENGE DATA
	respbuf[18] = buf[2] ^ 0xad;
	respbuf[19] = buf[4] ^ 0x3a;

	if (m_port->Send(respbuf, 21, 1000U) < 0)
	{
		LogWrite(LEVEL_ERROR, "CTTS100::Authenticate(): Send auth response failed.");
		return false;
	}

	// GET THE RESULT
	buflen = 0;
	while (buflen < 3)
	{
		if ((rc = m_port->Recv(buf + buflen, 256 - buflen, 5000U)) <= 0)
		{
			break;
		}
		buflen += rc;
	}

	// LOOKING FOR "P<CR><LF>" RESPONSE...
	if ((buflen >= 3) && ((buf[0] == 0x50) || (buf[0] == 0x70)))
	{
		// 'P' IS FOR PROCEED
		LogWrite(LEVEL_INFO, "CTTS100::Authenticate(): Auth successful.");
		return true;
	}

	LogWrite(LEVEL_ERROR, "CTTS100::Authenticate(): Auth failed!");
	return false;
}

//========================================================================
//!
//! \internal
//! \brief Perform startup initialization
//!
//! Attempt to detect the presence of a TTS-100 interface. If one is 
//! detected, attempt to authenticate with it.
//!
//! \retval bool Returns TRUE if the initialization succeeded.
//!
//========================================================================
bool CTTS100::OnStart ()
{
	bool isTTS100 = false;
    uint32_t cnt = 0;

	// VERIFY THAT THE SERIAL PORT EXISTS
	if (m_port == 0)
	{
		LogWrite(LEVEL_CRITICAL, "Error accessing serial port - bailing.");
		return false;
	}
	
    // ATTEMPT TO AUTO-DETECT A TTS-100 INTERFACE
	LogWrite(LEVEL_INFO, "Checking for TimeTrax interface...");
    for (cnt = 0U; cnt < MAX_AUTH_ATTEMPTS; ++cnt)
    {
        uint32_t major, minor;

        isTTS100 = QueryVersion(major, minor);
		if (isTTS100)
		{
			break;
		}
    }

    // IF A TTS-100 WAS DETECTED, ATTEMPT TO AUTHENTICATE WITH IT
    if (isTTS100)
    {
		bool auth = false;

		for (cnt = 0U; !auth && (cnt < MAX_AUTH_ATTEMPTS); ++cnt)
		{
			if (Authenticate())
			{
				LogWrite(LEVEL_INFO, "TimeTrax initialization complete.");
				auth = true;
			}
			else
			{
				LogWrite(LEVEL_DEBUG, "TimeTrax authentication failed - retrying...");
				sleep(1);
			}
		}
		if (!auth)
		{
			LogWrite(LEVEL_CRITICAL, "Throwing up hands.");
			return false;
		}
    }
	else
	{
		LogWrite(LEVEL_INFO, "No TimeTrax interface was detected.");
	}

	return CSirCon::OnStart();
}

//!
//! @}
//!

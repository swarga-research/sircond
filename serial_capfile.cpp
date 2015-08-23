/**
 * $DateTime: 2015/08/23 13:00:12 $                            
 * $Author: khan $                                     
 * $Id: //depot/sircond/serial_capfile.cpp#1 $
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
#include "serial.h"

namespace sr
{

//========================================================================
class CapfileSerialPort : public CSerialPort
{
public:
	CapfileSerialPort ();
	int32_t Open (const string& device);
	int32_t SetDataRate (uint32_t baud);
	int32_t Send (const uint8_t* data, size_t size, uint32_t timeout);
	int32_t Recv (uint8_t* data, size_t maxSize, uint32_t timeout);
	void Close ();

private:
	~CapfileSerialPort ();

	HANDLE hSerial;
	OVERLAPPED m_recv_ol;
};


//========================================================================
int CapfileSerialPort::Open (const string& device)
{

	hSerial = CreateFile(device.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
	if (hSerial == INVALID_HANDLE_VALUE)
	{
		DWORD err = GetLastError();

		switch (err)
		{
			case ERROR_FILE_NOT_FOUND:
				return ErrorInvalidPort;
			break;

			case ERROR_ACCESS_DENIED:
				return ErrorPortInUse;
			break;

			case ERROR_PATH_NOT_FOUND:
				return ErrorInvalidPort;
			break;

			default:
				return ErrorUnspecified;
			break;
		}
	}

	return 0;
}

//========================================================================
int CapfileSerialPort::SetDataRate (unsigned baud)
{

	// ALWAYS SUCCESSFUL
	return 0;
}

//========================================================================
int32_t CapfileSerialPort::Send (const uint8_t* data, 
								 size_t size, 
								 uint32_t timeout)
{

	// FAKE IT
	return static_cast<int32_t>(size);
}

//========================================================================
int32_t CapfileSerialPort::Recv (uint8_t* data, 
								 size_t maxSize, 
								 uint32_t timeout)
{
	DWORD bytes = 0;

	if (!ReadFile(hSerial, data, maxSize, &bytes, &m_recv_ol))
	{
		DWORD err = GetLastError();

		if (err != ERROR_IO_PENDING)
		{
			return ErrorReceiveError;
		}

		// WAIT FOR THE I/O TO COMPLETE
		if (WaitForSingleObject(m_recv_ol.hEvent, timeout) == WAIT_TIMEOUT)
		{
			CancelIo(hSerial);
			return ErrorTimeout;
		}

		// GET THE RESULTS OF THE READ OPERATION
		GetOverlappedResult(hSerial, &m_recv_ol, &bytes, TRUE);
	}
	
	return static_cast<int32_t>(bytes);
}

//========================================================================
void CapfileSerialPort::Close ()
{

	if (hSerial != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hSerial);
		hSerial = INVALID_HANDLE_VALUE;
	}
}

//========================================================================
inline CapfileSerialPort::CapfileSerialPort () : hSerial(INVALID_HANDLE_VALUE)
{

	memset(&m_recv_ol, '\0', sizeof(m_recv_ol));
	m_recv_ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}

//========================================================================
CapfileSerialPort::~CapfileSerialPort ()
{

	Close();
	CloseHandle(m_recv_ol.hEvent);
}

//========================================================================
CSerialPort* CSerialPort::New ()
{

	return new CapfileSerialPort;
}

//========================================================================
CSerialPort::~CSerialPort ()
{

}

}

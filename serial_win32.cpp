/*
 * $DateTime: 2015/08/23 13:00:12 $                                                                
 * $Id: //depot/sircond/serial_win32.cpp#3 $
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

//!
//! \file serial_win32.cpp
//! \brief WIN32 implementation of the serial port abstraction class.
//!


namespace sr
{

//!
//! \brief WIN32 version of the serial port abstraction.
//!
//========================================================================
class WindowsSerialPort : public CSerialPort
{
public:
	WindowsSerialPort ();
	int32_t Open (const string& device);
	int32_t SetDataRate (uint32_t baud);
	int32_t Send (const uint8_t* data, size_t size, uint32_t timeout);
	int32_t Recv (uint8_t* data, size_t maxSize, uint32_t timeout);
	void Close ();

private:
	~WindowsSerialPort ();

	HANDLE hSerial;
	OVERLAPPED m_send_ol;
	OVERLAPPED m_recv_ol;
};


//========================================================================
int WindowsSerialPort::Open (const string& device)
{
	string dev = "\\\\.\\" + device;

	hSerial = CreateFile(dev.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
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
int WindowsSerialPort::SetDataRate (unsigned baud)
{

	// FLUSH OUT ANY OLD GARBAGE
	if (!FlushFileBuffers(hSerial))
	{
		return ErrorUnspecified;
	}
	if (!PurgeComm(hSerial, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR))
	{
		return ErrorUnspecified;
	}

	// CONFIGURE THE PORT
	DCB dcb = {0};
	if (!GetCommState(hSerial, &dcb))
	{
		return ErrorUnspecified;
	}
	dcb.BaudRate	= baud;
	dcb.ByteSize	= 8;
	dcb.StopBits	= ONESTOPBIT;
	dcb.Parity		= NOPARITY;
	if (!SetCommState(hSerial, &dcb))
	{
		if (GetLastError() == ERROR_INVALID_PARAMETER)
		{
			return ErrorInvalidSettings;
		}
		return ErrorUnspecified;
	}

	// SET TIMEOUT AT 50MS
	COMMTIMEOUTS timeouts;
	timeouts.ReadIntervalTimeout		= 50;
	timeouts.ReadTotalTimeoutConstant	= 0;
	timeouts.ReadTotalTimeoutMultiplier	= 0;
	timeouts.WriteTotalTimeoutConstant	= 0;
	timeouts.WriteTotalTimeoutMultiplier= 0;
	if (!SetCommTimeouts(hSerial, &timeouts))
	{
		return ErrorUnspecified;
	}

	// FLUSH AGAIN (FOR LUCK ;)
	if (!PurgeComm(hSerial, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR))
	{
		return ErrorUnspecified;
	}

	return 0;
}

//========================================================================
int32_t WindowsSerialPort::Send (const uint8_t* data, 
								 size_t size, 
								 uint32_t timeout)
{
	DWORD bytes = 0U;
	DWORD ticks = GetTickCount();

	if (!WriteFile(hSerial, data, size, &bytes, &m_send_ol))
	{
		DWORD err = GetLastError();

		if (err != ERROR_IO_PENDING)
		{
			return ErrorTransmitError;
		}

		// WAIT FOR THE I/O TO COMPLETE
		if (WaitForSingleObject(m_send_ol.hEvent, timeout) == WAIT_TIMEOUT)
		{
			CancelIo(hSerial);
			return ErrorTimeout;
		}

		GetOverlappedResult(hSerial, &m_send_ol, &bytes, TRUE);
	}

	DWORD delta = GetTickCount() - ticks;
	LogWrite(LEVEL_DEBUG, "TX: %u bytes in %u ms", bytes, delta);

	return static_cast<int32_t>(bytes);
}

//========================================================================
int32_t WindowsSerialPort::Recv (uint8_t* data, 
								 size_t maxSize, 
								 uint32_t timeout)
{
	DWORD bytes = 0;
	DWORD ticks = GetTickCount();

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
	
	DWORD delta = GetTickCount() - ticks;
	LogWrite(LEVEL_DEBUG, "RX: %u bytes in %u ms", bytes, delta);

	return static_cast<int32_t>(bytes);
}

//========================================================================
void WindowsSerialPort::Close ()
{

	if (hSerial != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hSerial);
		hSerial = INVALID_HANDLE_VALUE;
	}
}

//========================================================================
inline WindowsSerialPort::WindowsSerialPort () : hSerial(INVALID_HANDLE_VALUE)
{

	// INITIALIZE THE OVERLAPPED I/O DATA
	memset(&m_send_ol, '\0', sizeof(m_send_ol));
	m_send_ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	memset(&m_recv_ol, '\0', sizeof(m_recv_ol));
	m_recv_ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}

//========================================================================
WindowsSerialPort::~WindowsSerialPort ()
{

	Close();
	CloseHandle(m_recv_ol.hEvent);
	CloseHandle(m_send_ol.hEvent);
}

//========================================================================
CSerialPort* CSerialPort::New ()
{

	return new WindowsSerialPort;
}

//========================================================================
CSerialPort::~CSerialPort ()
{

}

}

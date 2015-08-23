/**
 * $DateTime: 2015/08/23 13:00:12 $                                                              
 * $Id: //depot/sircond/serial.h#6 $
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
//! \file serial.h
//! \brief Declarations for the serial port abstraction class.
//!


#ifndef _SERIAL_PORT_H_
#define _SERIAL_PORT_H_

namespace sr
{

//!
//! \brief Serial port abstraction class.
//!
//! This is a minimal abstraction for RS-232 (or USB) serial ports. Its 
//! purpose is to provide a common interface for software compiled on 
//! Linux and Win32 platforms. Link the implementation .cpp file appropriate 
//! to the platform into the executable.
//!
class CSerialPort
{
public:
	static CSerialPort* New ();		//!< SERIAL PORT FACTORY FUNCTION
	virtual int32_t Open (const string& device) = 0;
	virtual int32_t SetDataRate (unsigned baud) = 0;
	virtual int32_t Send (const uint8_t* data, size_t size, unsigned timeout) = 0;
	virtual int32_t Recv (uint8_t* data, size_t maxSize, unsigned timeout) = 0;
	virtual void Close () = 0;
	virtual ~CSerialPort () = 0;

	enum Errors
	{
		ErrorUnspecified	 = -100,	//!< UNKNOWN ERROR
		ErrorInvalidPort	 = -101,	//!< SERIAL DEVICE SPECIFIED IS NOT VALID OR DOES NOT EXIST
		ErrorPortInUse		 = -102,	//!< SERIAL DEVICE IS ALREADY IN USE
		ErrorInvalidSettings = -103,	//!< SPECIFIED SETTINGS ARE NOT VALID FOR THE DEVICE
		ErrorTransmitError	 = -104,	//!< AN ERROR OCCURRED DURING DATA TRANSMISSION
		ErrorReceiveError	 = -105,	//!< AN ERROR OCCURRED DURING DATA RECEPTION
		ErrorTimeout         = -106		//!< OPERATION TIMED OUT
	};
};

}

#endif

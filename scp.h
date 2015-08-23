/**
* $DateTime: 2015/08/23 13:00:12 $
* $Id: //depot/sircond/scp.h#1 $
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

#ifndef _SCP_H_
#define _SCP_H_

//!
//! \file scp.h
//! \brief Definitions related to the SiriusConnect Protocol
//!
//! The SiriusConnect Protocol (SCP) is a simple binary protocol used by
//! SiriusConnect satellite radio receivers. 
//!
//! \note This implementation is based on reverse-engineering, not an 
//! official protocol specification. Because of this it may contain errors
//! and omissions. All names and other identifiers are guesses based on 
//! apparent function and may not match the official names in the official 
//! protocol specification.
//!
//! SCP is a binary, frame-based, stop-and-wait link-level protocol. Each 
//! message frame begins with a header containing a frame start sentinel 
//! (0xA4) and several other fields including a message ID, a sequence 
//! number, and the length of the payload. Following this header is the 
//! mesage payload, which is in turn followed by a one-byte 2's compliment
//! checksum of all other bytes in the frame including the start sentinel. 
//!
//! SCP requires that the frame start sentinel (0xA4) can only appear at 
//! the beginning of a frame; any occurrences of this value elsewhere in
//! the message, including the checksum trailer byte,  are replaced with 
//! an escape sequence. The ESC character itself is replaced by the 
//! sequence ESC ESC.
//!

#include <stdint.h>

//! A channel index
typedef uint8_t SCP_CHANNEL_INDEX;

const uint8_t PKT_SENTINEL = 0xA4U;		//!< Frame start sentinel
const uint8_t ASCII_ESC    = 0x1BU;		//!< The ASCII ESCape character
const uint8_t SENTINEL_ESC = 0x53U;		//!< Second character int he sentinel escape sequence (~0xA4)

//! MAXIMUM SIZE OF A FRAME PAYLOAD (BYTES)
const uint32_t SCP_MAX_DATA = 255U;

//! MAXIMUM NUMBER OF CHANNELS
const uint8_t SCP_MAX_CHANNELS = 224U;

//! INVALID CHANNEL SENTINEL
const SCP_CHANNEL_INDEX SCP_INVALID_CHANNEL = 255U;

//! NUMBER OF BYTES IN THE CHANNEL BITMAP ARRAY
const uint8_t SCP_CHANNEL_BITMAP_SIZE = SCP_MAX_CHANNELS / 8U;

//! Message IDs
enum SCP_MSG_ID
{
	MSG_SET = 0x00,				//!< Set the value of a radio parameter
	MSG_SET_RESP = 0x20,		//!< Radio's response to a SET message
	MSG_GET = 0x40,				//!< Get the current value of a radio parameter
	MSG_GET_RESP = 0x60,		//!< Radio's reponse to a GET message
	MSG_ASYNC = 0x80			//!< An asynchronous notification message fro the radio
};

//!< Frame header flags
enum SCP_FLAGS
{
	SF_CHKSUM = 0x01U,	//!< Bad checksum
	SF_BUSY = 0x02U,	//!< Radio busy
	SF_ACK = 0x80U		//!< General acknowledgement
};

//! GET command IDs
enum SCP_GET_IDS
{
	SCP_GET_GAIN = 0x02,
	SCP_GET_MUTE = 0x03,
	SCP_GET_POWER = 0x07,
	SCP_GET_CHANNELINFO = 0x08,
	SCP_GET_CHANNEL = 0x0a,
	SCP_GET_SONGINFO = 0x0d,
	SCP_GET_CHANNEL_MAP = 0x10,
	SCP_GET_SID = 0x11,
	SCP_GET_TZINFO = 0x12,
	SCP_GET_TIME = 0x13,
	SCP_GET_ASYNC = 0x14,
	SCP_GET_STATUS = 0x16,
	SCP_GET_RSSI = 0x18
};

//! SET command IDs
enum SCP_SET_IDS
{
	SCP_SET_GAIN = 0x02,
	SCP_SET_MUTE = 0x03,
	SCP_SET_POWER = 0x08,
	SCP_SET_RESET = 0x09,
	SCP_SET_CHANNEL = 0x0a,
	SCP_SET_TZ_INFO = 0x0c,
	SCP_SET_ASYNC = 0x0d
};

//! Async event IDs
enum SCP_ASYNC_IDS
{
	SCP_ASYNC_RESET = 0x00,
	SCP_ASYNC_SONGINFO = 0x01,
	SCP_ASYNC_SONGID = 0x02,
	SCP_ASYNC_TIME = 0x03,
	SCP_ASYNC_STATUS = 0x04,
	SCP_ASYNC_SIGNAL = 0x05,
};

//! Async notification flag definitions
enum SCP_ASYNC_FLAGS
{
	AF_TIME = 0x01,				//!< Current local time
	AF_SIGNAL = 0x02,			//!< Signal strength
	AF_CHANNELINFO = 0x04,		//!< Information for current channel
	AF_ALLCHANNELINFO = 0x08,	//!< Information for all channels (overrides AF_SONGID)
	AF_SONGID = 0x10			//!< Song identifier string
};

//! Status type codes
enum SCP_STATUS_TYPE
{
	ST_TUNE = 0x00,				//!< TUNE TO CHANNEL COMPLETE
	ST_SIGNAL = 0x01,			//!< SIGNAL LOST/ACQUIRED
	ST_ANTENNA = 0x02			//!< ANTENNA CONNECTED/DISCONNECTED
};

//! Header structure for SCP frames
typedef struct SHDR
{
    uint8_t sentinel;    //!< 0xA4 - PKT_SENTINEL
    uint8_t unk2;        //!< 0x03 - Unknown, usually 0x03
    uint8_t unk3;        //!< 0x00 - Unknown, usually 0x00
    uint8_t seq;         //!< Frame sequence number
    uint8_t flags;       //!< Flags (SF_XXX)
    uint8_t len;         //!< Payload length (bytes)
} *SHDRPTR;

//! SCP date/time structure
typedef struct
{
	uint16_t year;		//!< Year (NBO)
	uint8_t mon;		//!< Month
	uint8_t day;		//!< Day of the month
	uint8_t hour;		//!< Hour
	uint8_t min;		//!< Minute
	uint8_t sec;		//!< Second
	uint8_t dow;		//!< Day of the week (0 == Sunday)
	uint8_t dst;		//!< Daylight savings time active flag (Boolean)
} SCP_DATETIME;

//! Mamimum size of an SCP frame
//! Allows enough room for a header, a full payload, and one checksum byte
const uint32_t SCP_MAX_PKT = sizeof(SHDR) + SCP_MAX_DATA + 1U;

//! Field tags for the SONGINFO message
enum SONGINFOTAG
{
	SIT_ARTIST = 0x01,
	SIT_TITLE = 0x02,
	SIT_ALBUM = 0x03,
	SIT_COMPOSER = 0x06,
	SIT_SONGID = 0x86,
	SIT_ARTISTID = 0x88,
	SIT_ERASE = 0xE0
};

#endif


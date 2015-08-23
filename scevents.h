/**
* $DateTime: 2015/08/23 13:00:12 $
* $Id: //depot/sircond/scevents.h#1 $
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

#ifndef _SCEVENTS_H_
#define _SCEVENTS_H_

//!
//! \file scevents.h
//! \brief Declarations for SiriusConnect event classes
//!

#include <iostream>
#include "scp.h"

//!
//! \brief Event (virtual) base class.
//!
//! All SiriusConnect event classes are derived from this base class. 
//! These objects are simple wrappers for the details of each event.
//!
struct SCEvent
{
	virtual size_t deserialize(uint8_t* data, size_t len) { return 0u; };
	friend std::ostream& operator<< (std::ostream& out, SCEvent e)
	{
		out << "INVALID";
		return out;
	}
};

//!
//! \brief Startup event
//!
struct SCEStartup : public SCEvent
{
	friend std::ostream& operator<< (std::ostream& out, SCEStartup e)
	{
		out << "STARTUP";
		return out;
	}
};

//!
//! \brief Result code from a GET request
//!
struct SCEGetResult : public SCEvent
{
	size_t deserialize(uint8_t* data, size_t len);
	friend std::ostream& operator<< (std::ostream& out, SCEGetResult e)
	{
		out << "GET," << static_cast<unsigned>(e.result);
		return out;
	}
	uint16_t result;
};

//!
//! \brief Result code from a SET request
//!
struct SCESetResult : public SCEvent
{
	size_t deserialize(uint8_t* data, size_t len);
	friend std::ostream& operator<< (std::ostream& out, SCESetResult e)
	{
		out << "SET," << static_cast<unsigned>(e.result);
		return out;
	}
	uint16_t result;
};

//!
//! \brief Sirius Radio ID
//!
//! Each Sirius radio has a unique identifier (Sirius ID, or SID).
//! The SID is a string of numeric ASCII characters which is used 
//! for subscription purposes. This event contains the attached 
//! radio's SID string.
//!
struct SCESiriusID : public SCEvent
{
	size_t deserialize(uint8_t* data, size_t len);
	friend std::ostream& operator<< (std::ostream& out, SCESiriusID e)
	{
		out << "SID," << e.sid;
		return out;
	}
	string sid;
};

//!
//! \brief Gain value
//!
struct SCEGain : public SCEvent
{
	size_t deserialize(uint8_t* data, size_t len);
	friend std::ostream& operator<< (std::ostream& out, SCEGain e)
	{
		out << "GAIN," << static_cast<int>(e.gain);
		return out;
	}
	int8_t gain;
};

//!
//! \brief Mute value
//!
struct SCEMute : public SCEvent
{
	size_t deserialize(uint8_t* data, size_t len);
	friend std::ostream& operator<< (std::ostream& out, SCEMute e)
	{
		out << "MUTE," << static_cast<unsigned>(e.mute);
		return out;
	}
	uint8_t mute;
};

//!
//! \brief Song ID
//!
//! Sirius provides a unique identifier for each song or program on
//! their service. This identifier consists of a short string of
//! printable ASCII characters.
//!
struct SCESongID : public SCEvent
{
	size_t deserialize(uint8_t* data, size_t len);
	friend std::ostream& operator<< (std::ostream& out, SCESongID e)
	{
		out << "SONGID," << "\"" << e.songid << "\"";
		return out;
	}
	string songid;
};

//!
//! \brief Song info
//!
struct SCESongInfo : public SCEvent
{
	size_t deserialize(uint8_t* data, size_t len);
	friend std::ostream& operator<< (std::ostream& out, SCESongInfo e)
	{
		out << "SONGINFO,";
		out << static_cast<unsigned>(e.channel) << ",";
		out << "\"" << e.song_id << "\",";
		out << "\"" << e.artist_id << "\",";
		out << "\"" << e.title << "\",";
		out << "\"" << e.artist << "\",";
		out << "\"" << e.composer << "\"";
		return out;
	}
	SCP_CHANNEL_INDEX channel;	//!< The channel where this song is playing
	string title;				//!< Song title
	string artist;				//!< Artist's name
	string album;				//!< Album name
	string composer;			//!< Composer's name 
	string song_id;				//!< Sirius song ID string
	string artist_id;			//!< Sirius artist ID string
};

//!
//! \brief Current channel number
//!
//! This event indicates the channel to which the radio is currently
//! tuned.
//!
struct SCEChannel : public SCEvent
{
	size_t deserialize(uint8_t* data, size_t len);
	friend std::ostream& operator<< (std::ostream& out, SCEChannel e)
	{
		out << "CHANNEL," << static_cast<unsigned>(e.channel);
		return out;
	}
	SCP_CHANNEL_INDEX channel;
};

//!
//! \brief Info for a channel
//!
struct SCEChannelInfo : public SCEvent
{
	size_t deserialize(uint8_t* data, size_t len);
	friend std::ostream& operator<< (std::ostream& out, SCEChannelInfo e)
	{
		out << "CHANNELINFO,";
		out << static_cast<unsigned>(e.channel) << ",";
		out << static_cast<unsigned>(e.genre) << ",";
		out << "\"" << e.lname << "\",";
		out << "\"" << e.sname << "\",";
		out << "\"" << e.lgenre << "\",";
		out << "\"" << e.sgenre << "\"";
		return out;
	}
	SCP_CHANNEL_INDEX channel;	//!< Sirius channel number
	uint8_t genre;				//!< Sirius genre code
	string sname;				//!< Short channel name
	string lname;				//!< Long channel name
	string sgenre;				//!< Short genre name
	string lgenre;				//!< Long genre name
};

//!
//! \brief Channel Map update event
//!
//! The Channel Map is a bitmap containing a single bit flag for
//! each of the 224 Sirius channels. A '1' bit indicates that the
//! corresponding channel is valid and tunable; a '0' bit indicates
//! an invalid channel. The bitmap is stored as a byte array in big-
//! endian order, i.e. the most significant bit of first byte corresponds 
//! to Sirius channel 223, and the least significant bit of the last byte 
//! corresponds to Sirius channel 0.
//!
struct SCEChannelMap : public SCEvent
{
	size_t deserialize(uint8_t* data, size_t len);
	friend std::ostream& operator<< (std::ostream& out, SCEChannelMap e)
	{
		out << "CHANNELMAP";
		return out;
	}
	uint8_t channel_map[SCP_CHANNEL_BITMAP_SIZE];	//!< Bitmap of valid channels
};

//!
//! \brief Status
//!
struct SCEStatus : public SCEvent
{
	size_t deserialize(uint8_t* data, size_t len);
	friend std::ostream& operator<< (std::ostream& out, SCEStatus e)
	{
		out << "STATUS,";
		out << static_cast<unsigned>(e.type) << ",";
		out << static_cast<unsigned>(e.status1) << ",";
		out << static_cast<unsigned>(e.status2);
		return out;
	}
	uint8_t type;		//!< Type of status
	uint8_t status1;	//!< Primary status code
	uint8_t status2;	//!< Secondary status
};

//!
//! \brief RSSI event
//!
struct SCERSSI : public SCEvent
{
	size_t deserialize(uint8_t* data, size_t len);
	friend std::ostream& operator<< (std::ostream& out, SCERSSI e)
	{
		out << "RSSI,";
		out << static_cast<unsigned>(e.composite) << ",";
		out << static_cast<unsigned>(e.satellite) << ",";
		out << static_cast<unsigned>(e.terrestrial);
		return out;
	}
	uint8_t composite;
	uint8_t satellite;
	uint8_t terrestrial;
};

//!
//! \brief Signal acquired/lost event
//!
struct SCESignal : public SCEvent
{
	size_t deserialize(uint8_t* data, size_t len);
	friend std::ostream& operator<< (std::ostream& out, SCESignal e)
	{
		out << "SIGNAL," << static_cast<unsigned>(e.signal);
		return out;
	}
	uint8_t signal;
};

//!
//! \brief Antenna connected/disconnected event
//!
struct SCEAntenna : public SCEvent
{
	size_t deserialize(uint8_t* data, size_t len);
	friend std::ostream& operator<< (std::ostream& out, SCEAntenna e)
	{
		out << "ANTENNA," << static_cast<unsigned>(e.antenna);
		return out;
	}
	uint8_t antenna;
};

//!
//! \brief Reset event
//!
struct SCEReset : public SCEvent
{
	friend std::ostream& operator<< (std::ostream& out, SCEReset e)
	{
		out << "RESET";
		return out;
	}
};

//!
//! \brief Current power setting
//!
struct SCEPower : public SCEvent
{
	size_t deserialize(uint8_t* data, size_t len);
	friend std::ostream& operator<< (std::ostream& out, SCEPower e)
	{
		out << "POWER," << static_cast<unsigned>(e.power);
		return out;
	}
	uint8_t power;
};

//!
//! \brief Time Zone settings
//!
struct SCETimeZoneInfo : public SCEvent
{
	size_t deserialize(uint8_t* data, size_t len);
	friend std::ostream& operator<< (std::ostream& out, SCETimeZoneInfo e)
	{

		out << "TZINFO," << static_cast<int>(e.offset) << "," << static_cast<unsigned>(e.dst);
		return out;
	}
	int16_t offset;		//!< Offset in minutes from UTC
	uint8_t dst;		//!< Daylight savings time flag (Boolean)
};

//!
//! \brief Time event
//!
struct SCETime : public SCEvent
{
	size_t deserialize(uint8_t* data, size_t len);
	friend std::ostream& operator<< (std::ostream& out, SCETime e)
	{
		out << "TIME,";
		out << static_cast<unsigned>(e.year) << ",";
		out << static_cast<unsigned>(e.mon) << ",";
		out << static_cast<unsigned>(e.day) << ",";
		out << static_cast<unsigned>(e.hour) << ",";
		out << static_cast<unsigned>(e.min) << ",";
		out << static_cast<unsigned>(e.sec) << ",";
		out << static_cast<unsigned>(e.dow) << ",";
		out << static_cast<unsigned>(e.dst);
		return out;
	}
	uint16_t year;		//!< Year
	uint8_t mon;		//!< Month
	uint8_t day;		//!< Day of the month
	uint8_t hour;		//!< Hour
	uint8_t min;		//!< Minute
	uint8_t sec;		//!< Second
	uint8_t dow;		//!< Day of the week (0 == Sunday)
	uint8_t dst;		//!< Daylight savings time active flag (Boolean)
};

//!
//! \brief Shutdown event
//!
struct SCEShutdown : public SCEvent
{
	friend std::ostream& operator<< (std::ostream& out, SCEShutdown e)
	{
		out << "SHUTDOWN";
		return out;
	}
};

#endif

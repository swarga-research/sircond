/*
* $DateTime: 2015/08/23 13:00:12 $
* $Id: //depot/sircond/scevents.cpp#1 $
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
#include "sircon.h"
#include "scevents.h"

//!
//! \file scevents.cpp
//! \brief Implementation of SiriusConnect event classes.
//!

//!
//! \brief Parse the contents 
//!
//! \param[in] data Pointer to the message data to be parsed
//! \param[in] len Length of the message data
//!
//! \retval size_t The number of bytes processed from the input buffer
//!
//========================================================================
size_t SCEGetResult::deserialize(uint8_t* data, size_t len)
{

	return len;
}

//!
//! \brief Parse the contents 
//!
//! \param[in] data Pointer to the message data to be parsed
//! \param[in] len Length of the message data
//!
//! \retval size_t The number of bytes processed from the input buffer
//!
//========================================================================
size_t SCESetResult::deserialize(uint8_t* data, size_t len)
{

	return len;
}

//!
//! \brief Parse the contents 
//!
//! \param[in] data Pointer to the message data to be parsed
//! \param[in] len Length of the message data
//!
//! \retval size_t The number of bytes processed from the input buffer
//!
//========================================================================
size_t SCESiriusID::deserialize(uint8_t* data, size_t len)
{

	return CopyPascalString(data, sid);
}

//!
//! \brief Parse the contents 
//!
//! \param[in] data Pointer to the message data to be parsed
//! \param[in] len Length of the message data
//!
//! \retval size_t The number of bytes processed from the input buffer
//!
//========================================================================
size_t SCEGain::deserialize(uint8_t* data, size_t len)
{

	gain = static_cast<int32_t>(*(reinterpret_cast<int8_t*>(data)));
	return 1u;
}

//!
//! \brief Parse the contents 
//!
//! \param[in] data Pointer to the message data to be parsed
//! \param[in] len Length of the message data
//!
//! \retval size_t The number of bytes processed from the input buffer
//!
//========================================================================
size_t SCEMute::deserialize(uint8_t* data, size_t len)
{

	mute = *data;
	return 1u;
}

//!
//! \brief Parse the contents 
//!
//! \param[in] data Pointer to the message data to be parsed
//! \param[in] len Length of the message data
//!
//! \retval size_t The number of bytes processed from the input buffer
//!
//========================================================================
size_t SCESongID::deserialize(uint8_t* data, size_t len)
{

	return CopyPascalString(data, songid);
}

//!
//! \brief Parse the contents 
//!
//! \param[in] data Pointer to the message data to be parsed
//! \param[in] len Length of the message data
//!
//! \retval size_t The number of bytes processed from the input buffer
//!
//========================================================================
size_t SCEChannel::deserialize(uint8_t* data, size_t len)
{

	channel = *data;
	return 1u;
}

//!
//! \brief Parse the contents 
//!
//! \param[in] data Pointer to the message data to be parsed
//! \param[in] len Length of the message data
//!
//! \retval size_t The number of bytes processed from the input buffer
//!
//========================================================================
size_t SCEStatus::deserialize(uint8_t* data, size_t len)
{

	type = data[0];
	status1 = data[1];
	status2 = data[2];
	return 3u;
}

//!
//! \brief Parse the contents 
//!
//! \param[in] data Pointer to the message data to be parsed
//! \param[in] len Length of the message data
//!
//! \retval size_t The number of bytes processed from the input buffer
//!
//========================================================================
size_t SCERSSI::deserialize(uint8_t* data, size_t len)
{

	composite = data[0];
	satellite = data[1];
	terrestrial = data[2];

	return 3u;
}

//!
//! \brief Parse the contents 
//!
//! \param[in] data Pointer to the message data to be parsed
//! \param[in] len Length of the message data
//!
//! \retval size_t The number of bytes processed from the input buffer
//!
//========================================================================
size_t SCEChannelMap::deserialize(uint8_t* data, size_t len)
{

	assert(len >= SCP_CHANNEL_BITMAP_SIZE);

	memcpy(channel_map, data, SCP_CHANNEL_BITMAP_SIZE);
	return SCP_CHANNEL_BITMAP_SIZE;
}

//!
//! \brief Parse the contents 
//!
//! \param[in] data Pointer to the message data to be parsed
//! \param[in] len Length of the message data
//!
//! \retval size_t The number of bytes processed from the input buffer
//!
//========================================================================
size_t SCESignal::deserialize(uint8_t* data, size_t len)
{

	signal = *data;
	return 1u;
}

//!
//! \brief Parse the contents 
//!
//! \param[in] data Pointer to the message data to be parsed
//! \param[in] len Length of the message data
//!
//! \retval size_t The number of bytes processed from the input buffer
//!
//========================================================================
size_t SCEAntenna::deserialize(uint8_t* data, size_t len)
{

	antenna = *data;
	return 1u;
}

//!
//! \brief Parse the contents 
//!
//! \param[in] data Pointer to the message data to be parsed
//! \param[in] len Length of the message data
//!
//! \retval size_t The number of bytes processed from the input buffer
//!
//========================================================================
size_t SCEPower::deserialize(uint8_t* data, size_t len)
{

	power = *data;
	return 1u;
}

//!
//! \brief Parse the contents of a song info SCP message into a SONGINFO
//! structure.
//!
//! \param[in] data Pointer to the message data to be parsed
//! \param[in] len Length of the message data
//!
//! \retval size_t The number of bytes processed from the input buffer
//!
//========================================================================
size_t SCESongInfo::deserialize(uint8_t* data, size_t len)
{
	size_t offset = 0;		// CURRENT OFFSET INTO THE DATA BUFFER
	uint32_t num_fields;	// NUMBER OF FIELDS CONTAINED IN THE MESSAGE

	// SANITY CHECK
	if (len < 1)
	{
		return 0;
	}

	// GET THE NUMBER OF FIELDS
	num_fields = data[offset++];

	// PROCESS EACH FIELD
	for (uint32_t i = 0; (i < num_fields) && (offset < len); ++i)
	{
		uint32_t tag = data[offset++];

		switch (tag)
		{
			case SIT_ARTIST:
				offset += CopyPascalString(data + offset, artist);
			break;

			case SIT_TITLE:
				offset += CopyPascalString(data + offset, title);
			break;

			case SIT_ALBUM:
				offset += CopyPascalString(data + offset, album);
			break;

			case SIT_COMPOSER:
				offset += CopyPascalString(data + offset, composer);
			break;

			case SIT_SONGID:
				offset += CopyPascalString(data + offset, song_id);
			break;

			case SIT_ARTISTID:
				offset += CopyPascalString(data + offset, artist_id);
			break;

			case SIT_ERASE:
			break;

			default:
				LogWrite(LEVEL_DEBUG, "Unknown Song Info field tag %u", tag);
				string tmpstr;
				offset += CopyPascalString(data + offset, tmpstr);
			break;
		}
	}
	return offset;
}


//!
//! \brief Parse the contents of a channel info SCP message into an 
//! SCEChannelInfo object.
//!
//! \param[in] data Pointer to the message data to be parsed
//! \param[in] len Length of the message data
//! \retval size_t The number of bytes processed from the input buffer
//!
//========================================================================
size_t SCEChannelInfo::deserialize(uint8_t* data, size_t len)
{
	size_t offset = 0u;

	channel = data[offset++];
	genre = data[offset++];

	// SKIP PAST THE UNKNOWN STUFF
	offset = 5;

	// PARSE SHORT CHANNEL NAME
	offset += CopyPascalString(data + offset, sname);

	// PARSE LONG CHANNEL NAME
	offset += CopyPascalString(data + offset, lname);

	// PARSE SHORT GENRE NAME
	offset += CopyPascalString(data + offset, sgenre);

	// PARSE LONG GENRE NAME
	offset += CopyPascalString(data + offset, lgenre);

	return offset;
}

//!
//! \brief Parse the contents of a time zone info SCP message into an 
//! SCETimeZoneInfo object.
//!
//! \param[in] data Pointer to the message data to be parsed
//! \param[in] len Length of the message data
//! \retval size_t The number of bytes processed from the input buffer
//!
//========================================================================
size_t SCETimeZoneInfo::deserialize(uint8_t* data, size_t len)
{

	offset = data[0] << 8 | data[1];
	dst = data[2];

	return 3u;
}

//!
//! \brief Parse the contents of a time zone info SCP message into an 
//! SCETimeZoneInfo object.
//!
//! \param[in] data Pointer to the message data to be parsed
//! \param[in] len Length of the message data
//! \retval size_t The number of bytes processed from the input buffer
//!
//========================================================================
size_t SCETime::deserialize(uint8_t* data, size_t len)
{
	SCP_DATETIME* dt = reinterpret_cast<SCP_DATETIME*>(data);

	year = ntohs(dt->year);
	mon = dt->mon;
	day = dt->day;
	hour = dt->hour;
	min = dt->min;
	sec = dt->sec;
	dow = dt->dow;
	dst = dt->dst;
	return (sizeof(*dt));
}

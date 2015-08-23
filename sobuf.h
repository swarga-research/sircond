/**
 * $DateTime: 2015/08/23 13:00:12 $                                                               
 * $Id: //depot/sircond/sobuf.h#3 $
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

#ifndef _SOBUF_H_
#define _SOBUF_H_

//!
//! \file SOBUF.H
//!
//! \brief Declarations for the Slide-Over Buffer class.
//!

#include <stdint.h>

namespace sr
{

//!
//! \brief The Slide-Over Buffer class.
//! Slide-Over Buffers are containers for contiguous blocks 
//! of octet values such as frames, packets, messages, or
//! other PDUs. SOBuffers are so named because the data in 
//! the buffer "slides over" to make room for more
//! incoming data. SOBuffers maintain the data internally as
//! a contiguous block of bytes with no splits or pointer 
//! wrap-arounds to deal with, simplifying message processing
//! on the receiver side.
//!
class SOBuffer
{
public:
	SOBuffer() = delete;
	SOBuffer (size_t size);
	~SOBuffer () { if (m_buf != nullptr) delete [] m_buf; }
	bool Resize (size_t newsize);
	uint8_t* GetReadPtr () { return m_bufpos; }
	size_t GetReadLen () { return m_buflen; }
	void MarkRead (size_t len);
	uint8_t* GetWritePtr() { return (m_bufpos + m_buflen); }
	size_t GetWriteLen ();
	void MarkWritten (size_t len);
	void Clear () { MarkRead(GetReadLen()); }

private:
	size_t m_size;			//!< The maximum number of bytes which can be stored in the buffer
	uint8_t* m_buf;			//!< The buffer where the data is stored
	uint8_t* m_bufpos;		//!< The current read position within the buffer
	size_t m_buflen;		//!< The number of bytes currently stored in the buffer
	uint32_t m_slidecnt;	//!< DEBUG/TUNING: The number of times the buffer contents have been slid
};

}

#endif

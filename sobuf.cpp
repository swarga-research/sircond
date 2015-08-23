/*
 * $DateTime: 2015/08/23 13:00:12 $                                                                
 * $Id: //depot/sircond/sobuf.cpp#3 $
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
#include <string.h>
#include "sobuf.h"

namespace sr
{

//!
//! \brief Construct a SOBuffer of the specified size
//!
//! \param[in] size The number of bytes the buffer should hold
//!
//========================================================================
SOBuffer::SOBuffer(size_t size) : m_size(size), m_buf(0), m_bufpos(0), m_buflen(0), m_slidecnt(0u)
{

	m_buf = new uint8_t[m_size];
	m_bufpos = m_buf;
}

//!
//! \brief Resize a SOBuffer while preserving the contents
//!
//! \param[in] newsize The new maximum size of the buffer
//!
//! \retval bool Returns true if the buffer was successfully resized
//!
//! \note If the new size is smaller than the number of bytes currently
//! stored in the buffer, the buffer contents are truncated
//!
//========================================================================
bool SOBuffer::Resize (size_t newsize)
{
	uint8_t* bufptr = new uint8_t[newsize];

	if (bufptr != nullptr)
	{
		m_buflen = (m_buflen > newsize) ? newsize : m_buflen;
		memcpy(bufptr, m_bufpos, m_buflen);
		delete [] m_buf;
		m_bufpos = m_buf = bufptr;
	}
	return (bufptr != nullptr);
}

//!
//! \brief Advance the current read position within the buffer by the 
//! specified number of octets.
//!
//! \param[in] len The number of octets read from the buffer.
//!
//========================================================================
void SOBuffer::MarkRead(size_t len)
{

	assert(len <= m_buflen);

	m_bufpos += len;
	m_buflen -= len;
	if (m_buflen == 0u)
	{
		// THE BUFFER IS EMPTY, SO RESET THE READ POSITION BACK TO THE
		// START OF THE BUFFER
		m_bufpos = m_buf;
	}
}

//!
//! \brief Returns the number of octets available to be written 
//!
//! \retval size_t The available space remaining in the buffer
//!
//========================================================================
size_t SOBuffer::GetWriteLen ()
{
	size_t len = m_size - (m_bufpos - m_buf) - m_buflen;

	if (len == 0u)
	{
		// SLIDE OVER, BABY!
		memmove(m_buf, m_bufpos, m_buflen);
		m_bufpos = m_buf;
		len = m_size - m_buflen;
		m_slidecnt++;
		LogWrite(LEVEL_DEBUG, "SOB! %u", m_slidecnt);
	}

	return len;
}

//!
//! \brief Advance the current write position within the buffer by the 
//! specified number of octets.
//!
//! \param[in] len The number of octets added to the buffer.
//!
//========================================================================
void SOBuffer::MarkWritten (size_t len)
{

	assert(len <= GetWriteLen());

	m_buflen += len;
}

}

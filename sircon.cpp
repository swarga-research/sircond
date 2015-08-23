/*  
 * $DateTime: 2015/08/23 13:00:12 $
 * $Id: //depot/sircond/sircon.cpp#17 $
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
//! \file sircon.cpp
//! \brief Implementation of the SiriusConnect radio interface class.
//!

#include "pch.h"
#include "sircon.h"

//!< Number of 100ms timer ticks to wait for a busy-retransmit
static const uint32_t BUSY_DELAY_COUNT = 1;

//!< Maximum number of seconds which can elapse without link traffic
static const uint32_t LINK_TIMEOUT = 30;

//!
//! \brief Public constructor
//!
//! \param[in] device The name of the serial port device to use
//!
//========================================================================
CSirCon::CSirCon (const string& device) :
	m_port(nullptr),
	m_stagebuf(SCP_STAGEBUFSIZE),
	m_last_rx(0),
	m_busy_timer(0),
	m_seq(0u),
	m_last_seq(-1),
	m_seq_expected(0u),
	m_htimer(sr::INVALID_TIMER_HANDLE_VALUE),
	m_curr_channel(SCP_INVALID_CHANNEL),
	m_link_alive(false),
	m_link_fail_cnt(0u)
{

	m_port = sr::CSerialPort::New();
    if (m_port != 0)
    {
		if (Open(device.c_str()))
		{
    		m_last_rx = time(0);
		}
		else
		{
			delete m_port;
			m_port = 0;
		}
    }
    memset(m_channel_map, '\0', sizeof(m_channel_map));
}

//========================================================================
CSirCon::~CSirCon ()
{

	if (m_port != 0)
	{
		delete m_port;
		m_port = 0;
	}
}

//!
//! \brief Returns the time elapsed since a message was received from 
//! the radio
//!
//! \retval uint32_t Number of seconds since the last frame was received
//!
//========================================================================
uint32_t CSirCon::GetTimeSinceLastRx ()
{
	uint32_t retval = 0U;

	if (m_last_rx != 0)
	{
		time_t now = time(0);

		retval = static_cast<uint32_t>(difftime(now, m_last_rx));
	}

	return retval;
}

//!
//! \brief Open the serial port to the radio
//!
//! \param[in] device The name of the device to use
//!
//! \retval bool Returns true if the port was opened successfully
//!
//========================================================================
bool CSirCon::Open (const char* device)
{

	return ((m_port != 0) && (m_port->Open(device) == 0));
}

//!
//! \brief Close the serial port
//!
//========================================================================
void CSirCon::Close ()
{

	if (m_port != 0)
	{
		m_port->Close();
	}
}

//!
//! \brief Read data from the serial port, translating escape sequences
//!
//! \param[in,out] buf A pointer to the output buffer 
//! \param[in] maxlen Size (in bytes) of the output buffer
//! \param[out] bytesread Number of bytes actually read
//!
//! \retval bool Returns true if successful, or false if an error occurs
//!
//========================================================================
bool CSirCon::Read (uint8_t* buf, uint32_t maxlen, uint32_t* bytesread)
{
	static bool inESC = false;  // TRUE IF CURRENTLY IN AN ESCAPE SEQUENCE
    uint8_t tmpbuf[SCP_STAGEBUFSIZE];
    int32_t bytes = 0;

    assert(m_port != 0);

    *bytesread = 0;

    // READ THE RAW DATA FROM THE RADIO INTO A TEMPORARY BUFFER
    if ((bytes = m_port->Recv(tmpbuf, maxlen, 100U)) < 0)
    {
		if (bytes == sr::CSerialPort::ErrorTimeout)
		{
			return true;
		}
		LogWrite(LEVEL_ERROR, "Read() returned %d", bytes);
		return false;
    }

    // COPY THE RECEIVED DATA, DE-ESCAPING AS NEEDED
    for (int32_t i = 0; i < bytes; ++i)
    {
		// CHECK FOR ESCAPE SEQUENCE
        if (tmpbuf[i] == ASCII_ESC)
        {
            if (inESC)
            {
                // THIS IS AN ESCAPED ESC
                buf[(*bytesread)++] = ASCII_ESC;
                inESC = false;
            }
            else
            {
                // THIS IS THE FIRST BYTE OF AN ESCAPE SEQUENCE
                inESC = true;
            }
        }
        else
        {
            if (inESC)
            {
                switch (tmpbuf[i])
                {
                    case SENTINEL_ESC:
                        buf[(*bytesread)++] = PKT_SENTINEL;
                    break;

                    default:
                        LogWrite(LEVEL_ERROR, 
                                 "Unknown escape sequence 0x1b 0x%02x", 
                                 tmpbuf[i]);
                    break;
                }
                inESC = false;
            }
            else
            {
                buf[(*bytesread)++] = tmpbuf[i];
            }
        }
    }

    if (bytes > 0)
    {
    	m_last_rx = time(0);
    }
    return true;
}

//!
//! \brief Transmit a frame to the radio, inserting escape sequences as needed
//!
//! \param[in] buf A pointer to the message frame
//! \param[in] len The length of the frame (bytes)
//!
//! \retval bool Returns true if successful, or false if an error occurs
//!
//! \note The first character in the buffer is not escaped.
//!
//========================================================================
bool CSirCon::Write (uint8_t* buf, uint32_t len)
{
    uint8_t tmpbuf[SCP_STAGEBUFSIZE];	// STORAGE FOR TRANSLATED OUTGOING DATA
    uint32_t n = 0u;			// MESSAGE LENGTH AFTER TRANSLATION

    // COPY THE DATA INTO THE TEMPORARY BUFFER, ADDING ESCAPE
    // SEQUENCES AS NECESSARY
    for (uint32_t i = 0; (i < len) && (n < SCP_STAGEBUFSIZE); ++i)
    {
        switch (buf[i])
        {
            case PKT_SENTINEL:
                // A PACKET SENTINEL APPEARING ANYWHERE OTHER THAN THE
                // START OF THE PKT GETS ESCAPED
                if (i > 0)
                {
					tmpbuf[n++] = ASCII_ESC;
					tmpbuf[n++] = SENTINEL_ESC;
                }
                else
                {
					tmpbuf[n++] = buf[i];
                }
            break;

            case ASCII_ESC:
                // ESC BECOMES ESC-ESC
                tmpbuf[n++] = ASCII_ESC;
                tmpbuf[n++] = ASCII_ESC;
            break;

            default:
                // JUST COPY THE BYTE AS-IS
                tmpbuf[n++] = buf[i];
            break;
        }
    }

	// TRANSMIT THE TRANSLATED DATA TO THE RADIO
    int32_t result = m_port->Send(tmpbuf, n, 100U);
	if (result != static_cast<int32_t>(n))
	{
		LogWrite(LEVEL_ERROR, "CSirCon::Write(): Send() error %d", result);
	}

	return (result == static_cast<int32_t>(n));
}

//!
//! \internal
//! \brief Calculate the checksum for a message frame
//!
//! The SiriusConnect protocol uses a standard 2's
//! complement checksum computed across all bytes of
//! the frame.
//!
//! \param[in] data A pointer to the frame data
//! \param[in] len The length (in bytes) of the frame
//!
//! \retval uint8_t The calculated checksum
//!
//========================================================================
uint8_t CSirCon::ComputeChecksum (const uint8_t* data, uint32_t len)
{
    uint8_t sum = 0;

    for (uint32_t i = 0; i < len; ++i)
    {
        sum += data[i];
    }
    return (~sum + 1);
}

//!
//! \internal
//! \brief Validate the checksum for a message frame
//!
//! The SiriusConnect protocol uses a standard 2's
//! complement checksum computed across all bytes of
//! the frame. If the calculated sum of all frame bytes
//! is added tot he checksum byte, the result will be 0
//! unless an error occurred during transmission.
//!
//! \param[in] data A pointer to the frame data
//! \param[in] len The length (in bytes) of the frame
//!
//! \retval bool Returns true if the checksum is correct
//!
//========================================================================
bool CSirCon::ValidateChecksum (const uint8_t* data, uint32_t len)
{
    uint8_t sum = 0u;

    for (uint32_t i = 0u; i < len; ++i)
    {
        sum += data[i];
    }
    return (sum == 0u);
}

//!
//! \brief Transmits the SCP message frame to the radio.
//!
//! \note Assumes the caller is holding the THB mutex
//!
//========================================================================
bool CSirCon::TransmitFrame (MSGBUFPTR bufptr)
{

    if (!Write(bufptr->data, bufptr->len))
    {
        return false;
    }
    return true;
}

//!
//! \brief Construct an outgoing message frame
//!
//! \param[in] hdrbuf Points to the header for the message
//! \param[in] databuf Points to the message payload
//! \param[in] datalen The length of th epayload (bytes)
//!
//! \retval MSGBUFPTR A pointer to the constructed message frame.
//!
//========================================================================
MSGBUFPTR CSirCon::ComposeFrame(const uint8_t* databuf, uint32_t datalen)
{

	assert(datalen <= SCP_MAX_DATA);

	MSGBUFPTR bufptr = BufAlloc();
	if (bufptr != nullptr)
	{
		SHDR* hdrptr = reinterpret_cast<SHDR*>(bufptr->data);

		// PREPARE A HEADER FOR THE FRAME
		hdrptr->sentinel = PKT_SENTINEL;
		hdrptr->unk2 = 0x03;
		hdrptr->unk3 = 0x00;
		bufptr->seq = hdrptr->seq = m_seq++;
		hdrptr->flags = 0x00;
		hdrptr->len = static_cast<uint8_t>(datalen);

		bufptr->len = sizeof(*hdrptr);

		// NOW ADD THE PAYLOAD
		uint8_t* bufpos = bufptr->data + bufptr->len;
		for (uint32_t i = 0; i < datalen; ++i)
		{
			*bufpos++ = databuf[i];
			bufptr->len++;
		}

		// TACK ON THE CHECKSUM
		*bufpos++ = ComputeChecksum(bufptr->data, bufptr->len);
		bufptr->len++;
	}

	return bufptr;
}

//!
//! \brief Send an acknowledgement frame to the radio
//!
//! SCP acknowledgement frames can either be positive (indicating 
//! successful reception) or negative (indicating a checksum error
//! or that the radio is busy); The type of acknowledgement to send 
//! is indicated by the flags parameter.
//!
//! \param[in] ack Sequence number of the frame being acknowledged
//! \param[in] flags Acknowledgement flags (SF_XXX) 
//!
//! \retval bool Returns true if the frame was successfully transmitted
//!
//========================================================================
bool CSirCon::SendACK (uint8_t ack, uint8_t flags)
{
    SHDRPTR hdrptr;
    uint8_t buf[32];

    hdrptr = reinterpret_cast<SHDRPTR>(buf);
    hdrptr->sentinel = PKT_SENTINEL;
    hdrptr->unk2 = 0x03;
    hdrptr->unk3 = 0x00;
    hdrptr->seq = ack;
    hdrptr->flags = flags;
    hdrptr->len = 0;
    buf[sizeof(*hdrptr)] = ComputeChecksum(buf, sizeof(*hdrptr));

    return (Write(buf, sizeof(*hdrptr) + 1));
}

//!
//! \brief Static wrapper for the timer callback
//!
//! \param param[in] Aliased pointer to the CSirCon object
//!
//========================================================================
void CSirCon::TimerProcWrapper (void* param)
{
    CSirCon* pSirCon = reinterpret_cast<CSirCon*>(param);

    if (pSirCon != NULL)
    {
        pSirCon->TimerProc();
    }
}

//!
//! \brief Timer callback function 
//!
//! This function is invoked periodically from the timer thread to 
//! (re)transmit pending frames to the radio.
//!
//========================================================================
void CSirCon::TimerProc ()
{

    // IF THE TUNER IS BUSY, CONTNUE TO STALL
    if (m_busy_timer > 0)
    {
        m_busy_timer--;
        return;
    }

	m_queue_lock.lock();
	if (!m_queue.empty())
	{
		MSGBUFPTR bufptr = m_queue.front();

		if (++bufptr->retries > SIRCON_MAX_RETRIES)
		{
			// GIVING UP ON THIS FRAME
			m_queue.pop();
			OnTimeout(bufptr);
			BufFree(bufptr);
		}
		else
		{
			LogWrite(LEVEL_DEBUG, "Transmitting frame %02x...", bufptr->seq);
			TransmitFrame(bufptr);
		}
	}
	m_queue_lock.unlock();
}

//!
//! \brief Handler for frame acknowledgements.
//!
//========================================================================
void CSirCon::OnACK(MSGBUFPTR bufptr)
{

	LogWrite(LEVEL_DEBUG, "ACK received for seq %02x", bufptr->seq);

	// INFORM THE APPLICATION OF THE RESULT
	bufptr->result.set_value(SCR_SUCCESS);
}

//!
//! \brief Handler for link timeout events.
//!
//! In general the radio acknowledges transmitted frames within 100ms.
//! If no ACK is received within that time, the frame is retransmitted
//! and a counter is incremented.
//!
//! A link timeout occurs (and this handler is invoked) when a frame 
//! is not acknowledged by the radio after all retransmission attempts 
//! have been exhausted. 
//!
//! Link timeouts sometimes occur even when the radio is functioning
//! normally, so we will tolerate a certain number of link timeouts
//! before giving up and declaring the link completely dead. Once that
//! happens, the CSirCon object (and eventually the entire daemon) will 
//! terminate.
//!
//========================================================================
void CSirCon::OnTimeout (MSGBUFPTR bufptr)
{

	LogWrite(LEVEL_DEBUG, "Transmission timed out for seq %02x", bufptr->seq);

	// INFORM THE APPLICATION OF THE RESULT
	bufptr->result.set_value(SCR_TIMEOUT);

	m_link_alive = false;
	if (++m_link_fail_cnt > SIRCON_MAX_LINK_FAILURES)
	{
		LogWrite(LEVEL_CRITICAL, "Max failure count exceeded - exiting.");
		Shutdown();
	}
}

//!
//! \brief Performs one-time initialization for the SiriusConnect 
//! interface.
//!
//! \retval bool Returns true if all initialization was successful, or
//! false if an error occurred.
//!
//========================================================================
bool CSirCon::OnStart ()
{

	if (m_port == 0)
	{
		LogWrite(LEVEL_CRITICAL, "Could not open serial port.");
		return false;
	}

    // CREATE THE RETRANSMISSION TIMER
    m_htimer = m_timer.Create(100, this, TimerProcWrapper);
    m_timer.Start();

    // RESET THE RADIO
    SetDataRate(57600);

	// NOTIFY THE APPLICATION
	SCEStartup s;
	Notify(s);

    return true;
}

//!
//! \brief Main loop of the CSirCon object
//!
//! The primary activity of this function is to process incoming data
//! from the radio.
//!
//========================================================================
void CSirCon::OnRun ()
{

    while (!IsShutdown())
    {
        uint32_t resync = 0U;
        uint32_t bytes;

        assert(m_stagebuf.GetWriteLen() > 0U);

		// RETRIEVE ALL AVAILABLE DATA FROM THE RADIO
        if (!Read(m_stagebuf.GetWritePtr(), m_stagebuf.GetWriteLen(), &bytes))
        {
            break;
        }

        m_stagebuf.MarkWritten(bytes);

        if (bytes > 0)
        {
            LogWrite(LEVEL_DEBUG, 
                     "CSirCon::OnRun(): Read %u bytes, %u staged.", 
                     bytes, 
                     m_stagebuf.GetReadLen());

			if (!m_link_alive)
			{
				// BACK FROM THE DEAD!
				m_link_alive = true;
				m_link_fail_cnt = 0u;
			}
        }
        else
        {
        	if (GetTimeSinceLastRx() > LINK_TIMEOUT)
			{
				// IT's BEEN A WHILE SINCE WE'VE HEARD FROM THE RADIO
				// SEE IF IT'S STILL ALIVE
				if (m_link_alive)
				{
					// SEND AN INNOCUOUS REQUEST AS A KEEPALIVE PROBE
					GetRSSI();

					// AND RESET THE ACTIVITY TIMER
					m_last_rx = time(0);
				}
			}
        }

        // PARSE ALL AVAILABLE MESSAGES
		while (m_stagebuf.GetReadLen() >= sizeof(SHDR))
        {
            SHDRPTR hdrptr = reinterpret_cast<SHDRPTR>(m_stagebuf.GetReadPtr());

            // ARE WE ALIGNED WITH THE BEGINNING OF A MESSAGE?
            if (hdrptr->sentinel != PKT_SENTINEL)
            { 
				m_stagebuf.MarkRead(1U);
                resync++;
                continue;
            }

			// ORDINARILY THIS COUNT SHOULD ALWAYS BE 0
			// IF >0 IT PROBABLY INDICATES CORRUPTION DURING TRANSMISSION 
			// (E.G. NOISE OR DROPPED CHARACTERS ON THE SERIAL PORT)
            if (resync > 0U)
            {
				LogWrite(LEVEL_WARNING, "Resync bytes = %u", resync);
                resync = 0;
            }

            // DO WE HAVE THE COMPLETE MESSAGE?
            uint32_t msglen = sizeof(*hdrptr) + hdrptr->len + 1;
            if (msglen > m_stagebuf.GetReadLen())
            {
                break;
            }

            // VERIFY THE CHECKSUM
            if (ValidateChecksum(m_stagebuf.GetReadPtr(), msglen))
            {
                LogWrite(LEVEL_DEBUG, 
                         "PKT RX: seq %02X, flags 0x%02X, len %u", 
                         hdrptr->seq, 
                         hdrptr->flags, 
                         hdrptr->len);

                // WAS THIS AN ACKNOWLEDGEMENT?
                if (hdrptr->flags & SF_ACK)
                {
					LogWrite(LEVEL_DEBUG, 
							 "Received ACK %02x for sequence %02x", 
							 hdrptr->flags,
							 hdrptr->seq);

					m_queue_lock.lock();
					if (!m_queue.empty())
					{
						MSGBUFPTR bufptr = m_queue.front();
						if (hdrptr->seq == bufptr->seq)
						{
							if (hdrptr->flags & SF_CHKSUM)
							{
								// CRC CHECK FAILED - RETRANSMIT IMMEDIATELY
								LogWrite(LEVEL_DEBUG, "Bad CRC reported - resending now...");
								TransmitFrame(bufptr);
							}
							else if (hdrptr->flags & SF_BUSY)
							{
								// RADIO IS BUSY - SCHEDULE A RETRANSMISSION ON THE NEXT TICK
								LogWrite(LEVEL_DEBUG, "Radio is busy, resending later...");
								m_busy_timer = BUSY_DELAY_COUNT;
							}
							else
							{
								// RADIO RECEIVED THE FRAME OK
								OnACK(bufptr);
								m_queue.pop();
								BufFree(bufptr);
							}
						}
					}
					m_queue_lock.unlock();
                }
                else
                {
                    LogWrite(LEVEL_DEBUG, "Sending ACK for sequence %02x", hdrptr->seq);
                    SendACK(hdrptr->seq, SF_ACK);

					if (hdrptr->seq != m_last_seq)
					{
						Dispatch(m_stagebuf.GetReadPtr() + sizeof(SHDR), hdrptr->len);
						if (hdrptr->seq != m_seq_expected)
						{
							LogWrite(LEVEL_DEBUG, "Sequence error: expected %u, actual %u", static_cast<unsigned>(m_seq_expected), static_cast<unsigned>(hdrptr->seq));
						}
					}
					else
					{
						LogWrite(LEVEL_DEBUG, "Ignoring duplicate frame.");
					}
					m_last_seq = hdrptr->seq;
					m_seq_expected = (hdrptr->seq + 1) % 256;

                    // RESET THE BUSY TIMER
                    m_busy_timer = 0;
                }
            }
            else
            {
                LogWrite(LEVEL_DEBUG, "Invalid chksum!");
				SendACK(hdrptr->seq, SF_ACK | SF_CHKSUM);
                msglen = 1;                   // FORCE A RESYNC
            }

            // ADVANCE TO THE NEXT MESSAGE
			m_stagebuf.MarkRead(msglen);
			LogWrite(LEVEL_DEBUG, "%u bytes consumed, %u remaining.", msglen, m_stagebuf.GetReadLen());
        }
    }
    LogWrite(LEVEL_DEBUG, "CSirCon::OnRun() exiting.");
}

//========================================================================
void CSirCon::OnExit ()
{

	// NOTIFY THE APPLICATION THAT WE ARE SHUTTING DOWN
	SCEShutdown s;
	Notify(s);

	m_timer.Stop();

    // FREE ALL MEMORY USED FOR MESSAGE BUFFERS
    m_queue_lock.lock();
    while (!m_queue.empty())
    {
        MSGBUFPTR bufptr = m_queue.front();
        m_queue.pop();
        delete bufptr;
    }
    m_queue_lock.unlock();

    Close();
}

//!
//! \brief Allocate an SCP frame buffer
//!
//! This function allocates a frame buffer from the spare buffer pool
//! (if available) or from the runtime heap. The contents of the 
//! buffer are cleared before returning the buffer to the caller.
//!
//! \retval MSGBUFPTR A pointer to the allocated buffer. If a buffer
//! could not be allocated then nullptr is returned.
//!
//========================================================================
MSGBUFPTR CSirCon::BufAlloc()
{
	MSGBUFPTR bufptr = new MSGBUF();

	return bufptr;
}

//!
//! \brief Release an SCP frame buffer back to the buffer pool
//!
//! \param[in] bufptr A pointer to the buffer
//!
//========================================================================
void CSirCon::BufFree(MSGBUFPTR bufptr)
{

	delete bufptr;
}

//!
//! \brief Queue a message frame for transmission to the radio. 
//!
//! Compose a message frame and queue it for transmission.
//!
//! \param[in] data A pointer to the message data.
//! \param[in] len The length of the message (bytes).
//!
//! \retval SCRESULT The result of the operation.
//!
//========================================================================
std::future<SCRESULT> CSirCon::Send (uint8_t* data, uint32_t len)
{

	try
	{
		MSGBUFPTR bufptr = ComposeFrame(data, len);
		std::lock_guard<std::mutex> lk(m_queue_lock);

		m_queue.push(bufptr);
		return bufptr->result.get_future();
	}
	catch (std::bad_alloc&)
	{
		std::promise<SCRESULT> p;

		LogWrite(LEVEL_CRITICAL, "Memory allocation failure!");
		p.set_value(SCR_NOMEMORY);
		return (p.get_future());
	}
}

//!
//! \brief Dispatch a message from the radio to the appropriate handler 
//! function
//!
//! \param[in] data A pointer to the message data
//! \param[in] len The length of the message (bytes)
//!
//========================================================================
void CSirCon::Dispatch (uint8_t* data, uint32_t len)
{

	switch (*data)
	{
		case MSG_SET_RESP:
			DispatchSetResponse(data, len);
		break;

		case MSG_GET_RESP:
			DispatchGetResponse(data, len);
		break;

		case MSG_ASYNC:
			DispatchAsync(data, len);
		break;

		default:
			LogWrite(LEVEL_DEBUG, "Dispatch: unknown cmd 0x%02X", *data);
		break;
	}
}

//!
//! \brief Dispatch an asynchronous notification message from the radio
//!
//! \param[in] data A pointer to the message data
//! \param[in] len The length of the message data (bytes)
//!
//! \note For info that we cache, the cache mutex is acquired and held
//! both while updating the cache itself and while notifying the 
//! application that the data has been updated
//!
//========================================================================
void CSirCon::DispatchAsync (uint8_t* data, uint32_t len)
{

    LogWrite(LEVEL_DEBUG, "ASYNC event %02x, len %u", data[1], len);

	switch (data[1])
	{
		case SCP_ASYNC_RESET:
		{
			SCEReset r;

			Notify(r);
		}
		break;

		case SCP_ASYNC_SONGINFO:
		{
			SCESongInfo s;

			s.channel = static_cast<SCP_CHANNEL_INDEX>(data[2]);
			s.deserialize(data + 3, len - 3);
			Notify(s);
		}
		break;

		case SCP_ASYNC_SONGID:
		{
			SCESongID s;

			s.deserialize(data + 3, len - 3);
			Notify(s);
		}
		break;

		case SCP_ASYNC_TIME:
		{
			SCETime t;

			LogWrite(LEVEL_DEBUG, "Time change event.");
			t.deserialize(data + 2, len - 2);
			Notify(t);
		}
		break;

		case SCP_ASYNC_STATUS:
			switch (data[2])
			{
				case ST_TUNE:
				{
					std::lock_guard<std::mutex> lk(m_cache_lock);
					SCEChannel c;

					c.deserialize(data + 4, len - 4);
					m_curr_channel = c.channel;
					Notify(c);
				}
				break;

				case ST_SIGNAL:
				{
					SCESignal s;

					s.deserialize(data + 3, len - 3);
					Notify(s);
				}
				break;

				case ST_ANTENNA:
				{
					SCEAntenna a;

					a.deserialize(data + 3, len - 3);
					Notify(a);
				}
				break;

				default:
					LogWrite(LEVEL_DEBUG, "Unknown 0x80 0x04 notification type:");
				break;
			}
		break;

		case SCP_ASYNC_SIGNAL:
		{
			SCERSSI r;

			r.deserialize(data + 2, len - 2);
			Notify(r);
		}
		break;

		default:
			LogWrite(LEVEL_DEBUG, "Unknown async notification type 0x%02x", data[1]);
		break;
	}
}

//!
//! \brief Dispatch the radio's response to an earlier GET request
//!
//! \param[in] data A pointer to the message data
//! \param[in] len The length of the message data (bytes)
//!
//! \note For info that we cache, the cache mutex is acquired and held
//! both while updating the cache itself and while notifying the 
//! application that the data has been updated
//!
//========================================================================
void CSirCon::DispatchGetResponse (uint8_t* data, uint32_t len)
{
	uint32_t result = (data[2] << 8) | data[3];

	LogWrite(LEVEL_DEBUG, "GET response %02x %04x, len %u",
		data[1], result, len);

	SCEGetResult res;
	res.result = result;
	Notify(res);

	if (result == 0u)
	{
		switch (data[1])
		{
			case SCP_GET_GAIN:
			{
				SCEGain g;

				g.deserialize(data + 4, len - 4);
				Notify(g);
			}
			break;

			case SCP_GET_MUTE:
			{
				SCEMute m;

				m.deserialize(data + 4, len - 4);
				Notify(m);
			}
			break;

			case SCP_GET_POWER:
			{
				SCEPower p;

				p.deserialize(data + 4, len - 4);
				Notify(p);
			}
			break;

			case SCP_GET_CHANNEL:
			{
				std::lock_guard<std::mutex> lk(m_cache_lock);
				SCEChannel c;

				c.deserialize(data + 4, len - 4);
				m_curr_channel = c.channel;
				Notify(c);
			}
			break;

			case SCP_GET_CHANNELINFO:
			{
				uint32_t offset = 4;
				SCEChannelInfo c;
				SCESongInfo s;

				// GRAB CHANNEL AND SONG INFO
				offset += c.deserialize(data + offset, len - offset);
				offset += s.deserialize(data + offset, len - offset);
				s.channel = c.channel;
				Notify(c);
				Notify(s);
			}
			break;

			case SCP_GET_SONGINFO:
			{
				SCESongInfo s;

				s.channel = data[4];
				s.deserialize(data + 5, len - 5);
				Notify(s);
			}
			break;

			case SCP_GET_CHANNEL_MAP:
			{
				if (len >= 32u)
				{
					SCEChannelMap m;

					m.deserialize(data + 4, len - 4);
					Notify(m);

					// CACHE A COPY FOR CHANNEL VALIDITY CHECKS
					m_cache_lock.lock();
					memcpy(m_channel_map, data + 4, SCP_CHANNEL_BITMAP_SIZE);
					m_cache_lock.unlock();
				}
			}
			break;

			case SCP_GET_SID:
			{
				SCESiriusID s;

				s.deserialize(data + 4, len - 4);
				Notify(s);
			}
			break;

			case SCP_GET_TZINFO:
			{
				SCETimeZoneInfo t;

				t.deserialize(data + 4, len - 4);
				Notify(t);
			}
			break;

			case SCP_GET_TIME:
			{
				SCETime t;

				t.deserialize(data + 4, len - 4);
				Notify(t);
			}
			break;

			case SCP_GET_STATUS:
			{
				SCEStatus s;

				s.deserialize(data + 4, len - 4);
				Notify(s);
			}
			break;

			case SCP_GET_RSSI:
			{
				SCERSSI r;

				r.deserialize(data + 4, len - 4);
				Notify(r);
			}
			break;

			default:
				LogWrite(LEVEL_DEBUG, "Unknown 0x60 message 0x%02X", data[1]);
			break;
		}
	}
}

//!
//! \brief Dispatch the radio's response to an earlier SET request
//!
//! \param[in] data A pointer to the message data
//! \param[in] len The length of the message data (bytes)
//!
//========================================================================
void CSirCon::DispatchSetResponse (uint8_t* data, uint32_t len)
{
	uint32_t result = (data[2] << 8) | data[3];

	LogWrite(LEVEL_DEBUG, "SET response %02x %04x, len %u", 
		data[1], result, len);

	SCESetResult res;
	res.result = result;
	Notify(res);

	if (result == 0u)
	{
		switch (data[1])
		{
			case SCP_SET_CHANNEL:
				if (len > 4u)
				{
					uint32_t offset = 4;
					SCEChannelInfo c;
					SCESongInfo s;

					// GRAB CHANNEL AND SONG INFO
					offset += c.deserialize(data + offset, len - offset);
					offset += s.deserialize(data + offset, len - offset);
					m_curr_channel =  s.channel = c.channel;
					Notify(c);
					Notify(s);
				}
			break;
		}
	}
}

//!
//! \brief Test whether a channel index represents a valid channel.
//!
//! This function looks up the given channel index in the Channel Map 
//! and returns true if the corresponding bit is set.
//!
//========================================================================
bool CSirCon::IsValidChannel (SCP_CHANNEL_INDEX channel)
{
	std::lock_guard<std::mutex> lk(m_cache_lock);

    if (channel < SCP_MAX_CHANNELS)
    {
		uint8_t idx = SCP_CHANNEL_BITMAP_SIZE - (channel / 8u) - 1u;
		uint8_t mask = 1u << (channel % 8u);

		if ((m_channel_map[idx] & mask) == mask)
		{
			return true;
		}
    }

    return false;
}

//========================================================================
std::future<SCRESULT> CSirCon::GetMute()
{
	uint8_t mute[2] = { MSG_GET, SCP_GET_MUTE };

	return Send(mute, sizeof(mute));
}

//========================================================================
std::future<SCRESULT> CSirCon::GetStatus(SCP_STATUS_TYPE status_type)
{
	uint8_t status[3] = { MSG_GET, SCP_GET_STATUS, 0x00 };

	status[2] = static_cast<uint8_t>(status_type);
	return Send(status, sizeof(status));
}

//========================================================================
std::future<SCRESULT> CSirCon::SetGain(int8_t db)
{
    uint8_t gain[3] = { MSG_SET, SCP_SET_GAIN, 0x00 };

    gain[2] = static_cast<uint8_t>(db);
    return Send(gain, sizeof(gain));
}

//========================================================================
std::future<SCRESULT> CSirCon::SetMute(bool on)
{
    uint8_t mute[3] = { MSG_SET, SCP_SET_MUTE, 0x00 };

    if (on)
    {
        mute[2] = 0x01;
    }
    return Send(mute, sizeof(mute));
}

//========================================================================
std::future<SCRESULT> CSirCon::SetPower(uint8_t mode)
{
    uint8_t power[3] = { MSG_SET, SCP_SET_POWER, 0x00 };

    power[2] = mode & 0x03;
    return Send(power, sizeof(power));
}

//========================================================================
std::future<SCRESULT> CSirCon::Reset()
{
    uint8_t pwr_reset[2] = { MSG_SET, SCP_SET_RESET };

    return Send(pwr_reset, sizeof(pwr_reset));
}

//========================================================================
std::future<SCRESULT> CSirCon::SetChannel(SCP_CHANNEL_INDEX channel)
{
    uint8_t set_channel[6] = { MSG_SET, SCP_SET_CHANNEL, 0x00, 0x00, 0x00, 0x0b };

    set_channel[2] = channel;
    return Send(set_channel, sizeof(set_channel));
}

//========================================================================
std::future<SCRESULT> CSirCon::SetTZ(short offset, bool dst)
{
    uint8_t settz[5] = { MSG_SET, SCP_SET_TZ_INFO, 0x00, 0x00, 0x00 };

    settz[2] = offset >> 8;
    settz[3] = offset & 0xff;

    if (dst)
    {
        settz[4] = 0x01;
    }

    return Send(settz, sizeof(settz));
}

//========================================================================
std::future<SCRESULT> CSirCon::EnableAsyncNotifications(uint8_t flags)
{
    uint8_t async[7] = { MSG_SET, SCP_SET_ASYNC, 0x00, 0x00, 0x00, 0x3f, 0x00 };

    async[5] = flags;
    return Send(async, sizeof(async));
}

//========================================================================
std::future<SCRESULT> CSirCon::GetChannelMap()
{
    uint8_t channel_map[3] = { MSG_GET, SCP_GET_CHANNEL_MAP, 0x00 };

    return Send(channel_map, sizeof(channel_map));
}

//========================================================================
std::future<SCRESULT> CSirCon::GetSID()
{
    uint8_t get_sid[2] = { MSG_GET, SCP_GET_SID };

    return Send(get_sid, sizeof(get_sid));
}

//========================================================================
std::future<SCRESULT> CSirCon::GetChannel()
{
	uint8_t gc[2] = { MSG_GET, SCP_GET_CHANNEL };

	return Send(gc, sizeof(gc));
}

//========================================================================
std::future<SCRESULT> CSirCon::GetChannelInfo(SCP_CHANNEL_INDEX channel)
{
    uint8_t gci[6] = { MSG_GET, SCP_GET_CHANNELINFO, 0x00, 0x00, 0x00, 0x00 };

    gci[2] = channel;
    return Send(gci, sizeof(gci));
}

//========================================================================
std::future<SCRESULT> CSirCon::GetRSSI()
{
    uint8_t rssi[2] = { MSG_GET, SCP_GET_RSSI };

    return Send(rssi, sizeof(rssi));
}

//========================================================================
std::future<SCRESULT> CSirCon::GetSongInfo(SCP_CHANNEL_INDEX channel)
{
    uint8_t songinfo[4] = { MSG_GET, SCP_GET_SONGINFO, 0x00, 0x00 };

    songinfo[2] = channel;
    return Send(songinfo, sizeof(songinfo));
}

//========================================================================
std::future<SCRESULT> CSirCon::GetTime()
{
    uint8_t gettime[2] = { MSG_GET, SCP_GET_TIME };

    return Send(gettime, sizeof(gettime));
}

//========================================================================
std::future<SCRESULT> CSirCon::GetGain()
{
	uint8_t gain[2] = { MSG_GET, SCP_GET_GAIN };

	return Send(gain, sizeof(gain));
}

//========================================================================
std::future<SCRESULT> CSirCon::GetTZ()
{
    uint8_t tz[2] = { MSG_GET, SCP_GET_TZINFO };

    return Send(tz, sizeof(tz));
}

//========================================================================
std::future<SCRESULT> CSirCon::GetPower()
{
	uint8_t power[2] = { MSG_GET, SCP_GET_POWER };

	return Send(power, sizeof(power));
}


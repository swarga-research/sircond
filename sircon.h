/**
 * $DateTime: 2015/08/23 13:00:12 $
 * $Id: //depot/sircond/sircon.h#9 $
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

#ifndef _SIRCON_H_
#define _SIRCON_H_

//!
//! \file sircon.h
//! \brief Declarations for the SiriusConnect interface class.
//!

#include <future>
#include <queue>
using std::queue;

#include <string>
using std::string;

#include "observer.h"
#include "scp.h"
#include "scevents.h"
#include "serial.h"
#include "ctask.h"
#include "ctimer.h"
#include "sobuf.h"


//! Maximum number of times to retransmit a packet
const uint32_t SIRCON_MAX_RETRIES = 3u;

//! Number of link failures before bailing out
const uint32_t SIRCON_MAX_LINK_FAILURES = 10u;

//! Size of the input buffer (bytes)
const uint32_t SCP_STAGEBUFSIZE = 2u * SCP_MAX_PKT;

//! RESULT CODES
enum SCRESULT
{
	SCR_SUCCESS = 0,
	SCR_TIMEOUT,
	SCR_NOMEMORY
};

//! A container for queued messages 
typedef struct MSGBUF
{
	uint32_t timer;
	uint32_t retries;
	uint32_t len;
	uint32_t seq;
	std::promise<SCRESULT> result;
	uint8_t data[SCP_STAGEBUFSIZE];

	MSGBUF() : timer(0u), retries(0u), len(0u), seq(0u) {}
} *MSGBUFPTR;

//!
//! \brief The Sirius Connect radio interface object
//!
//! This object encapsulates all radio functionality.
//!
class CSirCon : public sr::CTask, public Subject<SCEvent>
{
public:
    CSirCon (const string& device);
    virtual ~CSirCon ();

	std::future<SCRESULT> Reset();
	std::future<SCRESULT> SetPower(uint8_t mode);
	std::future<SCRESULT> SetMute(bool on);
	std::future<SCRESULT> EnableAsyncNotifications(uint8_t flags);
	std::future<SCRESULT> SetGain(int8_t db);
	std::future<SCRESULT> SetTZ(short offset, bool dst);
	std::future<SCRESULT> SetChannel(SCP_CHANNEL_INDEX channel);
	std::future<SCRESULT> GetGain();
	std::future<SCRESULT> GetPower();
	std::future<SCRESULT> GetMute();
	std::future<SCRESULT> GetStatus(SCP_STATUS_TYPE status_type);
	std::future<SCRESULT> GetChannelMap();
	std::future<SCRESULT> GetSID();
	std::future<SCRESULT> GetChannel();
	std::future<SCRESULT> GetChannelInfo(SCP_CHANNEL_INDEX channel);
	std::future<SCRESULT> GetRSSI();
	std::future<SCRESULT> GetSongInfo(SCP_CHANNEL_INDEX channel);
	std::future<SCRESULT> GetTime();
	std::future<SCRESULT> GetTZ();

	bool IsLinkAlive() { return m_link_alive; };
    bool IsValidChannel (SCP_CHANNEL_INDEX channel);
	SCP_CHANNEL_INDEX GetCurrentChannel() { return m_curr_channel; }

    bool OnStart ();
    void OnRun ();
    void OnExit ();

protected:
    CSirCon ();
    bool Open (const char* device);
	bool SetDataRate (uint32_t baud) { return (m_port->SetDataRate(baud) == 0); }
    void Close ();
    bool Read (uint8_t* buf, uint32_t maxlen, uint32_t* uint8_tsread);
    bool Write (uint8_t* buf, uint32_t len);
    uint32_t GetTimeSinceLastRx ();
	void OnACK(MSGBUFPTR bufptr);
	void OnTimeout (MSGBUFPTR bufptr);

	sr::CSerialPort* m_port;		//!< The serial port object

private:
	sr::SOBuffer m_stagebuf;		//!< Staging buffer for incoming SCP frames
    time_t m_last_rx;				//!< Timestamp of last received frame
    uint32_t m_busy_timer;          //!< Count of timer ticks to wait while radio is busy
    uint8_t m_seq;                  //!< Next outgoing frame sequence number
    int32_t m_last_seq;				//!< Last frame sequence number seen
	uint8_t m_seq_expected;			//!< Expected next incoming frame sequence number
	sr::CTimer m_timer;             //!< Timer manager object
	sr::HTIMER m_htimer;            //!< Handle to the (re)transmit timer instance

	// QUEUE OF FRAMES WAITING TO BE TRANSMITTED
	std::mutex m_queue_lock;        //!< Queue mutex
    queue<MSGBUFPTR> m_queue;       //!< Queue of frames from the application

	// CACHED RADIO STATE INFORMATION
	std::mutex m_cache_lock;		//!< Serializes access to cached info
    uint8_t m_channel_map[SCP_CHANNEL_BITMAP_SIZE];	//!< Bitmap of valid channels
    SCP_CHANNEL_INDEX m_curr_channel;				//!< Channel to which the receiver is currently tuned

	bool m_link_alive;				//!< True if the SCP link to the radio is functional
	uint32_t m_link_fail_cnt;		//!< Count of link failures

    std::future<SCRESULT> Send (uint8_t* data, uint32_t len);
    uint8_t ComputeChecksum (const uint8_t* data, uint32_t len);
    bool ValidateChecksum (const uint8_t* data, uint32_t len);
	MSGBUFPTR BufAlloc();
	void BufFree(MSGBUFPTR bufptr);
	MSGBUFPTR ComposeFrame (const uint8_t* databuf, uint32_t datalen);
	bool TransmitFrame (MSGBUFPTR bufptr);
    static void TimerProcWrapper (void* param);
    void TimerProc ();
    bool SendACK (uint8_t ack, uint8_t flags);

    void Dispatch (uint8_t* data, uint32_t len);
    void DispatchAsync (uint8_t* data, uint32_t len);
    void DispatchGetResponse (uint8_t* data, uint32_t len);
    void DispatchSetResponse (uint8_t* data, uint32_t len);
};

#endif

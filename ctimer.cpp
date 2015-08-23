/*
 * $DateTime: 2015/08/23 13:00:12 $                                                                
 * $Id: //depot/sircond/ctimer.cpp#6 $
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
#include <time.h>
#include "ctimer.h"

namespace sr
{

// MINIMUM SLEEP TIME (MILLISECONDS)
const uint32_t MIN_SLEEPTIME = 100;


//========================================================================
TIMERREC::TIMERREC () :
  handle(0),
  count(0),
  interval(MIN_SLEEPTIME),
  instance(0),
  callback(NULL)
{

}

//========================================================================
TIMERREC::TIMERREC (const TIMERREC& t)
{

  handle = t.handle;
  count = t.count;
  interval = t.interval;
  instance = t.instance;
  callback = t.callback;
}

//========================================================================
CTimer::CTimer () :
  m_sleep_time(MIN_SLEEPTIME),
  m_sequence(0)
{

}

//========================================================================
CTimer::~CTimer ()
{

	Stop();

	// DESTROY ALL THE TIMERS
	m_timers.clear();
}

//========================================================================
//
//
//
//========================================================================
void CTimer::OnRun ()
{

    while (!IsShutdown())
    {
        list<TIMERREC>::iterator i;

        // WALK THE LIST OF TIMERS - UPDATE THEIR COUNTS, AND CALL THE TIMER
        // HANDLER IF THE TIMER EXPIRES
        m_critsect.lock();
        i = m_timers.begin();
        while (i != m_timers.end())
        {
            if (--((*i).count) == 0)
            {
                (*i).callback((*i).instance);
                (*i).count = ((*i).interval / m_sleep_time);
            }
            i++;
        }
        m_critsect.unlock();

		std::this_thread::sleep_for(std::chrono::milliseconds(m_sleep_time));
    }
}

//========================================================================
HTIMER CTimer::Create (uint32_t interval, void* instance, TIMERCALLBACK callback)
{
	TIMERREC r;

	r.handle = static_cast<HTIMER>(m_sequence++);
	r.interval = interval;
	r.instance = instance;
	r.callback = callback;

	r.count = interval/m_sleep_time;

	m_critsect.lock();
	m_timers.push_back(r);
	m_critsect.unlock();

	return (r.handle);
}

//========================================================================
void CTimer::Destroy (HTIMER h)
{
	std::lock_guard<std::mutex> lk(m_critsect);
	list<TIMERREC>::iterator i;

	i = m_timers.begin();
	while (i != m_timers.end())
	{
		if (h == (*i).handle)
		{
			m_timers.erase(i);
			break;
		}
		i++;
	}
}

//========================================================================
void CTimer::Restart (HTIMER h)
{
	std::lock_guard<std::mutex> lk(m_critsect);
	list<TIMERREC>::iterator i;

	i = m_timers.begin();
	while (i != m_timers.end())
	{
		if (h == (*i).handle)
		{
			(*i).count = (*i).interval / m_sleep_time;
			break;
		}
		i++;
	}
}

}

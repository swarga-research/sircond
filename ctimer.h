/**
 * $DateTime: 2015/08/23 13:00:12 $                                                                
 * $Id: //depot/sircond/ctimer.h#6 $
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

#ifndef _CTIMER_H_
#define _CTIMER_H_

#include <list>
using std::list;

#include "ctask.h"

namespace sr
{

typedef uint32_t HTIMER;								//!< HANDLE TO A TIMER OBJECT
const HTIMER INVALID_TIMER_HANDLE_VALUE = 0xffffffffu;	//!< SENTINEL VALUE FOR AN INVALID TIMER HANDLE
typedef void (*TIMERCALLBACK)(void*);					//!< SIGNATURE OF A TIMER CALLBACK FUNCTION

//!
//! \internal
//! \brief A periodic timer object. 
//!
//! This timer object calls the specified function periodically
//! at the specified interval. It will continue to make these
//! calls until the timer object is destroyed.
//!
class TIMERREC
{
public:
  HTIMER handle;              //!< THE HANDLE (UNIQUE ID) OF THIS TIMER
  uint32_t count;             //!< COUNT OF CLOCK TICKS SINCE TIMER LAST FIRED
  uint32_t interval;          //!< CALL INTERVAL (MS)
  void* instance;             //!< APPLICATION-SUPPLIED INSTANCE DATA
  TIMERCALLBACK callback;     //!< POINTER TO A FUNCTION THAT WILL BE CALLED WHEN A TIMER EXPIRES 

  TIMERREC ();                   
  TIMERREC (const TIMERREC& t);  
};

//!
//! \brief A timer manager class.
//!
//! Manages a set of periodic timer objects.
//!
class CTimer : public sr::CTask
{
public:
  CTimer ();
  ~CTimer ();

  HTIMER Create (uint32_t interval, void* instance, TIMERCALLBACK callback);
  void Restart (HTIMER h);
  void Destroy (HTIMER h);
  void OnRun ();

private:
  std::mutex m_critsect;        // SERIALIZES ACCESS TO THE TIMER LIST
  list<TIMERREC> m_timers;      // THE LIST OF ACTIVE TIMERS
  uint32_t m_sleep_time;        // TIME BETWEEN THREAD WAKEUPS
  uint32_t m_sequence;          // USED TO GENERATE TEMPORALLY UNIQUE TIMER HANDLE VALUES

  void UpdateSleepTime (uint32_t new_interval);  
};

}

#endif

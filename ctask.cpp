/*
 * $DateTime: 2015/08/23 13:00:12 $                                                               
 * $Id: //depot/sircond/ctask.cpp#9 $
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
//! \file ctask.cpp
//!
//! \brief Declarations for task abstraction class.
//!

#include <memory>
#include <assert.h>
#include <signal.h>
#include "ctask.h"

namespace sr
{

//!
//! \brief Destructor.
//!
//! \note Ideally the application should ensure that the thread is not 
//! joinable (i.e. by calling Stop()) prior to destroying the CTask
//! object and its underlying std::thread. If for any reason that didn't 
//! happen we do a "gratuitous" join here.
//!
//========================================================================
CTask::~CTask() 
{ 

	if (m_thread.joinable())
	{
		m_thread.join();
	}
}

//!
//! \brief Begin execution of a task.
//!
//! Called by the application to begin execution of a task.
//!
//! \retval bool Returns true if the task was successfully launched.
//!
//========================================================================
bool CTask::Start ()
{

	// THREAD ALREADY STARTED?
	assert(!m_thread.joinable());
	assert(m_state == STOPPED);

	m_state = STARTING;

	try
	{
		m_thread = std::thread(&CTask::Run, this);
	}
	catch (...)
	{
		return false;
	}

    return true;
}

//!
//! \brief Signal a task to terminate.
//!
//! Called by the application to ask a task to terminate. After 
//! requesting the shutdown, wait until the thread terminates before
//! returning to the caller.
//!
//! \retval bool Returns true if the task was successfully terminated.
//!
//========================================================================
bool CTask::Stop ()
{

    if (OnStop())
    {
		Shutdown();
		if (m_thread.joinable())
		{
			m_thread.join();
		}
    }
	else
	{
		return false;
	}

    return true;
}

//!
//! \brief Task main loop.
//!
//! The entry point of the thread that executes the task. Runs through the
//! full life cycle of the task before returning. A pointer to this 
//! function is passed to the thread object when the task is started.
//!
//========================================================================
void CTask::Run ()
{
#ifndef WIN32
	sigset_t mask;

	// BLOCK ALL SIGNALS BY DEFAULT
	sigfillset(&mask);
	int err = pthread_sigmask(SIG_BLOCK, &mask, 0);
	if (err == 0)
#endif
	{
		if (OnStart())
		{
			m_state = RUNNING;
			OnRun();
		}
	}

	OnExit();
	m_state = STOPPED;
}

}



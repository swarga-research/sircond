/**
 * $DateTime: 2015/08/23 13:00:12 $                                                               
 * $Id: //depot/sircond/ctask.h#7 $
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
//! \file ctask.h
//! \brief Declarations for task abstraction class.
//!

#ifndef _CTASK_H_
#define _CTASK_H_

#include <memory>
#include <string>
#include <thread>
#include <atomic>

namespace sr
{

//!
//! \brief A simple task abstraction.
//!
//! In the context of this framework, a task is loosely defined as work perfomred
//! by a thread. The code that performs this work is encapsulated by the CTask-based 
//! class. 
//!
//! A task's life cycle consists of up to 3 phases: an "initialization" phase where 
//! one-time setup is performed, a "run" phase where the bulk of the work is performed, 
//! and a "cleanup" phase where resources are released prior to the termination of the 
//! thread. Note that the run phase may be skipped if an error occurs during the
//! initialization phase. Each of these phases is represented by a separate member 
//! function in the CTask class. Derived classes override these functions in order to 
//! specify the work performed by the task.
//!
//! CTask also provides methods to start a task running and to request task 
//! termination. The task has the option to reject the termination request, if
//! necessary.
//!
class CTask
{
public:
	enum STATE
	{
		STARTING,	//!< Task is in the initialization phase
		RUNNING,	//!< Task is in rhe run phase
		STOPPED		//!< Task has terminated
	};

	CTask() : m_state(STOPPED), m_shutdown(false) {}
	CTask(CTask const&) = delete;
	CTask& operator=(CTask const&) = delete;
	virtual ~CTask();

	//!
	//! \brief Returns the current execution phase.
	//!
	//! \retval STATE The current execution phase.
	//!
	STATE GetState() { return m_state; }

	//!
	//! \brief Indicates that task shutdown has begun.
	//!
	//! \retval bool Returns true if a shutdown has been initiated.
	//!
	bool IsShutdown() { return m_shutdown; }

	bool Start ();
	bool Stop ();
			
protected:
	//!
	//! \brief Perform one-time initialization for the task.
	//!
	//! This function is called from the context of the task's 
	//! thread immediately after the thread is started. This 
	//! allows any required one-time initialization to occur.
	//!
	//! \retval bool Returns true to indicate the initialization was
	//! successful. If this function returns false, the run phase 
	//! will be skipped and the task will exit.
	//!
	virtual bool OnStart() { return true; }

	//!
	//! \brief The main loop of the task.
	//!
	//! If initialization is successful, this function is called from 
	//! the context of the task's thread. This function should not return
	//! until either the task has been completed or another thread
	//! requests a shutdown.
	//!
	virtual void OnRun() = 0;

	//!
	//! \brief Respond to a shutdown request.
	//!
	//! This function is called from another thread to request a shutdown
	//! of the task. The return value from this function determines whether
	//! the shutdown process will continue.
	//!
	//! \retval bool Returns true to indicate that it is now safe 
	//! to terminate the task. If this function returns false, the 
	//! shutdown process will be aborted.
	//!
	virtual bool OnStop() { return true; }

	//!
	//! \brief Perform final cleanup prior to termination.
	//!
	//! This function is called from the context of the task's thread to 
	//! perform any necessary cleanup (e.g. to release any resources
	//! allocated by the task) prior to the termination of the
	//! task's thread.
	//!
	virtual void OnExit() {};

	//!
	//! \internal
	//!
	//! \brief Called from elsewhere in the task to begin the shutdown process.
	//!
	void Shutdown() { m_shutdown = true; }
	
private:	
	STATE m_state;		//!< Current state of the task
	std::atomic<bool> m_shutdown;	//!< Flag to request task termination
	std::thread m_thread;	//!< The thread which executes this task
	
	void Run ();
};

}

#endif


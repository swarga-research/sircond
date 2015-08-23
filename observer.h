/**
* $DateTime: 2015/08/23 13:00:12 $
* $Id: //depot/sircond/observer.h#1 $
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

#ifndef _OBSERVER_H_
#define _OBSERVER_H_

//!
//! \file OBSERVER.H
//!
//! \brief A minimalist implementation of the Observer design pattern.
//!
//! The Observer design pattern is a publisher/subscriber model wherein
//! "observers" register for event notifications from a "subject" 
//!

#include <vector>
#include <algorithm>

//!
//! \brief The Observer class
//!
template <typename T>
class IObserver
{
public:
	virtual ~IObserver () { }
	//! \brief Called by the Subject when an event occurs
	virtual void Update (T& t) = 0;
};

//!
//! \brief The Subject class
//!
//! The Subject is the source of the event notifications being
//! monitored by the Observer. When an event occurs, the Update
//! method of each Observer is invoked in turn.
//!
template <typename T>
class Subject
{
public:
	//! \brief Add an Observer to the notification list
	void Attach (IObserver<T>* o) { m_list.push_back(o); }
	//! \brief Remove an Observer from the notification list
	void Detach (IObserver<T>* o) { m_list.remove(o); }
	//! \brief Send an update to all attached Observers
	void Notify(T& t) 
	{ 
		std::for_each(m_list.begin(), m_list.end(), [&t](IObserver<T>* o) { o->Update(t); });
	}

private:
    std::vector<IObserver<T>*> m_list;		//!< The list of registered Observers
};

#endif

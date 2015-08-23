/*
 * $Id: //depot/sircond/sircond.cpp#30 $
 * $Change: 2154 $
 * $DateTime: 2015/08/23 13:00:12 $
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
//! \file sircond.cpp
//! \brief Main Module.
//!

#ifdef WIN32
#pragma warning(disable:4996)
#endif

#include "pch.h"
#include <iostream>
#include "sirserver.h"

//!
//! \brief A simple application wrapper.
//!
class CDaemon
{
public:
	CDaemon ();
	int Launch (int argc, char* argv[]);

protected:
	bool Init (int argc, char* argv[]);
	void Run ();
	void Shutdown ();

private:
	string m_pidfile;
	string m_logroot;
	string m_logfile;
	bool m_shutdown;	
	CSirServer* m_server;
};

//========================================================================
CDaemon::CDaemon () : m_shutdown(false), m_server(0)
{

#ifndef WIN32
	m_pidfile = "/var/run/sircond.pid";
	m_logroot = "/var/log/";
#endif
	m_logfile = m_logroot + "sircond.log";
}

//========================================================================
bool CDaemon::Init (int argc, char* argv[])
{
#ifdef WIN32
	WSADATA wsadata;

	if (WSAStartup(MAKEWORD(1,1), &wsadata) != 0)
	{
		printf("Network initialization failed.\n");
		return 1;
	}
#endif

	// PROCESS COMMAND LINE ARGS
	if (argc < 2)
	{
		return false;
	}

	// OPEN A LOG FILE
	LogSetLevel(EL_DEBUG);
	if (!LogOpen(m_logfile))
	{
		std::cout << "Error opening log!" << std::endl;
		return false;
	}

	LogWrite(LEVEL_INFO, "Startup.");

#ifndef _WIN32
	// CREATE A PID FILE
	CreatePidFile(m_pidfile);
#endif

	// INSTANTIATE THE SERVER OBJECT
	m_server = new CSirServer(argv[1]);
	if (m_server == 0)
	{
		LogWrite(LEVEL_CRITICAL, "Failed to instantiate server object.");
		return false;
	}

	// LAUNCH THE SERVER
	if (!m_server->Start())
	{
		return false;
	}
	while (m_server->GetState() == CSirServer::STARTING)
	{
		sleep(1);
	}

	// RETURN LAUNCH STATUS
	return (m_server->GetState() == CSirServer::RUNNING);
}

//========================================================================
void CDaemon::Run ()
{
#ifndef WIN32
	sigset_t mask;

	// RUN UNTIL SIGNALLED
	sigfillset(&mask);
	if (pthread_sigmask(SIG_BLOCK, &mask, 0) == 0)
	{
		while (!m_shutdown)
		{
			siginfo_t info;
			if (sigwaitinfo(&mask, &info) > 0) 
			{
				switch (info.si_signo)
				{
					case SIGHUP:
						LogReopen(m_logfile);
						LogWrite(LEVEL_INFO, "Log pinched.");
					break;

					case SIGINT:
					case SIGTERM:
					default:
						LogWrite(LEVEL_DEBUG, 
							"Got signal %d from process %d - exiting.", 
							info.si_signo, 
							info.si_pid);
						m_shutdown = true;
					break;
				}
			}
			else
			{
				LogWrite(LEVEL_ERROR, "sigwait fail");
			}
		}
	}
	else
	{
		LogWrite(LEVEL_ERROR, "pthread_sigmask fail.");
	}
#else
	// UNDER WINDOWS JUST RUN UNTIL THE USER CLOSES THE APP
	while (!m_shutdown && (m_server->GetState() == CSirServer::RUNNING))
	{
		sleep(1);
	}
#endif
}

//========================================================================	
void CDaemon::Shutdown ()
{

	if (m_server != 0)
	{
		m_server->Stop();
		delete m_server;
		m_server = 0;
	}

	LogClose();

#ifndef WIN32
	unlink(m_pidfile.c_str());
#else
	WSACleanup();
#endif
}

//========================================================================
int CDaemon::Launch (int argc, char* argv[])
{

	if (Init(argc, argv))
	{
		Run();
	}
	Shutdown();

	return 0;
}

//========================================================================
int main (int argc, char* argv[])
{
	CDaemon d;

	return (d.Launch(argc, argv));
}





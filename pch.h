/**
 * $DateTime: 2015/08/23 13:22:19 $                                                                
 * $Id: //depot/sircond/pch.h#8 $
 * $Change: 2158 $
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

#ifndef _PCH_H_
#define _PCH_H_

//!
//! \file pch.h
//!
//! \brief Common header includes and Doxygen mainpage
//!

#ifdef WIN32

#define  STRICT
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdint.h>
#include <limits.h>
#include <process.h>
#include <stddef.h>
#include <tchar.h>
#include <stdint.h>

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif

#else

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <pthread.h>
#include <syslog.h>

typedef int SOCKET;
#define closesocket close
#define __cdecl	
#define INVALID_SOCKET	-1
#define CALLBACK
typedef unsigned char INT8U;
typedef unsigned short INT16U;
typedef unsigned long INT32U;

#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>

using std::ifstream;
using std::string;

#include "util.h"
#include "log.h"

//!
//! \mainpage SirCon Daemon
//!
//! Sircond is a management daemon that interfaces with a SiriusConnect
//! satellite radio receiver. SiriusConnect is a standard interface protocol 
//! "spoken" by Sirius satellite radio receivers which are capable of external
//! control (including "Sirius-Ready" car stereo units). Some SiriusConnect
//! receivers, such as the SCH2P, have an RS-232 serial port. Others, such as
//! the SC-H1, provide an 8-pin mini-DIN connector and require additional interface
//! circuitry to interface with a standard RS-232 or USB port. A third type of 
//! interface, the TTS-100 from TimeTrax, interfaces a SiriusConnect car stereo 
//! unit to a standard USB port. Sircond supports all of these variants.
//!
//! In addition to allowing user control of a SiriusConnect radio, Sircond also 
//! acts as an arbiter of that control, allowing simultaneous "read-only" access by 
//! multiple network clients while granting "read/write" access to only one client
//! at a time. This allows multiple clients to receive metadata updates and status
//! information while preventing conflicting actions (e.g. one user changing channels 
//! while another user is recording a program). 
//!
//! While intended for use on Linux systems, Sircond can also be compiled and run 
//! on Win32 machines.
//!

#endif




/**
 * $DateTime: 2015/08/23 13:00:12 $                                                                
 * $Id: //depot/sircond/log.h#7 $
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

#ifndef _LOG_H_
#define _LOG_H_

//
// \file log.h
// \brief Logging facility.
//

#include "pch.h"
#include <stdint.h>
#include <string>

using std::string;

// THE AVAILABLE ERROR LEVELS
enum
{
  EL_EMERGENCY = 0,
  EL_ALERT,
  EL_CRITICAL,
  EL_ERROR,
  EL_WARNING,
  EL_NOTICE,
  EL_INFO,
  EL_DEBUG,
  NUM_ERRORLEVELS
};

// CONVENIENCE MACROS
#define LEVEL_EMERGENCY   __FILE__,__LINE__,EL_EMERGENCY
#define LEVEL_ALERT       __FILE__,__LINE__,EL_ALERT
#define LEVEL_CRITICAL    __FILE__,__LINE__,EL_CRITICAL
#define LEVEL_ERROR       __FILE__,__LINE__,EL_ERROR
#define LEVEL_WARNING     __FILE__,__LINE__,EL_WARNING
#define LEVEL_NOTICE      __FILE__,__LINE__,EL_NOTICE
#define LEVEL_INFO        __FILE__,__LINE__,EL_INFO
#define LEVEL_DEBUG       __FILE__,__LINE__,EL_DEBUG

// API PROTOTYPES
void LogGenerateFilename (string prefix, string& filename);
bool LogOpen (string filename);
bool LogOpenSyslog (const char* ident);
void LogClose ();
bool LogReopen (string filename);
void LogSetLevel (int errorlevel);
bool LogWrite (const char* file, int line, int level, const char* fmt, ...);
void LogWriteHex (const char* file, int line, int level, const char* str, const uint8_t* data, uint32_t length);

#endif

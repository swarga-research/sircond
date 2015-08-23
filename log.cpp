/*
 * $DateTime: 2015/08/23 13:00:12 $                                                               
 * $Id: //depot/sircond/log.cpp#11 $
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

//
// \file log.cpp
// \brief Logging facility.
//

#include "pch.h"
#include "log.h"
#ifndef WIN32
#include <syslog.h>
#endif

static const uint32_t MAXLINE = 512;
static const uint32_t HEX_DUMP_LINE_LEN = 16;

// LOOKUP TABLE TO CONVERT ERROR LEVELS TO THEIR ENGLISH EQUIVALENTS
static const char* s_level_lut[NUM_ERRORLEVELS] = 
{
  "EMERGENCY",
  "ALERT",
  "CRITICAL",
  "ERROR",
  "WARNING",
  "NOTICE",
  "INFO",
  "DEBUG"
};

#ifndef WIN32
// CONVERSION TABLE TO SYSLOG LOG LEVELS
static const int s_syslog_level_lut[NUM_ERRORLEVELS] = 
{
	LOG_EMERG,
	LOG_ALERT,
	LOG_CRIT,
	LOG_ERR,
	LOG_WARNING,
	LOG_NOTICE,
	LOG_INFO,
	LOG_DEBUG
};
#endif

static FILE* s_file = NULL;         // A HANDLE TO THE LOG FILE ITSELF
static int s_errorlevel = EL_ERROR; // DEFAULT ERROR LEVEL IS ERROR
static std::mutex s_mutex;			// MUTEX TO CONTROL ACCESS TO THE OUTPUT BUFFER
static bool s_syslog = false;		// LOG TO SYSLOG VS. LOCAL FILE


//========================================================================
//
// OPEN A LOG FILE. RETURNS true IF SUCCESSFUL, OR false IF THE LOG FILE 
// COULD NOT BE OPENED.
//
//========================================================================
bool LogOpen (string filename)
{

	if (s_file != NULL)
	{
		return false;
	}

	s_file = fopen(filename.c_str(), "at");

	return (s_file != NULL);
}

//========================================================================
bool LogOpenSyslog (const char* ident)
{
#ifdef WIN32
	string filename(ident);
	filename += string(".log");
	LogOpen(filename.c_str());
#else
	setlogmask(LOG_UPTO(LOG_DEBUG));
	openlog(ident, LOG_PID | LOG_NDELAY | LOG_CONS, LOG_LOCAL0);
	s_syslog = true;
#endif
	return true;
}

//========================================================================
//
// CLOSES THE LOG FILE PREVIOUSLY OPENED WITH LogOpen().
//
//========================================================================
void LogClose ()
{

	if (s_syslog)
	{
#ifndef WIN32
		closelog();
#endif
		s_syslog = false;
	}
	else if (s_file != NULL)
	{
		fclose(s_file);
		s_file = NULL;
	}
}

//========================================================================
//
// SEAMLESSLY CLOSE THE CURRENT LOG FILE AND OPEN A NEW ONE WITH THE GIVEN
// FILENAME. RETURNS TRUE IF SUCCESSFUL, OR FALSE IF THE LOG FILE 
// COULD NOT BE REOPENED.
//
//========================================================================
bool LogReopen (string filename)
{
	std::lock_guard<std::mutex> lk(s_mutex);

	if (s_syslog)
	{
		// POINTLESS WHEN USING SYSLOG
		return true;
	}

	if (s_file != NULL)
	{
		LogClose();
	}
	s_file = fopen(filename.c_str(), "wt");

	return (s_file != NULL);
}

//========================================================================
//
// WRITE A LINE OF OUTPUT TO THE LOG FILE IF THE GIVEN ERROR LEVEL IS 
// LESS THAN OR EQUAL TO THE CURRENT ERROR LEVEL SETTING. THE OUTPUT LINE
// IS PREPENDED WITH THE CURRENT DATE AND TIME, FILENAME, LINE NUMBER, AND 
// ERROR LEVEL INFORMATION.
//
//========================================================================
bool LogWrite (const char* file, int line, int level, const char* fmt, ...)
{
	char linebuf[MAXLINE];         // OUTPUT BUFFER
	va_list args;

	if ((level > s_errorlevel) || ((s_file == 0) && !s_syslog))
	{
	    return false;
	}

	va_start(args, fmt);
	vsnprintf(linebuf, MAXLINE, fmt, args);
    va_end(args);

#ifndef WIN32
	if (s_syslog)
	{
		syslog(s_syslog_level_lut[level], linebuf);
	}
	else
#endif
	{
		time_t t = time(0);
		tm* tmptr = localtime(&t);

		s_mutex.lock();

		// WRITE THE RESULTS TO THE LOG FILE
		fprintf(s_file, 
			  "%04u/%02u/%02u %02u:%02u:%02u %s: %s\n", 
			  tmptr->tm_year + 1900, 
			  tmptr->tm_mon + 1,
			  tmptr->tm_mday,
			  tmptr->tm_hour,
			  tmptr->tm_min,
			  tmptr->tm_sec,
			  s_level_lut[level],
			  linebuf);
		fflush(s_file);

#ifdef _DEBUG
		// ECHO A COPY TO THE SCREEN
		printf("%s\n", linebuf);
#endif

		s_mutex.unlock();
	}

    return true;
}

//========================================================================
//
// SET THE ERROR LEVEL FOR LOGGING. LOG MESSAGES WILL ONLY APPEAR IN THE
// LOG IF THEIR ERROR LEVEL IS LESS THAN OR EQUAL TO THE ERROR LEVEL SET
// BY THIS FUNCTION.
//
//========================================================================
void LogSetLevel (int errorlevel)
{

	if ((errorlevel >= EL_EMERGENCY) && (errorlevel < NUM_ERRORLEVELS))
	{
		s_errorlevel = errorlevel;
	}
}

//========================================================================
void LogGenerateFilename (string prefix, string& filename)
{
	time_t t = time(0);
	tm* tmptr = localtime(&t);
	char tmp[128];

	// GENERATE A UNIQUE FILENAME BASED ON THE CURRENT TIME
	sprintf(tmp,
			"%s-%04u-%02u-%02u-%02u-%02u.log", 
			prefix.c_str(),
			tmptr->tm_year + 1900,
			tmptr->tm_mon + 1, 
			tmptr->tm_mday,  
			tmptr->tm_hour, 
			tmptr->tm_min);
	filename = tmp;
}





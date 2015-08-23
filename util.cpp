/*
 * $DateTime: 2015/08/23 13:00:12 $                                                                
 * $Id: //depot/sircond/util.cpp#3 $
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
#include <algorithm>
#include "util.h"


#ifdef WIN32
#pragma warning(disable:4996)

//!
//! \brief Delay the calling thread.
//! 
//! \param[in] seconds The number of seconds to delay the calling thread.
//!
//! \note Needed only on WIN32 systems, which do not natively provide a 
//! standard sleep() function.
//!
//========================================================================
void sleep (uint32_t seconds)
{

	Sleep(seconds * 1000UL);
}
#endif

//!
//! \brief Split the input string into tokens based on the provided 
//! separator character.
//!
//! \param[in] in_str The string to be tokenized.
//! \param[in] in_split Contains one or more separator characters used to 
//! split the input string into tokens.
//! \param[in] return_partial If true, partial tokens will be returned.
//! \retval vector<string> A vector of tokens extracted from the string.
//!
//! \note Shamelessly lifted from Kismet.
//!
//========================================================================
vector<string> StrTokenize (string in_str, 
							string in_split, 
							bool return_partial) 
{
    size_t begin = 0;
    size_t end = in_str.find(in_split);
    vector<string> ret;

    if (in_str.length() == 0)
	{
        return ret;
	}
    
    while (end != string::npos) 
	{
        string sub = in_str.substr(begin, end-begin);
        begin = end+1;
        end = in_str.find(in_split, begin);
        ret.push_back(sub);
    }

    if (return_partial && begin != in_str.size())
	{
        ret.push_back(in_str.substr(begin, in_str.size() - begin));
	}

    return ret;
}

//!
//! \brief A complex string tokenizer.
//! A complex string tokenizer which understands nested delimiters, such as 
//! "foo","bar","baz,foo",something and network protocols like
//! foo bar \001baz foo\001
//!
//! \param[in] in_str The string to be tokenized.
//! \param[in] in_split Contains one or more separator characters used to 
//! split the input string into tokens.
//! \param[in] in_quote Quotation characters used to delimit tokens which 
//! may contain embedded separation characters.
//!
//! \note Shamelessly lifted from Kismet.
//!
//========================================================================
vector<smart_word_token> BaseStrTokenize (string in_str, 
										  string in_split, 
										  string in_quote) 
{
	size_t begin = 0;
	size_t end = 0;
	vector<smart_word_token> ret;
    smart_word_token stok;
	int special = 0;
	string val;
	
	if (in_str.length() == 0)
		return ret;

	for (unsigned int x = 0; x < in_str.length(); x++) 
	{
		if (in_str.find(in_quote, x) == x) 
		{
			if (special == 0) 
			{
				// reset beginning on string if we're in a special block
				begin = x;
				special = 1;
			} 
			else 
			{
				special = 0;
			}

			continue;
		}

		if (special == 0 && in_str.find(in_split, x) == x) 
		{
			stok.begin = begin;
			stok.end = end;
			stok.word = val;

			ret.push_back(stok);

			val = "";
			x += in_split.length() - 1;

			begin = x;

			continue;
		}

		val += in_str[x];
		end = x;
	}

	stok.begin = begin;
	stok.end = end;
	stok.word = val;
	ret.push_back(stok);

	return ret;
}

//!
//! \brief Remove leading whitespace from a string.
//!
//! \param[in,out] str The string to be modified.
//! \retval char* A pointer to the modified input string.
//!
//========================================================================
char* chomp_leading (char* str)
{

	if (str != 0)
	{
		char* p = str;

		while ((*p != '\0') && (isascii(*p)) && (isspace(*p)))
		{
			p++;
		}
		memmove(str, p, strlen(p)+1);
	}

	return str;
}

//!
//! \brief Remove trailing whitespace from a string.
//!
//! \param[in,out] str The string to be modified.
//!
//! \retval char* A pointer to the modified input string.
//!
//========================================================================
char* chomp (char* str)
{
    int len = strlen(str);

    while ((len >= 0) && 
            ((str[len - 1] == ' ') || (str[len - 1] == '\t') || (str[len - 1] == '\r') || (str[len - 1] == '\n')))
    {
            *(str + len - 1) = '\0';
            len--;
    }
	return str;
}

//!
//! \brief Strip out the specified characters from a string.
//!
//! \param[in, out] str The string to be stripped.
//! \param[in] chars The character(s) to remove.
//!
//========================================================================
void StrRemoveChars (string& str, const char* chars) 
{

	for (uint32_t i = 0; i < strlen(chars); ++i) 
	{
		str.erase(remove(str.begin(), str.end(), chars[i]), 
			str.end());
	}
}

//!
//! \brief Check for an empty string.
//! A string is considered empty if it contains nothing but whitespace
//! characters.
//!
//! \param[in] str The string to be checked.
//!
//! \retval bool Returns true if the string contains nothing but 
//! whitespace (or nothing at all).
//!
//========================================================================
bool StrIsEmpty (const char* str)
{

	if (str != 0)
	{
		while (*str != '\0')
		{
			if (!isspace(*str))
			{
				return false;
			}
			str++;
		}
	}

	return true;
}

//!
//! \brief Convert a UNIX epoch time value to a string.
//! This function converts a UNIX epoch time value into a 
//! ctime()-style string (minus the trailing newline).
//!
//! \param[in] t The time to convert.
//! \param[out] s The string to contain the time information.
//!
//========================================================================
void TimeToString (time_t t, string& s)
{

	if (t == 0)
	{
		s = "<Unknown>";
	}
	else
	{
		s = chomp(ctime(&t));
	}
}

//!
//! \brief Resolve a hostname or IP address to an in_addr.
//!
//! \param[in] addrstr The hostname/IP address to resolve.
//! \param[out] addr The resolved address.
//!
//! \retval bool Returns true if the address was resolved, or false
//! if an error occurred.
//!
//========================================================================
bool Resolve (const char* addrstr, in_addr* addr)
{

	// IS IT A DOTTED QUAD?
	addr->s_addr = inet_addr(addrstr);
	if (addr->s_addr != INADDR_NONE)
	{
		return true;
	}

	// NOPE - MUST BE A HOSTNAME
	hostent* hp = gethostbyname(addrstr);
	if ((hp != 0) && (hp->h_addrtype == AF_INET) && (hp->h_addr_list[0] != 0))
	{
		addr->s_addr = *(reinterpret_cast<u_long*>(hp->h_addr_list[0]));
		return true;
	}

	// UNABLE TO RESOLVE!
	return false;
}

//!
//! \brief Create a plain ASCII text file containing the current process
//! ID.
//!
//! \param[in] pidfile Full path to the PID file to create.
//!
//! \retval bool Returns true if the file was created successfully.
//!
//========================================================================
bool CreatePidFile (string pidfile)
{
	FILE* f = fopen(pidfile.c_str(), "wt");

	if (f != 0)
	{
		fprintf(f, "%d\n", getpid());
		fclose(f);
		return true;
	}
	LogWrite(LEVEL_ERROR, "Cannot create pid file");
	return false;
}

//!
//! \brief Copy a Pascal-style string (with a leading length byte) into a 
//! C-style (NUL-terminated) string.
//!
//! \param[in] srcstr Pointer to the source string
//! \param[out] deststr Pointer to the destination string
//! \retval uint32_t The number of bytes processed (including the length byte)
//!
//========================================================================
uint32_t CopyPascalString(uint8_t* srcstr, char* deststr)
{
	uint32_t len = *srcstr++;

	memcpy(deststr, srcstr, len);
	deststr[len] = '\0';

	return (len + 1);
}

//!
//! \brief Copy a Pascal-style string (with a leading length byte) into a 
//! C++ std::string object.
//!
//! \param[in] srcstr Pointer to the source string
//! \param[in,out] deststr Reference to the destination string
//! \retval uint32_t The number of bytes processed (including the length byte)
//!
//========================================================================
uint32_t CopyPascalString(uint8_t* srcstr, string& deststr)
{
	uint32_t len = *srcstr++;

	deststr.clear();
	for (uint32_t i = 0; i < len; ++i)
	{
		deststr.push_back(*srcstr++);
	}
	return (len + 1);
}

/**
 * $DateTime: 2015/08/23 13:00:12 $                                                                
 * $Id: //depot/sircond/util.h#3 $
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

#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdint.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

//!
//! \brief 'smart' tokenizeing with start/end positions
//!
struct smart_word_token 
{
    string word;
    size_t begin;
    size_t end;

    smart_word_token& operator= (const smart_word_token& op) 
    {
        word = op.word;
        begin = op.begin;
        end = op.end;
        return *this;
    }
};

vector<string> StrTokenize (string in_str, 
			string in_split, 
			bool return_partial=true);
vector<smart_word_token> BaseStrTokenize (string in_str, 
					string in_split, 
					string in_quote);
vector<smart_word_token> NetStrTokenize (string in_str, string in_split);

#ifdef WIN32
void sleep (uint32_t seconds);
#endif

char* chomp (char* str);
char* chomp_leading (char* str);
bool StrIsEmpty (const char* str);
void StrRemoveChars (string& str, const char* chars);
void TimeToString (time_t t, string& s);
bool Resolve (const char* addrstr, in_addr* addr);
bool CreatePidFile (string pidfile);
uint32_t CopyPascalString(uint8_t* srcstr, char* deststr);
uint32_t CopyPascalString(uint8_t* srcstr, string& deststr);
#endif

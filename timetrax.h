/**
 * $DateTime: 2015/08/23 13:00:12 $                                                                 
 * $Id: //depot/sircond/timetrax.h#9 $
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
//! \file timetrax.h
//!
//! \brief Declarations for the CTTS100 class.
//!
//! The TTS-100  is a hardware interface box that links a SiriusConnect 
//! satellite receiver to a PC via USB. Produced circa 2005 by the 
//! now-defunct TimeTrax Technologies, Inc., this was an early means of
//! allowing a PC to control a SiriusConnect satellite receiver. 
//!
//! TimeTrax Technologies was an interesting company. Their first product was
//! a software package for the XMPCR receiver, one of the first satellite radio 
//! receivers with a serial port intended for control by a PC. By monitoring 
//! the metadata from the satellite, this software could determine when songs
//! began/ended and thus capture the audio stream into individual song files.
//! Users could let this software run overnight and wake up the next morning 
//! to a nice collection of .MP3 music files. This caused a wee bit of a stir
//! in the music industry with repercussions (e.g. the "Music Royalty Fee")
//! that continue to this day.
//!
//! Since there was no PC-controllable Sirius receiver available at that time, 
//! TimeTrax developed the TTS-100 interface. Sirius' car stereo receivers were 
//! modular in design with a TTL level serial interface. Car stereo 
//! manufacturers such as Pioneer and Kenwood created interface units which 
//! mated the generic SiriusConnect tuner module to their own car stereo units. 
//! TimeTrax leveraged this feature and created the TTS-100, which plugged into 
//! the same jack on the SiriusConnect tuners and spoke the same protocol. 
//! 
//! To accompany the TTS-100, TimeTrax developed a software package called Recast.
//! In a ham-handed attempt at control, the Recast software would "phone home" 
//! to the TimeTrax registration servers via the Internet every time it was 
//! launched, generating endless customer ire. This, in addition to general 
//! bugginess, spurred the efforts to reverse-engineer and development alternatives 
//! to Recast.
//!
//! In a similar vein, the TTS-100 hardware was designed to only allow "official" 
//! software to work with it. After a power cycle it requires a challenge-response
//! authentication dialog with the software. Until the software reponds correctly
//! to the challenge the control functions of the SiriusConnect receiver are 
//! inaccessible. Fortunately for the users this code was poorly implemented and 
//! vulnerable to a simple replay attack: by capturing the challenge reponse from
//! a legitimate copy of Recast and changing 2 bytes with a simple XOR operation,
//! third-party software can produce an acceptable reponse to any challenge. 
//!
//! TimeTrax Technologies disappeared in 2006 under mysterious circumstances; 
//! one day the TimeTrax web site vanished and HTTP requests were redirected to 
//! the XM satellite radio web site; you can imagine the consternation when the
//! license servers that were required to keep the Recast softwrae running went
//! offline. Fortunately by that time there were several third-party software 
//! packages available which did not require permission in order to run.
//! 
//! The company and its software may have sucked, but they made pretty good 
//! hardware. My TTS-100 is still working after more than 10 years of continuous 
//! use.
//!
//! @{
//!

#ifndef _TIMETRAX_H_
#define _TIMETRAX_H_

#include "sircon.h"

//!
//! \brief The interface class for the TTS-100 hardware.
//!
class CTTS100 : public CSirCon
{
public:
	CTTS100 (const string& device) : CSirCon(device) { }
	bool OnStart ();
    bool QueryVersion (uint32_t& major, uint32_t& minor);
    bool Authenticate ();

protected:
	CTTS100 ();
};

#endif



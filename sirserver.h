/**
 * $DateTime: 2015/08/23 13:00:12 $                                                                
 * $Id: //depot/sircond/sirserver.h#1 $
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
//! \file sirserver.h
//!
//! \brief Declarations for the SiriusConnect Server class.
//!

#ifndef _SIRSERVER_H_
#define _SIRSERVER_H_

#include "pch.h"
#include <map>
#include <list>
#include <typeinfo>
#include <typeindex>
#include "observer.h"
#include "server.h"
#include "sirclient.h"
#include "ctimer.h"
#include "timetrax.h"

using std::list;
using std::map;

// HANDLER FUNCTION SIGNATURES
typedef bool (CSirServer::*VALIDATIONFUNC)(CLIENT*, vector<string>&);
typedef void (CSirServer::*HANDLERFUNC)(CLIENT*,vector<string>&);
typedef void (CSirServer::*EVTHANDLER)(SCEvent&);

//!
//! \brief A SiriusConnect Server object
//!
//! This class provides the meat of the SiriusConnect Server's
//! functionality, including handlers for all of the Sirius
//! radio events as well as handlers for client commands.
//!
class CSirServer : public SERVER, public IObserver<SCEvent>
{
public:
	CSirServer (const string& device);
	bool OnStart ();
	void OnExit ();
	void ProcessCommand (CLIENT* client, string& cmd);

protected:
	virtual void OnDrop (CLIENT* client);

private:
	CSirServer ();
	virtual CLIENT* NewClient (SOCKET s) { return new CSirClient(*this, s, m_bufsize); }

	bool IsWaitingForControl(CLIENT* client);
	bool HasControl(CLIENT* client);
	bool AcquireControl(CLIENT* client);
	void ReleaseControl(CLIENT* client);
	bool CopyString(char* dest, string& src, uint32_t maxlen);
	void Notify(CLIENT* client, string msg);
	void NotifyResult(CLIENT* client, std::future<SCRESULT>& result);
	void NotifyAll(string msg);

	// CLIENT MESSAGE HANDLERS
	bool ValidateGetActivation(CLIENT* client, vector<string>& tokens);
	bool ValidateGetGain(CLIENT* client, vector<string>& tokens);
	bool ValidateGetMute(CLIENT* client, vector<string>& tokens);	
	bool ValidateGetPower(CLIENT* client, vector<string>& tokens);
	bool ValidateGetChannel(CLIENT* client, vector<string>& tokens);
	bool ValidateGetChannelInfo(CLIENT* client, vector<string>& tokens);
	bool ValidateGetSongInfo(CLIENT* client, vector<string>& tokens);
	bool ValidateGetSID(CLIENT* client, vector<string>& tokens);
	bool ValidateGetTZInfo(CLIENT* client, vector<string>& tokens);
	bool ValidateGetTime(CLIENT* client, vector<string>& tokens);
	bool ValidateGetStatus(CLIENT* client, vector<string>& tokens);
	bool ValidateGetRSSI(CLIENT* client, vector<string>& tokens);

	void ProcessGetActivation(CLIENT* client, vector<string>& tokens);
	void ProcessGetGain(CLIENT* client, vector<string>& tokens);
	void ProcessGetMute(CLIENT* client, vector<string>& tokens);
	void ProcessGetPower(CLIENT* client, vector<string>& tokens);
	void ProcessGetChannel(CLIENT* client, vector<string>& tokens);
	void ProcessGetChannelInfo(CLIENT* client, vector<string>& tokens);
	void ProcessGetSongInfo(CLIENT* client, vector<string>& tokens);
	void ProcessGetSID(CLIENT* client, vector<string>& tokens);
	void ProcessGetTZInfo(CLIENT* client, vector<string>& tokens);
	void ProcessGetTime(CLIENT* client, vector<string>& tokens);
	void ProcessGetStatus(CLIENT* client, vector<string>& tokens);
	void ProcessGetRSSI(CLIENT* client, vector<string>& tokens);

	bool ValidateSetReset(CLIENT* client, vector<string>& tokens);
	bool ValidateSetGain(CLIENT* client, vector<string>& tokens);
	bool ValidateSetMute(CLIENT* client, vector<string>& tokens);
	bool ValidateSetPower(CLIENT* client, vector<string>& tokens);
	bool ValidateSetChannel(CLIENT* client, vector<string>& tokens);
	bool ValidateSetTZInfo(CLIENT* client, vector<string>& tokens);
	bool ValidateSetAsync(CLIENT* client, vector<string>& tokens);

	void ProcessSetReset(CLIENT* client, vector<string>& tokens);
	void ProcessSetGain(CLIENT* client, vector<string>& tokens);
	void ProcessSetMute(CLIENT* client, vector<string>& tokens);
	void ProcessSetPower(CLIENT* client, vector<string>& tokens);
	void ProcessSetChannel(CLIENT* client, vector<string>& tokens);
	void ProcessSetTZInfo(CLIENT* client, vector<string>& tokens);
	void ProcessSetAsync(CLIENT* client, vector<string>& tokens);

	bool ValidateGet(CLIENT* client, vector<string>& tokens);
	bool ValidateSet(CLIENT* client, vector<string>& tokens);
	bool ValidateControl(CLIENT* client, vector<string>& tokens);
	bool ValidateQuit(CLIENT* client, vector<string>& tokens);

	void ProcessGet(CLIENT* client, vector<string>& tokens);
	void ProcessSet(CLIENT* client, vector<string>& tokens);
	void ProcessControl (CLIENT* client, vector<string>& tokens);
	void ProcessQuit (CLIENT* client, vector<string>& tokens);

	// SIRIUS EVENT HANDLERS
	void OnSCEStartup(SCEvent& param);
	void OnSCEGetResult(SCEvent& param);
	void OnSCESetResult(SCEvent& param);
	void OnSCESID(SCEvent& param);
	void OnSCEGain(SCEvent& param);
	void OnSCEMute(SCEvent& param);
	void OnSCEChannelInfo(SCEvent& param);
	void OnSCESongInfo(SCEvent& param);
	void OnSCEChannel(SCEvent& param);
	void OnSCEChannelMap(SCEvent& param);
	void OnSCEStatus(SCEvent& param);
	void OnSCERSSI(SCEvent& param);
	void OnSCESignal(SCEvent& param);
	void OnSCEPower(SCEvent& param);
	void OnSCEReset(SCEvent& param);
	void OnSCETime(SCEvent& param);
	void OnSCETZInfo(SCEvent& param);
	void OnSCEShutdown(SCEvent& param);
	void Update(SCEvent& e);

	bool m_initialized;

	map<string, std::pair<VALIDATIONFUNC, HANDLERFUNC>> m_cmd_handlers;
	map<string, std::pair<VALIDATIONFUNC,HANDLERFUNC>> m_get_handlers;
	map<string, std::pair<VALIDATIONFUNC, HANDLERFUNC>> m_set_handlers;
	map<std::type_index,EVTHANDLER> m_evt_handlers;
	list<CLIENT*> m_control_queue;
	CLIENT* m_controller;
	std::mutex m_queue_mutex;
	sr::CTimer m_timermgr;
	//! \brief The SiriusConnect tuner
	//! \note If no TTS-100 hardware is being used, this can be replaced by a CSirCon object instead.
	CTTS100 m_sircon;
};

#endif

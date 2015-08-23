/*
 * $DateTime: 2015/08/23 13:00:12 $                                                               
 * $Id: //depot/sircond/sirserver.cpp#1 $
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
#include <sstream>
#include <typeinfo>
#include "sirserver.h"

using std::stringstream;

static const uint16_t SIRCOND_PORT = 6114U;		// PORT FOR TEXT CLIENTS
static const uint32_t SIRCOND_BUFSIZE = 512U;	// SIZE OF CLIENT I/O BUFFERS
static const SCP_CHANNEL_INDEX SIRCOND_DEFAULT_CHANNEL = 184U;

//========================================================================
CSirServer::CSirServer(const string& device) : m_initialized(false), m_controller(0), m_sircon(device)
{

	// INITIALIZE EVENT HANDLER TABLE
	m_evt_handlers[typeid(SCEStartup)] = &CSirServer::OnSCEStartup;
	m_evt_handlers[typeid(SCEGetResult)] = &CSirServer::OnSCEGetResult;
	m_evt_handlers[typeid(SCESetResult)] = &CSirServer::OnSCESetResult;
	m_evt_handlers[typeid(SCESiriusID)] = &CSirServer::OnSCESID;
	m_evt_handlers[typeid(SCEGain)] = &CSirServer::OnSCEGain;
	m_evt_handlers[typeid(SCEMute)] = &CSirServer::OnSCEMute;
	m_evt_handlers[typeid(SCEChannelInfo)] = &CSirServer::OnSCEChannelInfo;
	m_evt_handlers[typeid(SCESongInfo)] = &CSirServer::OnSCESongInfo;
	m_evt_handlers[typeid(SCEChannel)] = &CSirServer::OnSCEChannel;
	m_evt_handlers[typeid(SCEChannelMap)] = &CSirServer::OnSCEChannelMap;
	m_evt_handlers[typeid(SCEStatus)] = &CSirServer::OnSCEStatus;
	m_evt_handlers[typeid(SCERSSI)] = &CSirServer::OnSCERSSI;
	m_evt_handlers[typeid(SCESignal)] = &CSirServer::OnSCESignal;
	m_evt_handlers[typeid(SCEReset)] = &CSirServer::OnSCEReset;
	m_evt_handlers[typeid(SCEPower)] = &CSirServer::OnSCEPower;
	m_evt_handlers[typeid(SCETime)] = &CSirServer::OnSCETime;
	m_evt_handlers[typeid(SCETimeZoneInfo)] = &CSirServer::OnSCETZInfo;
	m_evt_handlers[typeid(SCEShutdown)] = &CSirServer::OnSCEShutdown;

	// REGISTER FOR SIRIUS EVENT NOTIFICATIONS
	m_sircon.Attach(this);

	// INITIALIZE MAIN COMMAND HANDLER TABLE
	m_cmd_handlers["GET"] = { &CSirServer::ValidateGet, &CSirServer::ProcessGet };
	m_cmd_handlers["SET"] = { &CSirServer::ValidateSet, &CSirServer::ProcessSet };
	m_cmd_handlers["CONTROL"] = { &CSirServer::ValidateControl, &CSirServer::ProcessControl };
	m_cmd_handlers["QUIT"] = { &CSirServer::ValidateQuit, &CSirServer::ProcessQuit };

	// INITIALIZE GET HANDLER TABLE
	m_get_handlers["GAIN"] = { &CSirServer::ValidateGetGain, &CSirServer::ProcessGetGain };
	m_get_handlers["MUTE"] = { &CSirServer::ValidateGetMute, &CSirServer::ProcessGetMute };
	m_get_handlers["POWER"] = { &CSirServer::ValidateGetPower, &CSirServer::ProcessGetPower };
	m_get_handlers["CHANNEL"] = { &CSirServer::ValidateGetChannel, &CSirServer::ProcessGetChannel };
	m_get_handlers["CHANNELINFO"] = { &CSirServer::ValidateGetChannelInfo, &CSirServer::ProcessGetChannelInfo };
	m_get_handlers["SONGINFO"] = { &CSirServer::ValidateGetSongInfo, &CSirServer::ProcessGetSongInfo };
	m_get_handlers["SID"] = { &CSirServer::ValidateGetSID, &CSirServer::ProcessGetSID };
	m_get_handlers["TZINFO"] = { &CSirServer::ValidateGetTZInfo, &CSirServer::ProcessGetTZInfo };
	m_get_handlers["TIME"] = { &CSirServer::ValidateGetTime, &CSirServer::ProcessGetTime };
	m_get_handlers["STATUS"] = { &CSirServer::ValidateGetStatus, &CSirServer::ProcessGetStatus };
	m_get_handlers["RSSI"] = { &CSirServer::ValidateGetRSSI, &CSirServer::ProcessGetRSSI };

	// INITIALIZE SET HANDLER TABLE
	m_set_handlers["RESET"] = { &CSirServer::ValidateSetReset, &CSirServer::ProcessSetReset };
	m_set_handlers["GAIN"] = { &CSirServer::ValidateSetGain, &CSirServer::ProcessSetGain };
	m_set_handlers["MUTE"] = { &CSirServer::ValidateSetMute, &CSirServer::ProcessSetMute };
	m_set_handlers["POWER"] = { &CSirServer::ValidateSetPower, &CSirServer::ProcessSetPower };
	m_set_handlers["CHANNEL"] = { &CSirServer::ValidateSetChannel, &CSirServer::ProcessSetChannel };
	m_set_handlers["TZINFO"] = { &CSirServer::ValidateSetTZInfo, &CSirServer::ProcessSetTZInfo };
	m_set_handlers["ASYNC"] = { &CSirServer::ValidateSetAsync, &CSirServer::ProcessSetAsync };

	SetPort(SIRCOND_PORT); 
	SetBufferSize(SIRCOND_BUFSIZE);
}

//========================================================================
bool CSirServer::OnStart ()
{

	// FIRST START THE BASE CLASS
	if (!SERVER::OnStart())
	{
		return false;
	}

	// START THE TIMER MANAGER
	if (!m_timermgr.Start())
	{
		return false;
	}
	while (m_timermgr.GetState() == STARTING)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	if (m_timermgr.GetState() != RUNNING)
	{
		return false;
	}

	// START THE RADIO INTERFACE
	if (!m_sircon.Start())
	{
		return false;
	}
	while (m_sircon.GetState() == STARTING)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	if (m_sircon.GetState() != RUNNING)
	{
		return false;
	}

	// BEGIN THE INITIALIZATION SEQUENCE
	m_sircon.GetPower();

	return true;
}

//========================================================================
void CSirServer::OnExit ()
{

    LogWrite(LEVEL_INFO, "CSirServer::OnExit()");
    SERVER::OnExit();

#ifndef WIN32
    LogWrite(LEVEL_DEBUG, "Raising SIGTERM...");

    // FOR REASONS UNKNOWN RAISE DOES NOT WORK, SO USE KILL
    kill(getpid(), SIGTERM);
#endif
}

//!
//! \brief Determine whether a client is in the control waiting queue.
//!
//! \note Assumes the caller is already holding the wait queue mutex.
//!
//========================================================================
bool CSirServer::IsWaitingForControl (CLIENT* client)
{

	list<CLIENT*>::iterator i = m_control_queue.begin();
	while (i != m_control_queue.end())
	{
		if (*i == client)
		{
			return true;
		}
		else
		{
			i++;
		}
	}
	return false;
}

//!
//! \brief Determine whether a client owns the control mutex
//!
//! \note Assumes the caller is already holding the wait queue mutex.
//!
//========================================================================
bool CSirServer::HasControl(CLIENT* client)
{

	return (client == m_controller);
}

//========================================================================
bool CSirServer::AcquireControl (CLIENT* client)
{
	std::lock_guard<std::mutex> lk(m_queue_mutex);
	bool result = false;

	// IF NOBODY CURRENTLY HAS THE CONN, TAKE IT NOW
	if ((m_controller == 0) && m_control_queue.empty())
	{
		m_controller = client;
		result = true;
	}
	else
	{
		if (HasControl(client))
		{
			// ALREADY HAS IT!
			result = true;
		}		
		else if (!IsWaitingForControl(client))
		{
			// NOT ALREADY IN THE WAIT QUEUE
			m_control_queue.push_back(client);
		}
	}

	return result;
}

//========================================================================
void CSirServer::ReleaseControl (CLIENT* client)
{
	std::lock_guard<std::mutex> lk(m_queue_mutex);

	if (HasControl(client))
	{
		// CLIENT HAD THE CONN
		if (m_control_queue.empty())
		{
			// NO ONE ELSE IS WAITING
			m_controller = 0;
		}
		else
		{
			// GIVE CONTROL TO THE NEXT GUY
			m_controller = m_control_queue.front();
			m_control_queue.pop_front();
			Notify(m_controller, "CONTROL,ACQUIRED\n");
		}
	}
	else
	{
		// CLIENT DID NOT HAVE THE CONN
		list<CLIENT*>::iterator i = m_control_queue.begin();
		while (i != m_control_queue.end())
		{
			if (*i == client)
			{
				i = m_control_queue.erase(i);
			}
			else
			{
				i++;
			}
		}
	}
}

//========================================================================
void CSirServer::Notify (CLIENT* client, string msg)
{

	if (client != 0)
	{
		client->Send(reinterpret_cast<const uint8_t*>(msg.data()), msg.size());
	}
}

//========================================================================
void CSirServer::NotifyResult(CLIENT* client, std::future<SCRESULT>& result)
{
	SCRESULT rc = result.get();
	stringstream ss;

	switch (rc)
	{
		case SCR_SUCCESS:
			ss << "OK";
		break;

		case SCR_TIMEOUT:
			ss << "TIMEOUT";
		break;

		default:
			ss << "ERROR";
		break;
	}
	ss << std::endl;

	Notify(client, ss.str());
}

//========================================================================
void CSirServer::NotifyAll (string msg)
{

	Broadcast(reinterpret_cast<const uint8_t*>(msg.data()), msg.size());
}

//========================================================================
void CSirServer::OnDrop (CLIENT* client)
{

	ReleaseControl(client);
	SERVER::OnDrop(client);
}

/**
 * CLIENT COMMAND HANDLERS
 */

//========================================================================
bool CSirServer::ValidateGetGain (CLIENT* client, vector<string>& tokens)
{

	return (tokens.size() == 2);
}

//========================================================================
bool CSirServer::ValidateGetMute (CLIENT* client, vector<string>& tokens)
{

	return (tokens.size() == 2);
}

//========================================================================
bool CSirServer::ValidateGetPower (CLIENT* client, vector<string>& tokens)
{

	return (tokens.size() == 2);
}

//========================================================================
bool CSirServer::ValidateGetChannel(CLIENT* client, vector<string>& tokens)
{

	return (tokens.size() == 2);
}

//========================================================================
bool CSirServer::ValidateGetChannelInfo (CLIENT* client, vector<string>& tokens)
{

	return (tokens.size() == 2);
}

//========================================================================
bool CSirServer::ValidateGetSongInfo (CLIENT* client, vector<string>& tokens)
{

	return (tokens.size() == 2);
}

//========================================================================
bool CSirServer::ValidateGetSID (CLIENT* client, vector<string>& tokens)
{

	return (tokens.size() == 2);
}

//========================================================================
bool CSirServer::ValidateGetTZInfo (CLIENT* client, vector<string>& tokens)
{

	return (tokens.size() == 2);
}

//========================================================================
bool CSirServer::ValidateGetTime (CLIENT* client, vector<string>& tokens)
{

	return (tokens.size() == 2);
}

//========================================================================
bool CSirServer::ValidateGetStatus (CLIENT* client, vector<string>& tokens)
{

	return (tokens.size() == 3);
}

//========================================================================
bool CSirServer::ValidateGetRSSI (CLIENT* client, vector<string>& tokens)
{

	return (tokens.size() == 2);
}

//========================================================================
void CSirServer::ProcessGetGain (CLIENT* client, vector<string>& tokens)
{
	std::future<SCRESULT> r = m_sircon.GetGain();

	NotifyResult(client, r);
}

//========================================================================
void CSirServer::ProcessGetMute(CLIENT* client, vector<string>& tokens)
{
	std::future<SCRESULT> r = m_sircon.GetMute();

	NotifyResult(client, r);
}

//========================================================================
void CSirServer::ProcessGetPower(CLIENT* client, vector<string>& tokens)
{
	std::future<SCRESULT> r = m_sircon.GetPower();

	NotifyResult(client, r);
}

//========================================================================
void CSirServer::ProcessGetChannel(CLIENT* client, vector<string>& tokens)
{
	std::future<SCRESULT> r = m_sircon.GetChannel();

	NotifyResult(client, r);
}

//========================================================================
void CSirServer::ProcessGetChannelInfo(CLIENT* client, vector<string>& tokens)
{
	std::future<SCRESULT> r = m_sircon.GetChannelInfo(m_sircon.GetCurrentChannel());

	NotifyResult(client, r);
}

//========================================================================
void CSirServer::ProcessGetSongInfo(CLIENT* client, vector<string>& tokens)
{
	std::future<SCRESULT> r = m_sircon.GetSongInfo(m_sircon.GetCurrentChannel());

	NotifyResult(client, r);
}

//========================================================================
void CSirServer::ProcessGetTZInfo(CLIENT* client, vector<string>& tokens)
{
	std::future<SCRESULT> r = m_sircon.GetTZ();

	NotifyResult(client, r);
}

//========================================================================
void CSirServer::ProcessGetTime(CLIENT* client, vector<string>& tokens)
{
	std::future<SCRESULT> r = m_sircon.GetTime();

	NotifyResult(client, r);
}

//========================================================================
void CSirServer::ProcessGetStatus(CLIENT* client, vector<string>& tokens)
{
	SCP_STATUS_TYPE st = static_cast<SCP_STATUS_TYPE>(strtoul(tokens[2].c_str(), 0, 10));
	std::future<SCRESULT> r = m_sircon.GetStatus(st);

	NotifyResult(client, r);
}

//========================================================================
void CSirServer::ProcessGetSID(CLIENT* client, vector<string>& tokens)
{
	std::future<SCRESULT> r = m_sircon.GetSID();

	NotifyResult(client, r);
}

//========================================================================
void CSirServer::ProcessGetRSSI(CLIENT* client, vector<string>& tokens)
{
	std::future<SCRESULT> r = m_sircon.GetRSSI();

	NotifyResult(client, r);
}

//========================================================================
bool CSirServer::ValidateSetReset(CLIENT* client, vector<string>& tokens)
{

	return (tokens.size() == 2);
}

//========================================================================
bool CSirServer::ValidateSetGain(CLIENT* client, vector<string>& tokens)
{

	return (tokens.size() == 3);
}

//========================================================================
bool CSirServer::ValidateSetMute(CLIENT* client, vector<string>& tokens)
{

	return (tokens.size() == 3);
}

//========================================================================
bool CSirServer::ValidateSetPower(CLIENT* client, vector<string>& tokens)
{

	return (tokens.size() == 3);
}

//========================================================================
bool CSirServer::ValidateSetChannel(CLIENT* client, vector<string>& tokens)
{

	return (tokens.size() == 3);
}

//========================================================================
bool CSirServer::ValidateSetTZInfo(CLIENT* client, vector<string>& tokens)
{

	return (tokens.size() == 4);
}

//========================================================================
bool CSirServer::ValidateSetAsync(CLIENT* client, vector<string>& tokens)
{

	return (tokens.size() == 3);
}

//========================================================================
void CSirServer::ProcessSetReset(CLIENT* client, vector<string>& tokens)
{
	std::future<SCRESULT> r = m_sircon.Reset();

	NotifyResult(client, r);
}

//========================================================================
void CSirServer::ProcessSetGain(CLIENT* client, vector<string>& tokens)
{
	int8_t gain = static_cast<int8_t>(atoi(tokens[2].c_str()));
	std::future<SCRESULT> r = m_sircon.SetGain(gain);

	NotifyResult(client, r);
}

//========================================================================
void CSirServer::ProcessSetMute(CLIENT* client, vector<string>& tokens)
{
	bool on = (strtoul(tokens[2].c_str(), 0, 10) > 0u);
	std::future<SCRESULT> r = m_sircon.SetMute(on);

	NotifyResult(client, r);
}

//========================================================================
void CSirServer::ProcessSetPower(CLIENT* client, vector<string>& tokens)
{
	uint32_t mode = strtoul(tokens[2].c_str(), 0, 10);
	std::future<SCRESULT> r = m_sircon.SetPower(mode);

	NotifyResult(client, r);
}

//========================================================================
void CSirServer::ProcessSetChannel(CLIENT* client, vector<string>& tokens)
{
	SCP_CHANNEL_INDEX channel = static_cast<SCP_CHANNEL_INDEX>(strtoul(tokens[2].c_str(), 0, 10));
	std::future<SCRESULT> r = m_sircon.SetChannel(channel);

	NotifyResult(client, r);
}

//========================================================================
void CSirServer::ProcessSetTZInfo(CLIENT* client, vector<string>& tokens)
{
	int16_t offset = atoi(tokens[2].c_str());
	bool dst = (strtoul(tokens[3].c_str(), 0, 10) > 0u);
	std::future<SCRESULT> r = m_sircon.SetTZ(offset, dst);

	NotifyResult(client, r);
}

//========================================================================
void CSirServer::ProcessSetAsync(CLIENT* client, vector<string>& tokens)
{
	uint32_t flags = strtoul(tokens[2].c_str(), 0, 10);
	std::future<SCRESULT> r = m_sircon.EnableAsyncNotifications(flags);

	NotifyResult(client, r);
}

//========================================================================
bool CSirServer::ValidateGet(CLIENT* client, vector<string>& tokens)
{

	return (tokens.size() >= 2);
}

//========================================================================
bool CSirServer::ValidateSet(CLIENT* client, vector<string>& tokens)
{

	return ((tokens.size() >= 2) && HasControl(client));
}

//========================================================================
bool CSirServer::ValidateControl(CLIENT* client, vector<string>& tokens)
{

	return (tokens.size() == 2);
}

//========================================================================
bool CSirServer::ValidateQuit(CLIENT* client, vector<string>& tokens)
{

	return (tokens.size() == 1);
}

//========================================================================
void CSirServer::ProcessGet (CLIENT* client, vector<string>& tokens)
{

	LogWrite(LEVEL_DEBUG, "ProcessGet() with %u tokens.", tokens.size());

	map<string, std::pair<VALIDATIONFUNC,HANDLERFUNC>>::iterator it = m_get_handlers.find(tokens[1]);
	if (it != m_get_handlers.end())
	{
		VALIDATIONFUNC v = it->second.first;
		HANDLERFUNC p = it->second.second;

		if ((this->*v)(client, tokens))
		{
			(this->*p)(client, tokens);
		}
		else
		{
			LogWrite(LEVEL_ERROR, "Invalid GET command %s.", tokens[1].c_str());
		}
	}
	else
	{
		LogWrite(LEVEL_ERROR, "Unknown GET command %s.", tokens[1].c_str());
	}
}

//========================================================================
void CSirServer::ProcessSet (CLIENT* client, vector<string>& tokens)
{

	LogWrite(LEVEL_DEBUG, "ProcessSet() with %u tokens.", tokens.size());

	map<string, std::pair<VALIDATIONFUNC, HANDLERFUNC>>::iterator it = m_set_handlers.find(tokens[1]);
	if (it != m_set_handlers.end())
	{
		VALIDATIONFUNC v = it->second.first;
		HANDLERFUNC p = it->second.second;

		if ((this->*v)(client, tokens))
		{
			(this->*p)(client, tokens);
		}
		else
		{
			LogWrite(LEVEL_ERROR, "Invalid SET command %s.", tokens[1].c_str());
		}
	}
	else
	{
		LogWrite(LEVEL_ERROR, "Unknown SET command %s.", tokens[1].c_str());
	}
}

//========================================================================
void CSirServer::ProcessControl (CLIENT* client, vector<string>& tokens)
{
	stringstream ss;

	if (tokens[1] == "ACQUIRE")
	{
		if (AcquireControl(client))
		{
			ss << "CONTROL,ACQUIRED";
		}
		else
		{
			ss << "CONTROL,PENDING";
		}
	}
	else if (tokens[1] == "RELEASE")
	{
		ReleaseControl(client);
		ss << "CONTROL,RELEASED";
	}
	ss << std::endl;
	Notify(client, ss.str());
}

//========================================================================
void CSirServer::ProcessQuit (CLIENT* client, vector<string>& tokens)
{

	Drop(client);
}

//========================================================================
void CSirServer::ProcessCommand (CLIENT* client, string& cmd)
{
	vector<string> tokens = StrTokenize(cmd, " ");

	if (tokens.size() > 0)
	{
		// INVOKE THE CORRESPONDING HANDLER
		map<string, std::pair<VALIDATIONFUNC, HANDLERFUNC>>::iterator it = m_cmd_handlers.find(tokens[0]);
		if (it != m_cmd_handlers.end())
		{
			VALIDATIONFUNC v = it->second.first;
			HANDLERFUNC p = it->second.second;

			if ((this->*v)(client, tokens))
			{
				(this->*p)(client, tokens);
			}
			else
			{
				stringstream ss;

				ss << "ERROR" << std::endl;
				Notify(client, ss.str());
				LogWrite(LEVEL_ERROR, "Invalid command %s.", tokens[0].c_str());
			}
		}
		else
		{
			LogWrite(LEVEL_ERROR, "Unknown command %s.", tokens[0].c_str());
		}
	}
}

//========================================================================
bool CSirServer::CopyString (char* dest, string& src, uint32_t maxlen)
{

    *dest = '\0';
    if (!src.empty())
    {
        strncpy(dest, src.c_str(), maxlen);
        dest[maxlen-1] = '\0';
    }
    return true;
}


//
// SIRIUS EVENT HANDLERS
//

//========================================================================
void CSirServer::OnSCEStartup (SCEvent& param)
{
	SCEStartup& s = static_cast<SCEStartup&>(param);
	stringstream ss;

	ss << s << std::endl;
	NotifyAll(ss.str());
}

//========================================================================
void CSirServer::OnSCEGetResult(SCEvent& param)
{
	SCEGetResult& s = static_cast<SCEGetResult&>(param);
	stringstream ss;

	ss << s << std::endl;
	Notify(m_controller, ss.str());
}

//========================================================================
void CSirServer::OnSCESetResult(SCEvent& param)
{
	SCESetResult& s = static_cast<SCESetResult&>(param);
	stringstream ss;

	ss << s << std::endl;
	Notify(m_controller, ss.str());
}

//========================================================================
void CSirServer::OnSCESID(SCEvent& param)
{
	SCESiriusID& s = static_cast<SCESiriusID&>(param);
	stringstream ss;

	ss << s << std::endl;
	NotifyAll(ss.str());
}

//========================================================================
void CSirServer::OnSCEGain(SCEvent& param)
{
	SCEGain& g = static_cast<SCEGain&>(param);
	stringstream ss;

	ss << g << std::endl;
	NotifyAll(ss.str());
}

//========================================================================
void CSirServer::OnSCEMute(SCEvent& param)
{
	SCEMute& m = static_cast<SCEMute&>(param);
	stringstream ss;

	ss << m << std::endl;
	NotifyAll(ss.str());
}

//========================================================================
void CSirServer::OnSCETime(SCEvent& param)
{
	SCETime& t = static_cast<SCETime&>(param);
	stringstream ss;

	ss << t << std::endl;
	NotifyAll(ss.str());
}

//========================================================================
void CSirServer::OnSCETZInfo(SCEvent& param)
{
	SCETimeZoneInfo& t = static_cast<SCETimeZoneInfo&>(param);
	stringstream ss;

	ss << t << std::endl;
	NotifyAll(ss.str());
}

//========================================================================
void CSirServer::OnSCESongInfo(SCEvent& param)
{
	SCESongInfo& s = static_cast<SCESongInfo&>(param);
	stringstream ss;

	ss << s << std::endl;
	NotifyAll(ss.str());
}

//========================================================================
void CSirServer::OnSCEChannel(SCEvent& param)
{
	SCEChannel& c = static_cast<SCEChannel&>(param);
	stringstream ss;

	ss << c << std::endl;
	NotifyAll(ss.str());
}

//========================================================================
void CSirServer::OnSCEChannelInfo(SCEvent& param)
{
	SCEChannelInfo& c = static_cast<SCEChannelInfo&>(param);
	stringstream ss;

	ss << c << std::endl;
	NotifyAll(ss.str());
}

//========================================================================
void CSirServer::OnSCEChannelMap(SCEvent& param)
{
	SCEChannelMap& m = static_cast<SCEChannelMap&>(param);
	stringstream ss;

	ss << m << std::endl;
	NotifyAll(ss.str());
}

//========================================================================
void CSirServer::OnSCEStatus(SCEvent& param)
{
	SCEStatus& s = static_cast<SCEStatus&>(param);
	stringstream ss;

	ss << s << std::endl;
	NotifyAll(ss.str());
}

//========================================================================
void CSirServer::OnSCERSSI(SCEvent& param)
{
	SCERSSI& r = static_cast<SCERSSI&>(param);
	stringstream ss;

	ss << r << std::endl;
	NotifyAll(ss.str());
}

//========================================================================
void CSirServer::OnSCESignal(SCEvent& param)
{
	SCESignal& s = static_cast<SCESignal&>(param);
	stringstream ss;

	ss << s << std::endl;
	NotifyAll(ss.str());
}

//========================================================================
void CSirServer::OnSCEPower(SCEvent& param)
{
	SCEPower& p = static_cast<SCEPower&>(param);
	stringstream ss;

	ss << p << std::endl;
	NotifyAll(ss.str());

	if (!m_initialized && (p.power > 0u))
	{
		LogWrite(LEVEL_DEBUG, "I have the power!");
		m_initialized = true;
	}
}

//========================================================================
void CSirServer::OnSCEReset(SCEvent& param)
{

	// THE RADIO HAS JUST BEEN RESET - RESTART THE INITIALIZATION SEQUENCE
	m_initialized = false;
	m_sircon.SetPower(0x03);
}

//========================================================================
void CSirServer::OnSCEShutdown(SCEvent& param)
{
	SCEShutdown& s = static_cast<SCEShutdown&>(param);
	stringstream ss;

	ss << s << std::endl;
	NotifyAll(ss.str());

	Shutdown();
}

//========================================================================
void CSirServer::Update(SCEvent& e)
{

	// INVOKE THE CORRESPONDING HANDLER
	map<std::type_index, EVTHANDLER>::iterator it = m_evt_handlers.find(typeid(e));
	if (it != m_evt_handlers.end())
	{
		(this->*(it->second))(e);
	}
	else
	{
		LogWrite(LEVEL_DEBUG, "Unhandled Sirius event type %s", typeid(e).name());
	}
}

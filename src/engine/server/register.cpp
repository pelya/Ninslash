

#include <base/system.h>
#include <engine/shared/network.h>
#include <engine/shared/config.h>
#include <engine/console.h>
#include <engine/masterserver.h>

#include <mastersrv/mastersrv.h>

#include "register.h"

CRegister::CRegister()
{
	m_pNetServer = 0;
	m_pMasterServer = 0;
	m_pConsole = 0;

	m_RegisterState = REGISTERSTATE_START;
	m_RegisterStateStart = 0;
	m_RegisterFirst = 1;
	m_RegisterCount = 0;

	mem_zero(m_aMasterserverInfo, sizeof(m_aMasterserverInfo));
	m_UPNPThread = NULL;
}

void CRegister::RegisterNewState(int State)
{
	m_RegisterState = State;
	m_RegisterStateStart = time_get();
}

void CRegister::RegisterSendFwcheckresponse(NETADDR *pAddr)
{
	CNetChunk Packet;
	Packet.m_ClientID = -1;
	Packet.m_Address = *pAddr;
	Packet.m_Flags = NETSENDFLAG_CONNLESS;
	Packet.m_DataSize = sizeof(SERVERBROWSE_FWRESPONSE);
	Packet.m_pData = SERVERBROWSE_FWRESPONSE;
	m_pNetServer->Send(&Packet);
}

void CRegister::RegisterSendHeartbeat(NETADDR Addr)
{
	static unsigned char aData[sizeof(SERVERBROWSE_HEARTBEAT) + 2];
	unsigned short Port = g_Config.m_SvPort;
	CNetChunk Packet;

	mem_copy(aData, SERVERBROWSE_HEARTBEAT, sizeof(SERVERBROWSE_HEARTBEAT));

	Packet.m_ClientID = -1;
	Packet.m_Address = Addr;
	Packet.m_Flags = NETSENDFLAG_CONNLESS;
	Packet.m_DataSize = sizeof(SERVERBROWSE_HEARTBEAT) + 2;
	Packet.m_pData = &aData;

	// supply the set port that the master can use if it has problems
	if(g_Config.m_SvExternalPort)
		Port = g_Config.m_SvExternalPort;
	aData[sizeof(SERVERBROWSE_HEARTBEAT)] = Port >> 8;
	aData[sizeof(SERVERBROWSE_HEARTBEAT)+1] = Port&0xff;
	m_pNetServer->Send(&Packet);
}

void CRegister::RegisterSendCountRequest(NETADDR Addr)
{
	CNetChunk Packet;
	Packet.m_ClientID = -1;
	Packet.m_Address = Addr;
	Packet.m_Flags = NETSENDFLAG_CONNLESS;
	Packet.m_DataSize = sizeof(SERVERBROWSE_GETCOUNT);
	Packet.m_pData = SERVERBROWSE_GETCOUNT;
	m_pNetServer->Send(&Packet);
}

void CRegister::RegisterGotCount(CNetChunk *pChunk)
{
	unsigned char *pData = (unsigned char *)pChunk->m_pData;
	int Count = (pData[sizeof(SERVERBROWSE_COUNT)]<<8) | pData[sizeof(SERVERBROWSE_COUNT)+1];

	for(int i = 0; i < IMasterServer::MAX_MASTERSERVERS; i++)
	{
		if(net_addr_comp(&m_aMasterserverInfo[i].m_Addr, &pChunk->m_Address) == 0)
		{
			m_aMasterserverInfo[i].m_Count = Count;
			break;
		}
	}
}

void CRegister::Init(CNetServer *pNetServer, IEngineMasterServer *pMasterServer, IConsole *pConsole)
{
	m_pNetServer = pNetServer;
	m_pMasterServer = pMasterServer;
	m_pConsole = pConsole;
}

void CRegister::RegisterUpdate(int Nettype)
{
	int64 Now = time_get();
	int64 Freq = time_freq();

	if(!g_Config.m_SvRegister)
		return;

	m_pMasterServer->Update();

	if(m_RegisterState == REGISTERSTATE_START)
	{
		m_RegisterCount = 0;
		m_RegisterFirst = 1;
		RegisterNewState(REGISTERSTATE_UPDATE_ADDRS);
		m_pMasterServer->RefreshAddresses(Nettype);
		m_pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "register", "refreshing ip addresses");
		if (m_UPNPThread == NULL)
		{
			m_UPNPThread = thread_init(&RegisterUPNPThread, NULL);
		}
	}
	else if(m_RegisterState == REGISTERSTATE_UPDATE_ADDRS)
	{
		if(!m_pMasterServer->IsRefreshing())
		{
			int i;
			for(i = 0; i < IMasterServer::MAX_MASTERSERVERS; i++)
			{
				if(!m_pMasterServer->IsValid(i))
				{
					m_aMasterserverInfo[i].m_Valid = 0;
					m_aMasterserverInfo[i].m_Count = 0;
					continue;
				}

				NETADDR Addr = m_pMasterServer->GetAddr(i);
				m_aMasterserverInfo[i].m_Addr = Addr;
				m_aMasterserverInfo[i].m_Valid = 1;
				m_aMasterserverInfo[i].m_Count = -1;
				m_aMasterserverInfo[i].m_LastSend = 0;
			}

			m_pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "register", "fetching server counts");
			RegisterNewState(REGISTERSTATE_QUERY_COUNT);
		}
	}
	else if(m_RegisterState == REGISTERSTATE_QUERY_COUNT)
	{
		int Left = 0;
		for(int i = 0; i < IMasterServer::MAX_MASTERSERVERS; i++)
		{
			if(!m_aMasterserverInfo[i].m_Valid)
				continue;

			if(m_aMasterserverInfo[i].m_Count == -1)
			{
				Left++;
				if(m_aMasterserverInfo[i].m_LastSend+Freq < Now)
				{
					m_aMasterserverInfo[i].m_LastSend = Now;
					RegisterSendCountRequest(m_aMasterserverInfo[i].m_Addr);
				}
			}
		}

		// check if we are done or timed out
		if(Left == 0 || Now > m_RegisterStateStart+Freq*3)
		{
			// choose server
			int i;
			bool RegisteredCount = 0;
			for(i = 0; i < IMasterServer::MAX_MASTERSERVERS; i++)
			{
				if(!m_aMasterserverInfo[i].m_Valid || m_aMasterserverInfo[i].m_Count == -1)
					continue;

				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "Sending heartbeats to '%s'", m_pMasterServer->GetName(i));
				m_pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "register", aBuf);
				m_aMasterserverInfo[i].m_LastSend = 0;
				RegisterNewState(REGISTERSTATE_HEARTBEAT);
				RegisteredCount++;
			}

			if(RegisteredCount == 0)
			{
				m_pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "register", "WARNING: No master servers. Retrying in 60 seconds");
				RegisterNewState(REGISTERSTATE_ERROR);
			}
		}
	}
	else if(m_RegisterState == REGISTERSTATE_HEARTBEAT)
	{
		int i;
		// check if we should send heartbeat
		for(i = 0; i < IMasterServer::MAX_MASTERSERVERS; i++)
		{
			if(!m_aMasterserverInfo[i].m_Valid || m_aMasterserverInfo[i].m_Count == -1)
				continue;

			if(Now > m_aMasterserverInfo[i].m_LastSend+Freq*15)
			{
				m_aMasterserverInfo[i].m_LastSend = Now;
				RegisterSendHeartbeat(m_aMasterserverInfo[i].m_Addr);
			}

			if(Now > m_RegisterStateStart+Freq*60)
			{
				m_pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "register", "WARNING: Master server is not responding, switching master");
				RegisterNewState(REGISTERSTATE_START);
			}
		}
	}
	else if(m_RegisterState == REGISTERSTATE_REGISTERED)
	{
		if(m_RegisterFirst)
			m_pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "register", "server registered");

		m_RegisterFirst = 0;

		// check if we should send new heartbeat again
		if(Now > m_RegisterStateStart+Freq)
		{
			if(m_RegisterCount == 120) // redo the whole process after 60 minutes to balance out the master servers
				RegisterNewState(REGISTERSTATE_START);
			else
			{
				m_RegisterCount++;
				RegisterNewState(REGISTERSTATE_HEARTBEAT);
			}
		}
	}
	else if(m_RegisterState == REGISTERSTATE_ERROR)
	{
		// check for restart
		if(Now > m_RegisterStateStart+Freq*60)
			RegisterNewState(REGISTERSTATE_START);
	}
}

int CRegister::RegisterProcessPacket(CNetChunk *pPacket)
{
	// check for masterserver address
	bool Valid = false;
	NETADDR Addr1 = pPacket->m_Address;
	Addr1.port = 0;
	for(int i = 0; i < IMasterServer::MAX_MASTERSERVERS; i++)
	{
		NETADDR Addr2 = m_aMasterserverInfo[i].m_Addr;
		Addr2.port = 0;
		if(net_addr_comp(&Addr1, &Addr2) == 0)
		{
			Valid = true;
			break;
		}
	}
	if(!Valid)
		return 0;

	if(pPacket->m_DataSize == sizeof(SERVERBROWSE_FWCHECK) &&
		mem_comp(pPacket->m_pData, SERVERBROWSE_FWCHECK, sizeof(SERVERBROWSE_FWCHECK)) == 0)
	{
		RegisterSendFwcheckresponse(&pPacket->m_Address);
		return 1;
	}
	else if(pPacket->m_DataSize == sizeof(SERVERBROWSE_FWOK) &&
		mem_comp(pPacket->m_pData, SERVERBROWSE_FWOK, sizeof(SERVERBROWSE_FWOK)) == 0)
	{
		if(m_RegisterFirst)
			m_pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "register", "no firewall/nat problems detected");
		RegisterNewState(REGISTERSTATE_REGISTERED);
		return 1;
	}
	else if(pPacket->m_DataSize == sizeof(SERVERBROWSE_FWERROR) &&
		mem_comp(pPacket->m_pData, SERVERBROWSE_FWERROR, sizeof(SERVERBROWSE_FWERROR)) == 0)
	{
		m_pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "register", "ERROR: the master server reports that clients can not connect to this server.");
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "ERROR: configure your firewall/nat to let through udp on port %d.", g_Config.m_SvPort);
		m_pConsole->Print(IConsole::OUTPUT_LEVEL_STANDARD, "register", aBuf);
		RegisterNewState(REGISTERSTATE_ERROR);
		return 1;
	}
	else if(pPacket->m_DataSize == sizeof(SERVERBROWSE_COUNT)+2 &&
		mem_comp(pPacket->m_pData, SERVERBROWSE_COUNT, sizeof(SERVERBROWSE_COUNT)) == 0)
	{
		RegisterGotCount(pPacket);
		return 1;
	}

	return 0;
}

void CRegister::RegisterUPNPThread(void *)
{
	int64 Now = time_get();
	int64 Freq = time_freq();
	int64 RegisterUPNP = 0;

	if(Now > RegisterUPNP)
	{
		char cmd[1024];
		RegisterUPNP = Now + Freq * 200;
		unsigned short Port = g_Config.m_SvPort;
		if(g_Config.m_SvExternalPort)
			Port = g_Config.m_SvExternalPort;
		const char *Bindir = getenv("APPDIR");
		if(!Bindir)
			Bindir = ".";
		str_format(cmd, sizeof(cmd), "%s/upnpc -e Ninslash -a myself %d %d udp 300", Bindir, Port, Port);
		dbg_msg("Server", "UPNP register: %s\n", cmd);
		system(cmd);
	}
}

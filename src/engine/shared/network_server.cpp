/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/system.h>

#include <engine/console.h>

#include "config.h"
#include "netban.h"
#include "network.h"
#include <engine/external/md5/md5.h>
#include <engine/message.h>
#include <engine/shared/protocol.h>

bool CNetServer::Open(NETADDR BindAddr, CNetBan *pNetBan, int MaxClients, int MaxClientsPerIP, int Flags)
{
	// zero out the whole structure
	mem_zero(this, sizeof(*this));

	// open socket
	m_Socket = net_udp_create(BindAddr);
	if(!m_Socket.type)
		return false;

	m_pNetBan = pNetBan;

	// clamp clients
	m_MaxClients = MaxClients;
	if(m_MaxClients > NET_MAX_CLIENTS)
		m_MaxClients = NET_MAX_CLIENTS;
	if(m_MaxClients < 1)
		m_MaxClients = 1;

	m_MaxClientsPerIP = MaxClientsPerIP;

	if(secure_random_init() != 0)
	{
		dbg_msg("secure", "could not initialize secure RNG");
		return false;
	}

	secure_random_fill(m_SecurityTokenSeed, sizeof(m_SecurityTokenSeed));

	for(int i = 0; i < NET_MAX_CLIENTS; i++)
		m_aSlots[i].m_Connection.Init(m_Socket, true);

	return true;
}

int CNetServer::SetCallbacks(NETFUNC_NEWCLIENT pfnNewClient, NETFUNC_DELCLIENT pfnDelClient, void *pUser)
{
	m_pfnNewClient = pfnNewClient;
	m_pfnDelClient = pfnDelClient;
	m_UserPtr = pUser;
	return 0;
}

int CNetServer::Close()
{
	// TODO: implement me
	return 0;
}

int CNetServer::Drop(int ClientID, const char *pReason)
{
	// TODO: insert lots of checks here
	/*NETADDR Addr = ClientAddr(ClientID);

	dbg_msg("net_server", "client dropped. cid=%d ip=%d.%d.%d.%d reason=\"%s\"",
		ClientID,
		Addr.ip[0], Addr.ip[1], Addr.ip[2], Addr.ip[3],
		pReason
		);*/
	if(m_pfnDelClient)
		m_pfnDelClient(ClientID, pReason, m_UserPtr);

	m_aSlots[ClientID].m_Connection.Disconnect(pReason);

	return 0;
}

int CNetServer::Update()
{
	for(int i = 0; i < MaxClients(); i++)
	{
		m_aSlots[i].m_Connection.Update();
		if(m_aSlots[i].m_Connection.State() == NET_CONNSTATE_ERROR &&
			(!m_aSlots[i].m_Connection.m_TimeoutProtected ||
			 !m_aSlots[i].m_Connection.m_TimeoutSituation))
		{
			Drop(i, m_aSlots[i].m_Connection.ErrorString());
		}
	}

	return 0;
}

void CNetServer::SendMsgs(NETADDR &Addr, const CMsgPacker *Msgs[], int num) {
	CNetPacketConstruct m_Construct;
	mem_zero(&m_Construct, sizeof(m_Construct));
	unsigned char *pChunkData = &m_Construct.m_aChunkData[m_Construct.m_DataSize];

	for (int i = 0; i < num; i++) {
		const CMsgPacker *msg = Msgs[i];
		CNetChunkHeader Header;
		Header.m_Flags = 0;
		Header.m_Size = msg->Size();
		Header.m_Sequence = 0;
		pChunkData = Header.Pack(pChunkData);
		mem_copy(pChunkData, msg->Data(), msg->Size());
		*((unsigned char*)pChunkData) <<= 1;
		*((unsigned char*)pChunkData) |= 1;
		pChunkData += msg->Size();
		m_Construct.m_NumChunks++;
	}

	//
	m_Construct.m_DataSize = (int)(pChunkData-m_Construct.m_aChunkData);
	CNetBase::SendPacket(m_Socket, &Addr, &m_Construct, GetToken(Addr));
	CNetBase::SendPacket(m_Socket, &Addr, &m_Construct, GetPrevToken(Addr));
}

SECURITY_TOKEN CNetServer::GetToken(NETADDR &Addr) {
	long token_idx = time_get() / time_freq() / 10;
	return GetToken(Addr, token_idx);
}

SECURITY_TOKEN CNetServer::GetPrevToken(NETADDR &Addr) {
	long token_idx = time_get() / time_freq() / 10;
	return GetToken(Addr, token_idx - 1);
}

SECURITY_TOKEN CNetServer::GetToken(NETADDR &Addr, long timestamp) {
	md5_state_t md5;
	md5_byte_t digest[16];
	SECURITY_TOKEN securityToken;
	md5_init(&md5);
	md5_append(&md5, (unsigned char*)m_SecurityTokenSeed, sizeof(m_SecurityTokenSeed));
	md5_append(&md5, (unsigned char*)&Addr, sizeof(Addr));
	md5_append(&md5, (unsigned char*)&timestamp, sizeof(timestamp));
	md5_finish(&md5, digest);
	securityToken = *(SECURITY_TOKEN*)digest;
	if (securityToken < 0) {
		securityToken = -securityToken;
	}
	securityToken += 2;
	return securityToken;
}

bool CNetServer::CheckToken(NETADDR &Addr, SECURITY_TOKEN token) {
	return token == GetToken(Addr) || token == GetPrevToken(Addr);
}

/*
	TODO: chopp up this function into smaller working parts
*/
int CNetServer::Recv(CNetChunk *pChunk)
{
	while(1)
	{
		NETADDR Addr;

		// check for a chunk
		if(m_RecvUnpacker.FetchChunk(pChunk))
			return 1;

		// TODO: empty the recvinfo
		int Bytes = net_udp_recv(m_Socket, &Addr, m_RecvUnpacker.m_aBuffer, NET_MAX_PACKETSIZE);

		// no more packets for now
		if(Bytes <= 0)
			break;

		// check if we just should drop the packet
		char aBuf[128];
		if(NetBan() && NetBan()->IsBanned(&Addr, aBuf, sizeof(aBuf)))
		{
			// banned, reply with a message
			CNetBase::SendControlMsg(m_Socket, &Addr, 0, NET_CTRLMSG_CLOSE, aBuf, str_length(aBuf)+1, NET_SECURITY_TOKEN_UNSUPPORTED);
			continue;
		}

		bool Found = false;

		if(CNetBase::UnpackPacket(m_RecvUnpacker.m_aBuffer, Bytes, &m_RecvUnpacker.m_Data) == 0)
		{
			if(m_RecvUnpacker.m_Data.m_Flags&NET_PACKETFLAG_CONNLESS)
			{
				pChunk->m_Flags = NETSENDFLAG_CONNLESS;
				pChunk->m_ClientID = -1;
				pChunk->m_Address = Addr;
				pChunk->m_DataSize = m_RecvUnpacker.m_Data.m_DataSize;
				pChunk->m_pData = m_RecvUnpacker.m_Data.m_aChunkData;
				return 1;
			}
			else
			{
				// TODO: check size here
				if(m_RecvUnpacker.m_Data.m_Flags&NET_PACKETFLAG_CONTROL && m_RecvUnpacker.m_Data.m_aChunkData[0] == NET_CTRLMSG_CONNECT)
				{
					Found = false;

					// check if we already got this client
					for(int i = 0; i < MaxClients(); i++)
					{
						if(m_aSlots[i].m_Connection.State() != NET_CONNSTATE_OFFLINE &&
							m_aSlots[i].m_Connection.State() != NET_CONNSTATE_ERROR &&
							net_addr_comp(m_aSlots[i].m_Connection.PeerAddress(), &Addr) == 0)
						{
							Found = true; // silent ignore.. we got this client already
							//if(m_aSlots[i].m_Connection.State() == NET_CONNSTATE_ERROR)
							//{
							//	m_aSlots[i].m_Connection.Feed(&m_RecvUnpacker.m_Data, &Addr);
							//	if(m_pfnNewClient)
							//		m_pfnNewClient(i, m_UserPtr);
							//}
							break;
						}
					}

					// client that wants to connect
					if(!Found)
					{
						// only allow a specific number of players with the same ip
						NETADDR ThisAddr = Addr, OtherAddr;
						int FoundAddr = 1;
						ThisAddr.port = 0;
						for(int i = 0; i < MaxClients(); ++i)
						{
							if(m_aSlots[i].m_Connection.State() == NET_CONNSTATE_OFFLINE)
								continue;

							OtherAddr = *m_aSlots[i].m_Connection.PeerAddress();
							OtherAddr.port = 0;
							if(!net_addr_comp(&ThisAddr, &OtherAddr))
							{
								if(FoundAddr++ >= m_MaxClientsPerIP)
								{
									char aBuf[128];
									str_format(aBuf, sizeof(aBuf), "Only %d players with the same IP are allowed", m_MaxClientsPerIP);
									CNetBase::SendControlMsg(m_Socket, &Addr, 0, NET_CTRLMSG_CLOSE, aBuf, sizeof(aBuf), NET_SECURITY_TOKEN_UNSUPPORTED);
									return 0;
								}
							}
						}

						for(int i = 0; i < MaxClients(); i++)
						{
							if(m_aSlots[i].m_Connection.State() == NET_CONNSTATE_OFFLINE)
							{
								Found = true;
								SECURITY_TOKEN securityToken = GetToken(Addr);
								CNetBase::SendControlMsg(m_Socket, &Addr, 0, NET_CTRLMSG_CONNECTACCEPT, SECURITY_TOKEN_MAGIC, sizeof(SECURITY_TOKEN_MAGIC), securityToken);
								break;
							}
						}

						if(!Found)
						{
							const char FullMsg[] = "This server is full";
							CNetBase::SendControlMsg(m_Socket, &Addr, 0, NET_CTRLMSG_CLOSE, FullMsg, sizeof(FullMsg), NET_SECURITY_TOKEN_UNSUPPORTED);
						}
					}
				}
				else
				{
					// normal packet, find matching slot
					Found = false;
					for(int i = 0; i < MaxClients(); i++)
					{
						if(net_addr_comp(m_aSlots[i].m_Connection.PeerAddress(), &Addr) == 0)
						{
							if(m_aSlots[i].m_Connection.State() == NET_CONNSTATE_OFFLINE ||
								m_aSlots[i].m_Connection.State() == NET_CONNSTATE_ERROR)
								continue;
							if(m_aSlots[i].m_Connection.Feed(&m_RecvUnpacker.m_Data, &Addr))
							{
								if(m_RecvUnpacker.m_Data.m_DataSize)
									m_RecvUnpacker.Start(&Addr, &m_aSlots[i].m_Connection, i);
							}
							Found = true;
						}
					}

					if (!Found) {
						CNetChunkHeader h;
						unsigned char *pData = m_RecvUnpacker.m_Data.m_aChunkData;
						pData = h.Unpack(pData);
						CUnpacker Unpacker;
						Unpacker.Reset(pData, h.m_Size);
						int Msg = Unpacker.GetInt() >> 1;
						if (Msg == NETMSG_INPUT) {
							int ack = Unpacker.GetInt();
							if (CheckToken(Addr, ack)) {
								for(int i = 0; i < MaxClients(); i++)
								{
									if(m_aSlots[i].m_Connection.State() == NET_CONNSTATE_OFFLINE)
									{
										Found = true;
										CNetPacketConstruct *pPacket = &m_RecvUnpacker.m_Data;
										pPacket->m_DataSize -= sizeof(SECURITY_TOKEN);
										SECURITY_TOKEN securityToken = *(SECURITY_TOKEN*)&pPacket->m_aChunkData[pPacket->m_DataSize];
										if (!CheckToken(Addr, securityToken)) {
											securityToken = NET_SECURITY_TOKEN_UNSUPPORTED;
										}
										CNetPacketConstruct m_Construct;
										mem_zero(&m_Construct, sizeof(m_Construct));
										unsigned char *pChunkData;
										m_Construct.m_aChunkData[0] = NET_CTRLMSG_CONNECT;
										mem_copy(m_Construct.m_aChunkData + 1, SECURITY_TOKEN_MAGIC, sizeof(SECURITY_TOKEN_MAGIC));
										mem_copy(m_Construct.m_aChunkData + 1 + sizeof(SECURITY_TOKEN_MAGIC), &securityToken, sizeof(securityToken));
										m_Construct.m_DataSize = 1 + sizeof(SECURITY_TOKEN_MAGIC) + sizeof(securityToken);
										m_Construct.m_Flags = NET_PACKETFLAG_CONTROL;
										m_aSlots[i].m_Connection.Feed(&m_Construct, &Addr, securityToken);
										if(m_pfnNewClient)
											m_pfnNewClient(i, m_UserPtr);
										break;
									}
								}

								if(!Found)
								{
									const char FullMsg[] = "This server is full";
									CNetBase::SendControlMsg(m_Socket, &Addr, 0, NET_CTRLMSG_CLOSE, FullMsg, sizeof(FullMsg), NET_SECURITY_TOKEN_UNSUPPORTED);
								}
							} else {
								dbg_msg("BAD TOKEN", "");
							}
						} else {
							CMsgPacker Msg(NETMSG_SNAPEMPTY);
							SECURITY_TOKEN securityToken = GetToken(Addr);
							Msg.AddInt((int)securityToken);
							Msg.AddInt((int)securityToken + 1);
							const CMsgPacker *Msgs[] = {&Msg, &Msg, &Msg};
							SendMsgs(Addr, Msgs, 3);
						}
					}
				}
			}
		}
	}
	return 0;
}

int CNetServer::Send(CNetChunk *pChunk)
{
	if(pChunk->m_DataSize >= NET_MAX_PAYLOAD)
	{
		dbg_msg("netserver", "packet payload too big. %d. dropping packet", pChunk->m_DataSize);
		return -1;
	}

	if(pChunk->m_Flags&NETSENDFLAG_CONNLESS)
	{
		// send connectionless packet
		CNetBase::SendPacketConnless(m_Socket, &pChunk->m_Address, pChunk->m_pData, pChunk->m_DataSize);
	}
	else
	{
		int Flags = 0;
		dbg_assert(pChunk->m_ClientID >= 0, "errornous client id");
		dbg_assert(pChunk->m_ClientID < MaxClients(), "errornous client id");

		if(pChunk->m_Flags&NETSENDFLAG_VITAL)
			Flags = NET_CHUNKFLAG_VITAL;

		if(m_aSlots[pChunk->m_ClientID].m_Connection.QueueChunk(Flags, pChunk->m_DataSize, pChunk->m_pData) == 0)
		{
			if(pChunk->m_Flags&NETSENDFLAG_FLUSH)
				m_aSlots[pChunk->m_ClientID].m_Connection.Flush();
		}
		else
		{
			//Drop(pChunk->m_ClientID, "Error sending data");
		}
	}
	return 0;
}

void CNetServer::SetMaxClientsPerIP(int Max)
{
	// clamp
	if(Max < 1)
		Max = 1;
	else if(Max > NET_MAX_CLIENTS)
		Max = NET_MAX_CLIENTS;

	m_MaxClientsPerIP = Max;
}

bool CNetServer::SetTimedOut(int ClientID, int OrigID)
{
	if (m_aSlots[ClientID].m_Connection.State() != NET_CONNSTATE_ERROR)
		return false;

	m_aSlots[ClientID].m_Connection.SetTimedOut(ClientAddr(OrigID), m_aSlots[OrigID].m_Connection.SeqSequence(), m_aSlots[OrigID].m_Connection.AckSequence(), m_aSlots[OrigID].m_Connection.SecurityToken());
	m_aSlots[OrigID].m_Connection.Reset();
	return true;
}

void CNetServer::SetTimeoutProtected(int ClientID)
{
	m_aSlots[ClientID].m_Connection.m_TimeoutProtected = true;
}

int CNetServer::ResetErrorString(int ClientID)
{
	m_aSlots[ClientID].m_Connection.ResetErrorString();
	return 0;
}

const char *CNetServer::ErrorString(int ClientID)
{
	return m_aSlots[ClientID].m_Connection.ErrorString();
}

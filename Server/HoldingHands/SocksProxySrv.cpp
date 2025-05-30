#include "stdafx.h"
#include "SocksProxySrv.h"
#include "SocksProxyTcp.h"
#include "SocksProxyListener.h"
#include "utils.h"
#include "dbg.h"
#include <limits.h>
#include "dbg.h"
#include "Client.h"

CSocksProxySrv::CSocksProxySrv(CClient*pClient) :
CEventHandler(pClient, SOCKS_PROXY)
{
	m_listener = 0;
	m_lastID = 0;
	m_free = MAX_CLIENT_COUNT;
	m_spinlock = 0;
	m_ver = 0x5;
	m_debug_cnt = 0;
	memset(m_bitmap, -1, sizeof(m_bitmap));
	memset(m_clients, 0, sizeof(m_clients));
}


CSocksProxySrv::~CSocksProxySrv()
{
	//do nothing....
	dbg_log("CSocksProxySrv::~CSocksProxySrv()");
}

void CSocksProxySrv::OnOpen()
{
	SetSocksVersion(0x5);
}

void CSocksProxySrv::OnClose()
{
	//Stop accept.
	if (m_listener)
	{
		m_listener->Close();
		m_listener->Put();
		m_listener = NULL;
	}


#ifdef DEBUG
	int cnt = 0;
#endif

	//release sockets which is used by remote.
	for (int i = 0; i < MAX_CLIENT_COUNT; i++)
	{
		if (m_clients[i])
		{
#ifdef DEBUG
			cnt++;
#endif
			OnProxyClose(i);
		}
	}

	

	Notify(WM_SOCKS_PROXY_ERROR, (WPARAM)TEXT("Connection close...."));
	return;
}


void CSocksProxySrv::OnEvent(UINT32 e, BYTE *lpData, UINT32 Size)
{
	switch (e)
	{
	case SOCK_PROXY_REQUEST_RESULT:
		OnProxyResponse(lpData,Size);
		break;
	case SOCK_PROXY_DATA:
		OnProxyData(lpData, Size);
		break;
	case SOCK_PROXY_CLOSE:
		OnProxyClose(*(DWORD*)lpData);
		break;
	}
}

DWORD CSocksProxySrv::GetConnections()
{
	return MAX_CLIENT_COUNT - m_free;
}


void CSocksProxySrv::StopProxyServer()
{
	if (m_listener)
	{
		m_listener->Close();
		m_listener->Put();
		m_listener = NULL;
	}
}

BOOL CSocksProxySrv::StartProxyServer(UINT Port,const char* IP,const char* UDPAssociateAddr)
{
	BOOL bAssociated = FALSE;
	if (m_listener)
	{
		m_listener->Close();
		m_listener->Put();
		m_listener = NULL;
	}

	m_listener = new CSocksProxyListener(this);
	m_UDPAssociateAddr = inet_addr(UDPAssociateAddr);

	if (!m_listener->Create())
	{
		dbg_log("m_listener->Create() failed");
		goto __failed__;
	}

	if (!m_listener->Bind(Port, IP))
	{
		dbg_log("m_listener->Bind(Port, IP) failed");
		goto __failed__;
	}

	if (!m_listener->Listen(1024))
	{
		dbg_log("m_listener->Bind(Port, IP) failed");
		goto __failed__;
	}

	_read_lock(&m_pClient->m_rw_spinlock);
	
	if (m_pClient->m_flag & FLAG_ASSOCIATED)
	{
		if (m_pClient->m_Iocp->AssociateSock(m_listener))
		{
			bAssociated = TRUE;
		}
	}
	_read_unlock(&m_pClient->m_rw_spinlock);

	if (bAssociated && m_listener->Accept(NULL))
	{
		return TRUE;
	}

__failed__:
	m_listener->Close();
	m_listener->Put();
	m_listener = NULL;
	return FALSE;
}

void CSocksProxySrv::SetSocksVersion(BYTE Version)
{
	if (Version != 0x4 && Version != 0x5)
		return;
	
	m_ver = Version;
}
BYTE CSocksProxySrv::GetSocksVersion()
{
	return m_ver;
}


void CSocksProxySrv::OnProxyClose(DWORD ID)
{
	CSocksProxyTcp * client;

	assert(ID >= 0 && ID < MAX_CLIENT_COUNT);
	assert(m_clients[ID]);

	client = m_clients[ID];
	m_clients[ID] = NULL;;

	client->Close();
	client->Put();
}

void CSocksProxySrv::OnProxyData(BYTE * lpData, DWORD Size)
{
	UINT32 ID = *(UINT32*)lpData;
	lpData += sizeof(ID);
	Size   -= sizeof(ID);
	
	assert(ID >= 0 && ID < MAX_CLIENT_COUNT && m_clients[ID]);
	m_clients[ID]->OnProxyData(lpData, Size);
}

void CSocksProxySrv::OnProxyResponse(BYTE * lpData, DWORD Size){
	UINT32 ID = *(UINT32*)lpData;
	lpData += sizeof(ID);
	Size   -= sizeof(ID);
	
	assert(ID >= 0 && ID < MAX_CLIENT_COUNT && m_clients[ID]);
	m_clients[ID]->OnProxyResponse(lpData, Size);
}



void CSocksProxySrv::FreeClientID(UINT ClientID)
{
	assert(!m_clients[ClientID]);

	__spin_lock(m_spinlock);
	m_bitmap[ClientID / 32] |= (1 << (ClientID % 32));
	__spin_unlock(m_spinlock);

	InterlockedIncrement(&m_free);
}

//alloc client id by bitmap.
int CSocksProxySrv::AllocClientID(CSocksProxyTcp * client)
{
	DWORD Used = 0;
	int ID = -1;

	if (m_free <= 0){
		return -1;
	}

	__spin_lock(m_spinlock);
	while (Used < MAX_CLIENT_COUNT)
	{
		int id = 0;
		if (!__popcnt(m_bitmap[m_lastID]))
		{
			m_lastID = (m_lastID + 1) % (MAX_CLIENT_COUNT / 32);
			Used += 32;
			continue;
		}
		id = _tzcnt_u32(m_bitmap[m_lastID]);
		m_bitmap[m_lastID] &= ~(1 << id);

		ID = m_lastID * 32 + id;
		break;
	}
	__spin_unlock(m_spinlock);
	
	if (ID >= 0)
	{
		InterlockedDecrement(&m_free);
		client->Get();						//远程客户对client的引用.
		m_clients[ID] = client;
	}
	else
	{
		dbg_log("Alloc ClientID Failed");
	}
	return ID;
}
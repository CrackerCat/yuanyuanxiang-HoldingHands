#pragma once
#include "TCPSocket.h"
#include <stdint.h>

#include <client_common.h>

class CClient :	public CTCPSocket
{
private:
	friend class CEventHandler;
	CEventHandler * m_pHandler;
	
	UINT     m_State;
	
	//recv Buffer;
	pkt_head * m_pkt;
	BYTE *     m_r_ptr;
	BYTE *     m_w_ptr;
	BYTE *     m_end_ptr;
	BYTE *     m_lpReadBuf;
	UINT32	   m_ReadBufSize;

	BYTE *   m_lpWriteBuf;
	UINT     m_WriteBufSize;
	HANDLE   m_hEvent;
	char    m_Server[64];
	UINT	m_Port;

public:
	virtual void OnRecv(BYTE * lpData, UINT32 nTransferredBytes, void * lpParam, DWORD Error);
	virtual void OnSend(BYTE * lpData, UINT32 nTransferredBytes, void * lpParam, DWORD Error);

	void Run();
	void Send(BYTE *lpData, UINT32 Size);
	void Send(vec * Bufs, int nBuf);

	void OnRecvCompletePacket(BYTE * lpData, UINT32 Size);
	void OnConnect(const SOCKADDR *Addr, int iAddrLen, void * lpParam, DWORD Error);
	void SetCallbackHandler(CEventHandler* h){ m_pHandler = h; };

	CClient();
	virtual ~CClient();
};

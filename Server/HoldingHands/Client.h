#pragma once
#include "TCPSocket.h"
#include <client_common.h>

//you should setNotifyWindow wen you get WM_CLIENT_LOGIN,
#define WM_CLIENT_LOGIN (WM_USER + 102)


class CClient :
	public CTCPSocket
{

private:
	friend class CEventHandler;

	HWND     m_hNotifyWindow;
	CEventHandler * m_pHandler;
	
	UINT     m_State;

	//recv Buffer;
	pkt_head * m_pkt;
	
	BYTE *     m_r_ptr;
	BYTE *     m_w_ptr;
	BYTE *     m_end_ptr;

	BYTE *     m_lpReadBuf;
	UINT32	   m_ReadBufSize;

	//
	BYTE *   m_lpWriteBuf;
	UINT     m_WriteBufSize;
	HANDLE   m_hEvent;
public:
	
	virtual void OnRecv(BYTE * lpData, UINT32 nTransferredBytes, void * lpParam, DWORD Error);
	virtual void OnSend(BYTE * lpData, UINT32 nTransferredBytes, void * lpParam, DWORD Error);

	void SetCallbackHandler(CEventHandler* h)
	{
		m_pHandler = h;
	}

	void Run();
	void Send(BYTE *lpData, UINT32 Size);
	void Send(vec * Bufs, int nBuf);

	void OnRecvCompletePacket(BYTE * lpData, UINT32 Size);

	LRESULT Notify(DWORD Msg, WPARAM wParam = 0, LPARAM lParam = 0, BOOL Sync = TRUE);
	void SetNotifyWindow(HWND hWnd){ m_hNotifyWindow = hWnd; };


	CClient(SOCKET hSock);
	virtual ~CClient();
};


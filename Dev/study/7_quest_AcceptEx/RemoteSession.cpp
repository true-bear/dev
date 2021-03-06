#include "RemoteSession.h"
#include "Packet.h"
#pragma comment(lib,"mswsock.lib")

RemoteSession::RemoteSession()
{
	ZeroMemory(&mRecvOverEx, sizeof(CustomOverEx));
	ZeroMemory(&mSendOverEx, sizeof(CustomOverEx));
	ZeroMemory(&mAcceptOverEx, sizeof(CustomOverEx));
	mRemoteSock = INVALID_SOCKET;
}

bool RemoteSession::AcceptReady(SOCKET listenSock)
{
	mRemoteSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP,
		NULL, 0, WSA_FLAG_OVERLAPPED);
	if (mRemoteSock == INVALID_SOCKET)
	{
		std::cout << "[ERR] AccpetAsync WSASocket fail" << std::endl;
	}

	ZeroMemory(&mAcceptOverEx, sizeof(CustomOverEx));

	DWORD bytes = 0;
	DWORD flags = 0;
	mAcceptOverEx.mWSABuf.len = 0;
	mAcceptOverEx.mWSABuf.buf = nullptr;
	mAcceptOverEx.mIoType = IOOperation::ACCEPT;
	mAcceptOverEx.mUid = mUID;

	auto ret = AcceptEx(
		listenSock,
		mRemoteSock,
		mAcceptBuf,
		0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&bytes,
		(LPWSAOVERLAPPED) & (mAcceptOverEx)
	);

	if (!ret)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			return false;
			printf_s("AcceptEx Error : %d\n", GetLastError());
		}
	}
	return true;

}
bool RemoteSession::AcceptFinish(HANDLE mIocp)
{
	mIsLive = true;

	SOCKADDR_IN		RemoteAddr;
	int nAddrLen = sizeof(SOCKADDR_IN);
	char clientIP[32] = { 0, };
	inet_ntop(AF_INET, &(RemoteAddr.sin_addr), clientIP, 32 - 1);

	std::cout << "[접속] index :"<<mUID<<"   socket num:"<<mRemoteSock << std::endl;

	RecvMsg();
	return true;
}
bool RemoteSession::SendReady(const unsigned int size, char* msg)
{
	std::lock_guard<std::mutex> guard(mSendLock);

	if ((mSendBuffPos + size) > MAX_SOCKBUF)
	{
		mSendBuffPos = 0;
	}

	auto pSendBuf = &mSendReservedBuf[mSendBuffPos];
	CopyMemory(pSendBuf, msg, size);

	mSendBuffPos += size;

	return true;
}

bool RemoteSession::SendPacket()
{
	/// 이전 send가 완료되어야 패킷을 보낸다.
	if (mSendPendingCnt > 0 || mSendBuffPos <= 0)
	{
		return false;
	}

	std::lock_guard<std::mutex> guard(mSendLock);

	++mSendPendingCnt;

	CopyMemory(mSendBuf, &mSendReservedBuf[0], mSendBuffPos);

	//Overlapped I/O을 위해 각 정보를 셋팅해 준다.
	mSendOverEx.mWSABuf.len = mSendBuffPos;
	mSendOverEx.mWSABuf.buf = &mSendBuf[0];
	mSendOverEx.mIoType = IOOperation::SEND;

	DWORD dwRecvNumBytes = 0;
	int nRet = WSASend(mRemoteSock,
		&(mSendOverEx.mWSABuf),
		1,
		&dwRecvNumBytes,
		0,
		(LPWSAOVERLAPPED) & (mSendOverEx),
		NULL);

	//socket_error이면 client socket이 끊어진걸로 처리한다.
	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		printf("[에러] WSASend()함수 실패 : %d\n", WSAGetLastError());
		return false;
	}
	

	if (mSendPendingCnt == 1)
	{
		mSendBuffPos = 0;
		return true;
	}
	else 
		return false;
}
void RemoteSession::SendFinish(unsigned long len)
{
	// IO 완료 됐을 때 호출 된다.
	std::lock_guard<std::mutex> guard(mSendLock);
	--mSendPendingCnt;

	std::cout << "Send Complete byte:" << len << std::endl;
}

bool RemoteSession::RecvMsg()
{
	DWORD dwFlag = 0;
	DWORD dwRecvNumBytes = 0;

	//Overlapped I/O을 위해 각 정보를 셋팅해 준다.
	mRecvOverEx.mWSABuf.len = MAX_SOCKBUF;
	mRecvOverEx.mWSABuf.buf = mRecvOverEx.mBuf.data();
	mRecvOverEx.mIoType = IOOperation::RECV;

	int nRet = WSARecv(mRemoteSock,
		&(mRecvOverEx.mWSABuf),
		1,
		&dwRecvNumBytes,
		&dwFlag,
		(LPWSAOVERLAPPED) & (mRecvOverEx),
		NULL);

	//socket_error이면 client socket이 끊어진걸로 처리한다.
	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		printf("[에러] WSARecv()함수 실패 : %d\n", WSAGetLastError());
		return false;
	}

	return true;
}
bool RemoteSession::RecvFinish(const char* pNext, const unsigned long remain)
{
	return true;
}

bool RemoteSession::UnInit(bool IsForce, SOCKET mListenSock)
{
	struct linger stLinger = { 0, 0 };	// SO_DONTLINGER로 설정

	// 강제 종료
	if (IsForce)
	{
		stLinger.l_onoff = 1;
	}
	std::cout << "[Session] Release-> " << mUID <<"   SOCKET:"<<mRemoteSock<< std::endl;


	shutdown(mRemoteSock, SD_BOTH);
	setsockopt(mRemoteSock, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));
	closesocket(mRemoteSock);

	mRemoteSock = INVALID_SOCKET;
	mIsLive = false;

	AcceptReady(mListenSock);

	return true;
}




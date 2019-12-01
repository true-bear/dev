#include "RemoteSession.h"
#include "Packet.h"

RemoteSession::RemoteSession()
{
	ZeroMemory(&mRecvOverEx, sizeof(CustomOverEx));
	ZeroMemory(&mSendOverEx, sizeof(CustomOverEx));
	mRemoteSock = INVALID_SOCKET;
}


//bool RemoteSession::SendPushInLogic(unsigned int size, char* pMsg)
//{
//	// ������ ���������� ������ ���ؾ� �ȴ�. 
//	auto sendOver = new CustomOverEx;
//	ZeroMemory(sendOver, sizeof(CustomOverEx));
//	sendOver->m_wsaBuf.len = size;
//	sendOver->m_wsaBuf.buf = new char[size];
//	CopyMemory(sendOver->m_wsaBuf.buf, pMsg, size);
//	sendOver->m_eOperation = IOOperation::SEND;
//
//	std::lock_guard<std::mutex> guard(mSendLock);
//
//	mSendQ.push(sendOver);
//
//	if (mSendQ.size() == 1)
//	{
//		SendMsg();
//	}
//
//	return true;
//}
//
//void RemoteSession::SendPop()
//{
//	std::lock_guard<std::mutex> guard(mSendLock);
//
//	delete[] mSendQ.front()->m_wsaBuf.buf;
//	delete mSendQ.front();
//
//	mSendQ.pop();
//
//	if (!mSendQ.empty())
//	{
//		SendMsg();
//	}
//}
//
//void RemoteSession::SendMsg()
//{
//
//	auto sendOver = mSendQ.front();
//	DWORD byte = 0;
//
//	int nRet = WSASend(mRemoteSock,
//		&(sendOver->m_wsaBuf),
//		1,
//		&byte,
//		0,
//		(LPWSAOVERLAPPED)sendOver,
//		NULL);
//	
//	
//	printf("[Send] Byte: %d \n", byte);
//
//	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
//	{
//		std::cout << " Err WSASend() code: " << WSAGetLastError() << std::endl;
//	}
//}

bool RemoteSession::SendReady()
{
	return true;
}
bool RemoteSession::SendPacket(char* pBuf , int len)
{
	std::lock_guard<std::mutex> guard(mSendLock);

	/// ���� send�� �Ϸ�Ǿ�� �� �Լ����� ��Ŷ�� ������.
	if (mSendPendingCnt > 0)
		return false;

	// WSASend
	CustomOverEx* sendOver = new CustomOverEx;
	sendOver->m_wsaBuf.len = len;
	sendOver->m_wsaBuf.buf = pBuf;
	sendOver->m_eOperation = IOOperation::SEND;


	DWORD sendbytes = 0;
	DWORD flags = 0;
	auto ret = WSASend(
		mRemoteSock, 
		&sendOver->m_wsaBuf,
		1, 
		&sendbytes, 
		flags, 
		(LPWSAOVERLAPPED)sendOver,
		NULL);

	if (ret == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			// ���� �����ش�.
			delete sendOver;
			return true;
		}
	}

	if (sendbytes < len)
	{
		mSendOverEx.mRemainByte += len - sendbytes;
		return false;
	}
	
	++mSendPendingCnt;

	if (mSendPendingCnt == 1) 
		return true;
	else 
		return false;
}

void RemoteSession::SendFinish(unsigned long len)
{
	// IO �Ϸ� ���� �� ȣ�� �ȴ�.
	std::lock_guard<std::mutex> guard(mSendLock);
	--mSendPendingCnt;

	//std::cout << "Send Complete byte:" << len << std::endl;
}

bool RemoteSession::RecvMsg()
{
	DWORD dwFlag = 0;
	DWORD dwRecvNumBytes = 0;

	//Overlapped I/O�� ���� �� ������ ������ �ش�.
	mRecvOverEx.m_wsaBuf.len = MAX_SOCKBUF;
	mRecvOverEx.m_wsaBuf.buf = mRecvOverEx.mBuf.data();
	mRecvOverEx.m_eOperation = IOOperation::RECV;

	int nRet = WSARecv(mRemoteSock,
		&(mRecvOverEx.m_wsaBuf),
		1,
		&dwRecvNumBytes,
		&dwFlag,
		(LPWSAOVERLAPPED) & (mRecvOverEx),
		NULL);

	//socket_error�̸� client socket�� �������ɷ� ó���Ѵ�.
	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		printf("[����] WSARecv()�Լ� ���� : %d\n", WSAGetLastError());
		return false;
	}

	return true;
}

bool RemoteSession::RecvReady()
{
	// zero byte recv
	return true;
}

bool RemoteSession::RecvFinish(const char* pNext, const unsigned long remain)
{
//	mRecvOverEx.m_eOperation = IOOperation::RECV;
//	mRecvOverEx.mRemainByte = remain;
//
//	auto diff = (int)(remain - (mRecvOverEx.mCurrMark - pNext));
//
//	mRecvOverEx.m_wsaBuf.len = MAX_SOCKBUF;
//	mRecvOverEx.m_wsaBuf.buf = m_ConnectionInfo.RingRecvBuffer.ForwardMark(moveMark, m_RecvBufSize, remainByte);
//
//
	return true;
}




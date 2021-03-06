#pragma once

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <mswsock.h>
#include <iostream>
#include <queue>
#include <mutex>
#include <array>
#pragma comment(lib,"mswsock.lib")

const int MAX_SOCKBUF = 1024;	
const int MAX_WORKERTHREAD = 4; 

enum class IOOperation 
{
	RECV ,
	SEND,
	ACCEPT
};

//WSAOVERLAPPED구조체를 확장
struct CustomOverEx
{
	WSAOVERLAPPED mWSAOverlapped;		//Overlapped I/O구조체
	WSABUF		  mWSABuf;				//Overlapped I/O작업 버퍼
	unsigned int  mUid = -1;			// unique id
	unsigned int  mRemainByte = 0;

	std::array<char, MAX_SOCKBUF> mBuf;
	int         mSendPos = 0;

	IOOperation mIoType;			//작업 동작 종류

	CustomOverEx()
	{
		memset(&mWSAOverlapped, 0, sizeof(OVERLAPPED));
		memset(&mWSABuf, 0, sizeof(WSABUF));
		mBuf.fill(0);
	}
	void Init()
	{
		mBuf.fill(0);
	}
};

// 원격 세션
class RemoteSession
{
public:

	RemoteSession();
	void UnInit(bool IsForce, SOCKET listenSock);

	bool AcceptReady(SOCKET listenSock);
	bool AcceptFinish(HANDLE mhIocp);

	bool SendReady();
	bool SendPacket(char* pBuf , int len);
	void SendFinish(unsigned long len);

	bool RecvMsg();
	bool RecvReady();
	bool RecvFinish(const char* pNextBuf, const unsigned long remain);

	void SetUniqueId(int& id) { mUID = id; }
	
	unsigned int GetUniqueId() const { return mUID; }
	int GetSendBuffPos() { return mSendBuffPos; }
	SOCKET& GetSock() { return mRemoteSock; }

	bool IsLive() const { return mIsLive; }

	void IncreaseBuffPos(int len) { mSendBuffPos += len; }

	SOCKET						mRemoteSock;	
	CustomOverEx				mRecvOverEx;	
	CustomOverEx				mSendOverEx;	
	CustomOverEx				mAcceptOverEx;
private:
	std::mutex					mSendLock;
	bool						mIsLive = false;
	int                         mSendBuffPos = 0;      // 샌드 버퍼 위치
	int							mSendPendingCnt = 0;  // 다 보내지 못한 경우 체크
	unsigned int mUID = -1;
};
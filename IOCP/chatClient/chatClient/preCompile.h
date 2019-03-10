#pragma once

//이곳에서 통합관리


#pragma warning( disable:4786 )
#pragma warning( disable:4251 )
#pragma warning( disable:4311 )
#pragma warning( disable:4244 )
#pragma warning( disable:4996 )

const int SERVER_PORT = 9000;
const int MAX_WORKER_THREAD = 2;
const int MAX_BUFF_SIZE = 1024;
const int MAX_PACKET_SIZE = 255;



// Protocol ID
const int  CS_MSG = 1;
const int  SC_MSG = 1;



#pragma comment(lib,"Ws2_32.lib")


#include <iostream>
#include <WinSock2.h>
#include <winsock.h>
#include <thread>
#include <Windows.h>
#include <thread>

using namespace std;



// 이벤트

enum EVENTTYPE { E_RECV, E_SEND, E_BROAD_CAST };

// 작업이 끝날 때까지 유지되어야 하기 때문에 전역으로 둠. 로컬로 두면 뻗는다. 
struct WSAOVERLAPPED_EX
{
	WSAOVERLAPPED over;
	WSABUF wsabuf;
	char IOCP_buf[MAX_BUFF_SIZE];
	EVENTTYPE event_type;
};




//클라이언트 정보를 담기위한 구조체
struct sClientInfo {
	SOCKET				sSocketClient;			//Cliet와 연결되는 소켓
	WSAOVERLAPPED_EX	sRecvOverlappedEx;	//RECV Overlapped I/O작업을 위한 변수
	WSAOVERLAPPED_EX	sSendOverlappedEx;	//SEND Overlapped I/O작업을 위한 변수
	//생성자에서 멤버 변수들을 초기화
	sClientInfo()
	{
		ZeroMemory(&sRecvOverlappedEx, sizeof(WSAOVERLAPPED_EX));
		ZeroMemory(&sSendOverlappedEx, sizeof(WSAOVERLAPPED_EX));
		sSocketClient = INVALID_SOCKET;
	}
};


#pragma pack (push, 1)    // 구조체 사이즈 맞춤

struct cs_packet_msg
{
	BYTE size;
	BYTE type;
};

// 서버에서 클라 구조체 정의
struct sc_packet_msg
{
	BYTE size;
	BYTE type;
};

#pragma pack(pop)


void readThread();
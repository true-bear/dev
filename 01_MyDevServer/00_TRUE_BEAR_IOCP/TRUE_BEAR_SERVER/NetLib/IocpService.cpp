#include "IocpService.h"

IocpService::IocpService() 
{
	WSADATA w;
	WSAStartup(MAKEWORD(2, 2), &w);
}
IocpService::~IocpService() 
{
	WSACleanup();
}

void IocpService::StartIocpService()
{
	m_Iocp = std::make_unique<Iocp>();
	m_ListenSock = std::make_unique<CustomSocket>();
	
	
	// 리슨 소켓 생성 
	m_ListenSock->CreateSocket();

	// bind and listen
	m_ListenSock->BindAndListenSocket();

	// 메세지 풀 생성


	// 리슨 소켓을 iocp에 등록 
	m_Iocp->ResisterIocp(m_ListenSock->m_sock, nullptr);

	// 세션 풀 생성
	CreateSessionList();

	// 워커 스레드 생성 
	CreateWorkThread();
}

void IocpService::StopIocpService()
{
	if (m_Iocp->m_workIocp != INVALID_HANDLE_VALUE)
	{
		m_IsRunWorkThread = false;
		CloseHandle(m_Iocp->m_workIocp);

		for (int i = 0; i < m_WorkerThreads.size(); ++i)
		{
			if (m_WorkerThreads[i].get()->joinable())
			{
				m_WorkerThreads[i].get()->join();
			}
		}
	}

	if (m_ListenSock->m_sock != INVALID_SOCKET)
	{
		closesocket(m_ListenSock->m_sock);
		m_ListenSock->m_sock = INVALID_SOCKET;
	}

	WSACleanup();
}

bool IocpService::getNetworkMessage(INT8& msgType , INT32&  sessionIdx,char* pBuf , INT16& copySize )
{
	Message* pMsg = nullptr;
	Session* pSession = nullptr;
	DWORD ioSize = 0;


	if (!m_Iocp->GQCS(pMsg, pSession, ioSize))
	{
		return false;
	}

	switch (pMsg->Type)
	{
	case MsgType::Session:
		//DoPostConnection(pConnection, pMsg, msgOperationType, connectionIndex);
		break;
	case MsgType::Close:
		//TODO 재 사용에 딜레이를 주도록 한다. 이유는 재 사용으로 가능 도중 IOCP 워크 스레드에서 이 세션이 호출될 수도 있다. 
	//	DoPostClose(pConnection, pMsg, msgOperationType, connectionIndex);
		break;
	case MsgType::OnRecv:
		//DoPostRecvPacket(pConnection, pMsg, msgOperationType, connectionIndex, pBuf, copySize, ioSize);
	//	m_pMsgPool->DeallocMsg(pMsg);
		break;
	}


	return true;
}


bool IocpService::CreateSessionList()
{
	for (int i = 0; i < MAX_SESSION_COUNT; ++i)
	{
		auto pSession = new Session();
		pSession->Init(m_ListenSock->m_sock, i);
		pSession->DoAcceptOverlapped();
		
		// 왜 벡터로 하면 터질까? 
		//m_SessionList.push_back(pSession);
		m_SessionList.insert({ i, pSession });
	}

	return true;
}

void IocpService::DestorySessionList()
{
	for (int i = 0; i < MAX_SESSION_COUNT; ++i)
	{
		m_SessionList.erase(i);
	}
}

bool IocpService::CreateWorkThread()
{
	for (int i = 0; i < WORKER_THREAD_COUNT; ++i)
	{
		m_WorkerThreads.push_back(std::make_unique<std::thread>([&]() {WorkThread(); }));
	}

	return true;
}

void IocpService::WorkThread()
{
	while (m_IsRunWorkThread)
	{
		DWORD ioSize = 0;
		CustomOverlapped* pOver = nullptr;
		Session* pSession = nullptr;


		auto result = GetQueuedCompletionStatus(
			m_Iocp->m_workIocp,
			&ioSize,
			reinterpret_cast<PULONG_PTR>(&pSession),
			reinterpret_cast<LPOVERLAPPED*>(&pOver),
			INFINITE);


		if (pOver == nullptr)
		{
			if (WSAGetLastError() != 0 && WSAGetLastError() != WSA_IO_PENDING)
			{
				std::cout << "GQCS in Worker fail..." <<WSAGetLastError()<< std::endl;
			}
			continue;
		}

		if (!result || (0 == ioSize && OPType::Accept != pOver->type))
		{
			// 킥할 세션들 처리해주어야함
			//HandleExceptionWorkThread(pConnection, pOverlappedEx);
			continue;
		}

		switch (pOver->type)
		{
		case OPType::Accept:
			DoAcceptEx(pOver);
			break;
		case OPType::Recv:
			DoRecv(pOver, ioSize);
			break;
		case OPType::Send:
			//DoSend(pOverlappedEx, ioSize);
			break;
		}
	}
}

Session* IocpService::GetSession(const int sessionIdx)
{
	if (sessionIdx < 0 || sessionIdx >= MAX_SESSION_COUNT)
	{
		return nullptr;
	}
	auto iter = m_SessionList.find(sessionIdx);
	return iter->second;
}

void IocpService::DoAcceptEx(const CustomOverlapped* pOverlappedEx)
{
	auto pSession = GetSession(pOverlappedEx->SessionIdx);
	if (pSession == nullptr)
	{
		return;
	}

	pSession->DecrementAcceptIORefCount();


	// AcceptEx 마무리 작업
	if (pSession->SetNetAddressInfo() == false)
	{
		std::cout << "SetNetAddressInfo is failed..." << std::endl;
		if (pSession->CloseComplete())
		{
			//HandleExceptionCloseConnection(pConnection);
		}
		return;
	}

	// 해당 소켓을 등록
	if (!pSession->BindIOCP(m_Iocp->m_workIocp))
	{
		if (pSession->CloseComplete())
		{
			//HandleExceptionCloseConnection(pConnection);
		}
		return;
	}

	// 세션 상태 갱신
	pSession->UpdateSessionState();


	if (!pSession->PostRecv(pSession->RecvBufferBeginPos(), 0))
	{
		std::cout << "PostRecv Error" << std::endl;
		//HandleExceptionCloseConnection(pConnection);
	}

	// PCQS 도 던져준다.
	// 다른 세션들도 메세지 받게



	// 출력 확인용
	std::cout << "Session Join ["<<pSession->GetIndex()<<"]" << std::endl;
}

void IocpService::DoRecv(CustomOverlapped* pOver, const DWORD ioSize)
{
	Session* pSession = GetSession(pOver->SessionIdx);
	if (pSession == nullptr)
	{
		return;
	}

	pSession->DecrementRecvIORefCount();

	pOver->OverlappedExWsaBuf.buf = pOver->pOverlappedExSocketMessage;
	pOver->OverlappedExRemainByte += ioSize;

	auto remainByte = pOver->OverlappedExRemainByte;
	auto pNext = pOver->OverlappedExWsaBuf.buf;

	//PacketForwardingLoop(pConnection, remainByte, pNext);

	if (!pSession->PostRecv(pNext, remainByte))
	{
		if (pSession->CloseComplete())
		{
			//HandleExceptionCloseConnection(pConnection);
		}
	}


	// echo send  임시 코드 나중에 지워주자 . 지금은 동작 잘함 
	WSABUF b;
	b.buf = pOver->pOverlappedExSocketMessage;
	b.len = ioSize;
	int m_writeFlag = 0;
	DWORD sendByte = 0;

	auto ret = WSASend(
		pSession->GetClientSocket(),
		&b,
		1,
		&sendByte,
		m_writeFlag,
		NULL,
		NULL
	);
}
#include <thread>
#include <chrono>
#include "../02_NetworkLayer/SelectNetwork.h"


#include "PktProcessMain.h"
#include "ClientManager.h"

#include "LogicMain.h"


namespace LogicLayer
{
	LogicMain::LogicMain() {}
	LogicMain::~LogicMain() {}
	void LogicMain::Init()
	{
		m_pSelectNetwork = std::make_unique<NetworkLayer::SelectNetwork>();
		m_pSelectNetwork->InitNetwork();
	

		m_pClientManager = std::make_unique<ClientManager>();
		m_pClientManager->Init();


		m_pPacketProcess = std::make_unique<PktProcessMain>();
		m_pPacketProcess->Init(m_pSelectNetwork.get());


		m_IsRun = true;
	}
	void LogicMain::Run()
	{
		while (m_IsRun)
		{
			m_pSelectNetwork->Run();
			while (true)
			{
				auto packetInfo = m_pSelectNetwork->GetPacketInfo();

				if (packetInfo.PacketId == 0)
					break;
				else
					// ���� �ܿ��� ���� ��Ŷ�� ó���Ѵ�. 
					m_pPacketProcess->Process(packetInfo);
				
			}

			// �����ܿ��� ������ ���� üũ 
			m_pPacketProcess->StateCheck();
		}
	}

	void LogicMain::Stop()
	{
		m_IsRun = false;
	}


}
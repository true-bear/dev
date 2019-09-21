#include "Session.h"
#include "SessionManager.h"
#include "IocpService.h"

SessionManager* gSessionMgr = nullptr;

SessionManager::SessionManager() :mCurrentIssueCount(0), mCurrentReturnCount(0)
{
}
SessionManager::~SessionManager()
{
}

void SessionManager::CreateSessionPool()
{
	// �̰� �׽�Ʈ �ϸ鼭 �����غ� �� 
	m_MaxSessionCount = 1000;

	// ���� Ǯ �����
	for (int i = 0; i < m_MaxSessionCount; ++i)
	{
		Session* client = new Session();
		mFreeSessionList.push_back(client);

		// �̰� �����ؾ���. vector�� �ϸ� ������.....
		m_SessionList.push_back(client);
	}

	printf_s("[DEBUG][%s] m_MaxSessionCount: %d\n", __FUNCTION__, m_MaxSessionCount);
}



bool SessionManager::AcceptClientSessions()
{
	FastSpinlockGuard guard(mSpinLock);
	int i = 0;
	while (mCurrentIssueCount - mCurrentReturnCount < m_MaxSessionCount)
	{
		Session* newClient = mFreeSessionList.back();
		mFreeSessionList.pop_back();

		++mCurrentIssueCount;

		if (false == newClient->PostAccept()) 
		{
			return false;
		}

	}

	return true;
}



void SessionManager::ReturnSession(Session* session)
{
	FastSpinlockGuard guard(mSpinLock);

	session->SessionReset();
	mFreeSessionList.push_back(session);
	++mCurrentReturnCount;
}
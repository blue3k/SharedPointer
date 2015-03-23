////////////////////////////////////////////////////////////////////////////////
// 
// CopyRight (c) 2013 Blue3k
// 
// Author : KyungKun Ko
//
// Description : Test 1
//
////////////////////////////////////////////////////////////////////////////////


#include "Common\Typedefs.h"
#include "SharedPointer.h"
#include "SharedReferenceManager.h"
#include "Thread/SystemSynchronize.h"
#include "Thread/Thread.h"

#include "windows.h"
#include "stdio.h"
#include "tchar.h"
#include <unordered_map>

#include <chrono>
#include "SharedPointerTest.h"




#define MAX_TEST_ID      60
#define MAX_ACCESS_COUNT 10
#define MAX_GROUP_ONE    10
#define MAX_GROUP_TWO    10

const unsigned TaskWorkerInterval = 10;

// this thread group can create/delete pointer
class ThreadGroupOne : public BR::Thread
{
public:
	static BR::Mutex stm_StorageLock;
	static BR::SharedPointerT<SharedObjectType> stm_PointerStorage[MAX_TEST_ID];

	BR::SharedReferenceManager& m_referenceManager;

	ThreadGroupOne(BR::SharedReferenceManager& referenceManager)
		:m_referenceManager(referenceManager)
	{

	}

	virtual bool Run() override
	{
		while (1)
		{
			ULONG loopInterval = UpdateInterval(TaskWorkerInterval);

			if (CheckKillEvent(loopInterval))
			{
				// Kill Event signaled
				break;
			}



			SharedObjectType* sharedPtr = nullptr;
			BR::SharedPointerT<SharedObjectType> localShared;
			{
				BR::MutexScopeLock scopeLock(stm_StorageLock);
				long randID = rand() % MAX_TEST_ID;
				if (stm_PointerStorage[randID] == nullptr)
				{
					sharedPtr = new SharedObjectType(randID);
					m_referenceManager.RegisterSharedObject(sharedPtr);
					localShared = BR::SharedPointerT<SharedObjectType>(sharedPtr);
					stm_PointerStorage[randID] = localShared;
				}
				else
				{
					localShared = stm_PointerStorage[randID];
					auto usedCount = localShared->pItem->UsedCount.fetch_add(1,std::memory_order_relaxed)+1;
					if (usedCount > MAX_ACCESS_COUNT)
						stm_PointerStorage[randID] = BR::SharedPointerT<SharedObjectType>();
				}
			}

			// Pretend to use
			Sleep(0);
		}

		return true;
	}
};

BR::Mutex ThreadGroupOne::stm_StorageLock;
BR::SharedPointerT<SharedObjectType> ThreadGroupOne::stm_PointerStorage[MAX_TEST_ID];



// this thread group can query pointer
class ThreadGroupTwo : public BR::Thread
{
public:
	static BR::Mutex stm_StorageLock;
	static BR::WeakPointerT<SharedObjectType> stm_PointerStorage[MAX_TEST_ID];

	virtual bool Run() override
	{
		while (1)
		{
			ULONG loopInterval = UpdateInterval(TaskWorkerInterval);

			if (CheckKillEvent(loopInterval))
			{
				// Kill Event signaled
				break;
			}


			BR::WeakPointerT<SharedObjectType> localShared;
			{
				BR::MutexScopeLock scopeLock(stm_StorageLock);
				long randID = rand() % MAX_TEST_ID;
				if (stm_PointerStorage[randID] == nullptr)
				{
					BR::MutexScopeLock scopeLock(ThreadGroupOne::stm_StorageLock);
					if (ThreadGroupOne::stm_PointerStorage[randID] != nullptr)
					{
						localShared = ThreadGroupOne::stm_PointerStorage[randID];
						stm_PointerStorage[randID] = localShared;

					}
				}
				else
				{
					localShared = stm_PointerStorage[randID];
					BR::SharedPointerT<SharedObjectType> shared;
					localShared.GetSharedPointer(shared);
					if (shared != nullptr)
					{
						auto usedCount = shared->pItem->WeakUsedCount.fetch_add(1, std::memory_order_relaxed);
						if (usedCount > MAX_ACCESS_COUNT)
						{
							shared->WeakReleasedCount.fetch_add(1, std::memory_order_relaxed);
							stm_PointerStorage[randID] = BR::WeakPointerT<SharedObjectType>();
						}
					}
				}
			}

			// Pretend to use
			Sleep(0);

		}
		return true;
	}
};

BR::Mutex ThreadGroupTwo::stm_StorageLock;
BR::WeakPointerT<SharedObjectType> ThreadGroupTwo::stm_PointerStorage[MAX_TEST_ID];



void SharedPointerTest1()
{
	BR::SharedReferenceManager *pReferenceManager = new BR::SharedReferenceManager();


	{
		SharedObjectType *sharedObj;
		BR::SharedPointerT<SharedObjectType> sharedPtr;
		BR::SharedPointerT<SharedObjectType> sharedPtr2;
		{
			sharedObj = new SharedObjectType(1);
			pReferenceManager->RegisterSharedObject(sharedObj);
			sharedPtr = BR::SharedPointerT<SharedObjectType>(sharedObj);
			sharedPtr2 = sharedPtr;
			BR::SharedPointerT<SharedObjectType> sharedPtr3 = sharedPtr;
		}
		{
			BR::SharedPointerT<SharedObjectType> sharedPtr4(sharedPtr);
		}
		{
			BR::SharedPointerT<SharedObjectType> sharedPtr5((SharedObjectType*)sharedPtr);
		}
	}
	pReferenceManager->UpdateReferenceManager();


	std::vector<BR::Thread*> Threads;
	for (int iThread = 0; iThread < MAX_GROUP_ONE; iThread++)
	{
		auto thread = new ThreadGroupOne(*pReferenceManager);
		Threads.push_back(thread);
		thread->Start();
	}

	for (int iThread = 0; iThread < MAX_GROUP_TWO; iThread++)
	{
		auto thread = new ThreadGroupTwo;
		Threads.push_back(thread);
		thread->Start();
	}

	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();
	while ((std::chrono::system_clock::now() - start) < std::chrono::seconds(TestScale*10))
	{
		pReferenceManager->UpdateReferenceManager();
		Sleep(100);
	}

	std::for_each(Threads.begin(), Threads.end(), [](BR::Thread* pThread)
	{
		pThread->Stop(true);
		delete pThread;
	});
	Threads.empty();

	for (int iEle = 0; iEle < MAX_TEST_ID; iEle++)
	{
		ThreadGroupOne::stm_PointerStorage[iEle] = BR::SharedPointerT<SharedObjectType>();
	}

	for (int iEle = 0; iEle < MAX_TEST_ID; iEle++)
	{
		ThreadGroupTwo::stm_PointerStorage[iEle] = BR::WeakPointerT<SharedObjectType>();
	}

	pReferenceManager->UpdateReferenceManager();
	delete pReferenceManager;
}



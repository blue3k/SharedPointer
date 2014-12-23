// ShardPointerTest.cpp : Defines the entry point for the console application.
//


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



#define MAX_TEST_ID      60
#define MAX_ACCESS_COUNT 10
#define MAX_GROUP_ONE    10
#define MAX_GROUP_TWO    10

class SharedObjectType : public BR::SharedObject
{
public:
	long Identity;
	long WeakReleasedCount;

	SharedObjectType(long identity)
		:Identity(identity)
		, WeakReleasedCount(0)
	{
		pItem = new TestData;
		TestID = Identity;
	}

	~SharedObjectType()
	{

	}


	struct TestData {
		BR::Interlocked::CounterType UsedCount;
		BR::Interlocked::CounterType WeakUsedCount;
	};
	TestData *pItem;

	virtual void Dispose() override
	{
		delete pItem;
		pItem = nullptr;
	}
};



const unsigned TaskWorkerInterval = 50;

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
					auto usedCount = BR::Interlocked::Increment(localShared->pItem->UsedCount);
					if (usedCount > MAX_ACCESS_COUNT)
						stm_PointerStorage[randID] = BR::SharedPointerT<SharedObjectType>();
				}
			}

			// Pretend to use
			Sleep(50);
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
						//_CrtCheckMemory();
					}
				}
				else
				{
					localShared = stm_PointerStorage[randID];
					BR::SharedPointerT<SharedObjectType> shared;
					localShared.GetSharedPointer(shared);
					if (shared != nullptr)
					{
						auto usedCount = BR::Interlocked::Increment(shared->pItem->WeakUsedCount);
						if (usedCount > MAX_ACCESS_COUNT)
						{
							BR::Interlocked::Increment(shared->WeakReleasedCount);
							stm_PointerStorage[randID] = BR::WeakPointerT<SharedObjectType>();
						}
					}
				}
			}

			// Pretend to use
			Sleep(10);

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
	end = std::chrono::system_clock::now();
	while ((std::chrono::system_clock::now() - start) < std::chrono::seconds(60))
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



////////////////////////////////////////////////////////////
//
//
//


class WorkingEntity : public BR::SharedObject
{
private:
	UINT m_WorkerThreadID;
	UINT m_EntityID;


public:
	struct TestData {
		BR::Interlocked::CounterType Item;
	};
	TestData *pItem;

	WorkingEntity(UINT entityID)
		: m_WorkerThreadID(0)
		, m_EntityID(entityID)
	{
		pItem = new TestData;
		pItem->Item = 1;
		TestID = entityID;
	}

	inline UINT GetWorkerThreadID()                   { return m_WorkerThreadID; }

	HRESULT AssignWorkerThreadID(UINT workerThreadID)
	{
		if (m_WorkerThreadID != 0)
			return E_UNEXPECTED;

		m_WorkerThreadID = workerThreadID;

		return S_OK;
	}

	virtual void Dispose() override
	{
		delete pItem;
		pItem = nullptr;
	}
};


class TaskWorkerThread : public BR::Thread
{
public:
	static ULONGLONG WorkedItems;
	static ULONGLONG SkippedWorkedItems;

	TaskWorkerThread()
		: m_TaskWorkerInterval(1)
	{
	}

	~TaskWorkerThread()
	{
	}

	HRESULT PushTask(BR::WeakPointerT<WorkingEntity> taskItem)
	{
		if (taskItem == nullptr)
			return E_INVALIDARG;

		m_WorkItemQueue.push(taskItem);

		return S_OK;
	}

	virtual bool Run() override
	{
		while (true)
		{
			ULONG loopInterval = UpdateInterval(m_TaskWorkerInterval);

			if (CheckKillEvent(loopInterval))
			{
				// Kill Event signaled
				break;
			}

			BR::WeakPointerT<WorkingEntity> workItem;
			while (m_WorkItemQueue.try_pop(workItem))
			{
				BR::SharedPointerT<WorkingEntity> workObj;
				workItem.GetSharedPointer(workObj);


				// If disposed skip item
				if (workObj == nullptr)
				{
					BR::Interlocked::Increment(SkippedWorkedItems);
					continue;
				}

				// Do some works
				workObj->pItem->Item = workObj->GetReferenceCount();
				// let other threads do some job
				Sleep(0);
				workObj->pItem->Item = workObj->GetReferenceCount();

				BR::Interlocked::Increment(WorkedItems);
			}
		}

		return true;
	}

private:

	// Task worker mnimum interval
	ULONG m_TaskWorkerInterval;

	// Task queue
	Concurrency::concurrent_queue<BR::WeakPointerT<WorkingEntity>> m_WorkItemQueue;
};

ULONGLONG TaskWorkerThread::WorkedItems = 0;
ULONGLONG TaskWorkerThread::SkippedWorkedItems = 0;


class EntityTaskManager
{
private:

	UINT m_NumberOfWorker;
	UINT m_NumberOfTestEntity;

	UINT m_LatestAssignedWorkerID;

	std::vector<TaskWorkerThread*> m_Workers;

	BR::SharedReferenceManager m_ReferenceManager;
	std::unordered_map<UINT, BR::SharedPointerT<WorkingEntity>> m_EntityManager;

public:

	EntityTaskManager()
		: m_NumberOfWorker(5)
		, m_NumberOfTestEntity(100)
		, m_LatestAssignedWorkerID(0)
	{

	}

	~EntityTaskManager()
	{

	}

	HRESULT InitializeTaskManager()
	{
		for (UINT worker = 0; worker < m_NumberOfWorker; worker++)
		{
			auto pWorker = new TaskWorkerThread;

			m_Workers.push_back(pWorker);

			pWorker->Start();
		}

		// setup test entities
		for (UINT entity = 0; entity < m_NumberOfTestEntity; entity++)
		{
			UINT entityID = entity + 1;
			auto entityPtr = new WorkingEntity(entityID);
			m_ReferenceManager.RegisterSharedObject(entityPtr);
			m_EntityManager.insert(std::make_pair(entityID, entityPtr));
		}

		return S_OK;
	}

	void TerminateTaskManager()
	{
		std::for_each(m_Workers.begin(), m_Workers.end(), [](TaskWorkerThread* pWorker)
		{
			pWorker->Stop(true);

			delete pWorker;
		});

		m_Workers.clear();

		m_EntityManager.clear();

		m_ReferenceManager.UpdateReferenceManager();

		AssertRel(m_ReferenceManager.m_PendingFreeObjects.size() == 0);
		m_ReferenceManager.m_PendingFreeObjects.clear();
	}

	// Enqueue event
	HRESULT TestEnqueueEvent()
	{
		auto entityID = rand() % m_NumberOfTestEntity;
		auto itEntity = m_EntityManager.find(entityID);
		UINT workID = 0;
		BR::SharedPointerT<WorkingEntity> entity;
		if (itEntity == m_EntityManager.end())
		{
			auto newObject = new WorkingEntity(entityID);
			m_ReferenceManager.RegisterSharedObject(newObject);

			entity = BR::SharedPointerT<WorkingEntity>(newObject);
			m_EntityManager.insert(std::make_pair(entityID, entity));
		}
		else
		{
			entity = itEntity->second;
		}

		auto isDelete = (rand() % 100) > 70;
		if (entity->GetWorkerThreadID() == 0)
		{
			// round robin
			m_LatestAssignedWorkerID = (m_LatestAssignedWorkerID + 1) % m_Workers.size();
			entity->AssignWorkerThreadID(m_LatestAssignedWorkerID);
		}

		workID = entity->GetWorkerThreadID();

		BR::WeakPointerT<WorkingEntity> weakPointer;
		entity.GetWeakPointer(weakPointer);

		m_Workers[workID]->PushTask(weakPointer);

		if (isDelete && itEntity != m_EntityManager.end())
		{
			m_EntityManager.erase(itEntity);
		}

		return S_OK;
	}

	void Update()
	{
		m_ReferenceManager.UpdateReferenceManager();
	}

};


EntityTaskManager* entityManager = nullptr;
void SharedPointerTest2()
{
	const INT64 TEST_LENGTH = 9999999;

	entityManager = new EntityTaskManager;
	entityManager->InitializeTaskManager();

	for (INT64 ID = 0; ID < TEST_LENGTH; ID++)
	{
		entityManager->TestEnqueueEvent();
		entityManager->Update();
	};

	while ((TaskWorkerThread::WorkedItems + TaskWorkerThread::SkippedWorkedItems) < TEST_LENGTH)
	{
		Sleep(1000);
		entityManager->Update();
	}

	entityManager->Update();
	entityManager->TerminateTaskManager();

	delete entityManager;
}




int _tmain(int argc, _TCHAR* argv[])
{
	SharedPointerTest1();

	_CrtDumpMemoryLeaks();

	SharedPointerTest2();

	_CrtDumpMemoryLeaks();

	return 0;
}



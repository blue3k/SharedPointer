////////////////////////////////////////////////////////////////////////////////
// 
// CopyRight (c) 2013 MadK
// 
// Author : KyungKun Ko
//
// Description : Test 2
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
		CounterType Item;
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
	static BR::SyncCounter WorkedItems;
	static BR::SyncCounter SkippedWorkedItems;

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
				std::atomic_thread_fence(std::memory_order_acquire);

				BR::SharedPointerT<WorkingEntity> workObj;
				workItem.GetSharedPointer(workObj);


				// If disposed skip item
				if (workObj == nullptr)
				{
					SkippedWorkedItems.fetch_add(1,std::memory_order_relaxed);
					continue;
				}

				// Do some works
				workObj->pItem->Item = (CounterType)workObj->GetReferenceCount();
				// let other threads do some job
				Sleep(0);
				workObj->pItem->Item = (CounterType)workObj->GetReferenceCount();

				WorkedItems.fetch_add(1, std::memory_order_relaxed);
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

BR::SyncCounter TaskWorkerThread::WorkedItems = 0;
BR::SyncCounter TaskWorkerThread::SkippedWorkedItems = 0;


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
		weakPointer = entity;

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
	const INT64 TEST_LENGTH = 999999 * TestScale;

	entityManager = new EntityTaskManager;
	entityManager->InitializeTaskManager();

	for (INT64 ID = 0; ID < TEST_LENGTH; ID++)
	{
		entityManager->TestEnqueueEvent();
		entityManager->Update();
	};

	while ((TaskWorkerThread::WorkedItems.load(std::memory_order_relaxed) + TaskWorkerThread::SkippedWorkedItems.load(std::memory_order_relaxed)) < TEST_LENGTH)
	{
		Sleep(1000);
		entityManager->Update();
	}

	entityManager->Update();
	entityManager->TerminateTaskManager();

	delete entityManager;
}



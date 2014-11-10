


#include "synchronize.h"
#include "sharedpointermanager.h"
#include <queue>


namespace BR
{
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	Internal implementation
	//

	class SharedPointerManager_Impl
	{
	private:

		Mutex m_Mutex;
		std::queue<ObjectRef*> m_FreeQueue;

	public:

		SharedPointerManager_Impl()
		{

		}

		~SharedPointerManager_Impl()
		{
			
		}

		// 
		void FreeSharedReference(ObjectRef* pObj)
		{
			if (pObj == nullptr)
				return;

			MutexScopeLock localLock(m_Mutex);
			Interlocked::Increment(pObj->ManagerReferenceCount);
			m_FreeQueue.push(pObj);
		}

		void FreeWeakReference(ObjectRef* pObj)
		{
			if (pObj == nullptr)
				return;

			MutexScopeLock localLock(m_Mutex);
			Interlocked::Increment(pObj->ManagerReferenceCount);
			m_FreeQueue.push(pObj);
		}

		void InitializeSharedPointerManager()
		{
		}

		// Garbagte Collect free Pointers
		void FreePendingDelete()
		{
			MutexScopeLock localLock(m_Mutex);

			auto numItems = m_FreeQueue.size();
			for (auto item = 0; item < numItems; item++)
			{
				if (m_FreeQueue.empty()) break;

				auto pObj = m_FreeQueue.front();
				m_FreeQueue.pop();

				auto managerRefCount = Interlocked::Decrement(pObj->ManagerReferenceCount);
				// it's queued more than once. leave this object for later operation
				if (managerRefCount > 0) continue;
				assert(managerRefCount >= 0);

				// skip not released pointers
				if (pObj->ReferenceCount > 0)
					continue;

				// dispose if main reference counter became zero
				if (!pObj->IsDisposed)
				{
					pObj->Dispose();
					pObj->IsDisposed = true;
				}

				// somebody is still using this object pointer
				if (pObj->WeakReferenceCount > 0)
					continue;

				// final check to prevent ABA reference count operation case
				if (pObj->ManagerReferenceCount > 0)
					continue;

				// noone is using this object pointer. We are good to release it
				delete pObj;
			}
		}
	};



	SharedPointerManager_Impl* SharedPointerManager::m_pInstance = nullptr;

	// Free object
	// The object will be added to release queue
	void SharedPointerManager::FreeSharedReference(ObjectRef* pObj)
	{
		if (m_pInstance == nullptr)
			return;

		m_pInstance->FreeSharedReference(pObj);
	}

	void SharedPointerManager::FreeWeakReference(ObjectRef* pObj)
	{
		if (m_pInstance == nullptr)
			return;

		m_pInstance->FreeWeakReference(pObj);
	}

	void SharedPointerManager::InitializeSharedPointerManager()
	{
		if (m_pInstance != nullptr)
			return;

		m_pInstance = new SharedPointerManager_Impl;
		m_pInstance->InitializeSharedPointerManager();
	}

	void SharedPointerManager::TerminateSharedPointerManager()
	{
		if (m_pInstance == nullptr)
			return;

		m_pInstance->FreePendingDelete();
		delete m_pInstance;
	}

	// Garbagte Collect free Pointers
	void SharedPointerManager::UpdateSharedPointerManager()
	{
		if (m_pInstance == nullptr)
			return;

		m_pInstance->FreePendingDelete();
	}



}


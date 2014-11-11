


#include "Thread/synchronize.h"
#include "sharedreferencemanager.h"



namespace BR
{
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	Internal implementation
	//


	void SharedReferenceManager::RegisterSharedObject(SharedObject* pSharedObject)
	{
		assert(pSharedObject->m_ReferenceManagerObject == nullptr || pSharedObject->m_ReferenceManagerObject == this);

		if (pSharedObject->m_ReferenceManagerObject != this)
			Interlocked::Increment(m_ObjectCount);

		// This will just link manager to object
		pSharedObject->m_ReferenceManagerObject = this;
	}

	// 
	void SharedReferenceManager::FreeSharedReference(SharedObject* pObj)
	{
		if (pObj == nullptr)
			return;

		MutexScopeLock localLock(m_Mutex);
		Interlocked::Increment(pObj->m_ManagerReferenceCount);
		m_FreeQueue.push(pObj);
	}

	void SharedReferenceManager::FreeWeakReference(SharedObject* pObj)
	{
		if (pObj == nullptr)
			return;

		MutexScopeLock localLock(m_Mutex);
		Interlocked::Increment(pObj->m_ManagerReferenceCount);
		m_FreeQueue.push(pObj);
	}


	// Garbagte Collect free Pointers
	void SharedReferenceManager::FreePendingDelete()
	{
		MutexScopeLock localLock(m_Mutex);

		auto numItems = m_FreeQueue.size();
		for (auto item = 0; item < numItems; item++)
		{
			if (m_FreeQueue.empty()) break;

			auto pObj = m_FreeQueue.front();
			m_FreeQueue.pop();

			auto managerRefCount = Interlocked::Decrement(pObj->m_ManagerReferenceCount);
			// it's queued more than once. leave this object for later operation
			if (managerRefCount > 0) continue;
			assert(managerRefCount >= 0);

			// skip not released pointers
			if (pObj->m_ReferenceCount > 0)
				continue;

			// dispose if main reference counter became zero
			if (!pObj->m_IsDisposed)
			{
				pObj->Dispose();
				pObj->m_IsDisposed = true;
			}

			// somebody is still using this object pointer
			if (pObj->m_WeakReferenceCount > 0)
				continue;

			// final check to prevent ABA reference count problem
			if (pObj->m_ManagerReferenceCount > 0)
				continue;

			// no one is using this object pointer. We are good to release it
			delete pObj;
		}

	}

}


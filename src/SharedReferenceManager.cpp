


#include "Thread/synchronize.h"
#include "SharedReferenceManager.h"
#include "SharedPointer.h"


namespace BR
{


	WeakPointer SharedPointer::GetWeakPointer()
	{
		return WeakPointer(m_pObject);
	}

	const WeakPointer SharedPointer::GetWeakPointer() const
	{
		return WeakPointer(m_pObject);
	}



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

			// This can't be processed now
			if( pObj->m_ManageCount > 0 )
			{
				m_FreeQueue.push(pObj);
				continue;
			}

			auto managerRefCount = Interlocked::Decrement(pObj->m_ManagerReferenceCount);
			// it's queued more than once. leave this object for later operation
			if (managerRefCount > 0)
				continue;
			assert(managerRefCount >= 0);

			// skip not released pointers
			if (pObj->GetReferenceCount() > 0)
				continue;

			// dispose if main reference counter became zero
			if (!pObj->m_IsDisposed)
			{
				pObj->Dispose();
				pObj->m_IsDisposed = true;
			}

			// somebody is still using this object pointer
			if (pObj->GetWeakReferenceCount() > 0)
				continue;

			// final check to prevent ABA reference count problem
			if (pObj->GetManagerReferenceCount() > 0)
				continue;

			// no one is using this object pointer. We are good to release it
			assert(pObj->GetReferenceCount() == 0 && pObj->GetWeakReferenceCount() == 0);
			delete pObj;
		}


	}

}


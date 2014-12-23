////////////////////////////////////////////////////////////////////////////////
// 
// CopyRight (c) 2014 MadK
// 
// Author : KyungKun Ko
//
// Description : Shared pointer
//
////////////////////////////////////////////////////////////////////////////////



#include "Thread/Synchronize.h"
#include "SharedReferenceManager.h"
#include "SharedPointer.h"



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

#ifdef REFERENCE_DEBUG_TRACKING
		Assert(pSharedObject->TestID >= 0 && pSharedObject->TestID < 10000);
		m_PendingFreeObjects.insert(std::make_pair((unsigned int)pSharedObject->TestID, pSharedObject));
#endif
	}

	// 
	void SharedReferenceManager::FreeSharedReference(SharedObject* pObj
#ifdef REFERENCE_DEBUG_TRACKING
		, const char* fileName, int lineNumber
#endif
		)
	{
		if (pObj == nullptr)
			return;

#ifdef REFERENCE_DEBUG_TRACKING
		auto state = pObj->m_SharedObjectState;

		pObj->LatestReleaseFile = fileName;
		pObj->LatestReleaseLine = lineNumber;
		pObj->LatestReleaseState = state;
#endif

		Interlocked::Increment(pObj->m_ManagerReferenceCount);
		m_FreeQueue.push(pObj);
	}



	// Garbagte Collect free Pointers
	void SharedReferenceManager::UpdateReferenceManager()
	{
		auto numItems = m_FreeQueue.unsafe_size();
		for (auto item = 0; item < numItems; item++)
		{
			SharedObject* pObj = nullptr;
			if (!m_FreeQueue.try_pop(pObj))
				//if (FAILED(m_FreeQueue.Dequeue(pObj)))
				break;

			// This can't be processed now
			if( pObj->m_ManageCount > 0 )
			{
#ifdef REFERENCE_DEBUG_TRACKING
				pObj->LatestQueueProcessResult = "Still In Manage";
#endif
				m_FreeQueue.push(pObj);
				continue;
			}

			auto managerRefCount = Interlocked::Decrement(pObj->m_ManagerReferenceCount);
			// it's queued more than once. leave this object for later operation
			if (managerRefCount > 0)
			{
#ifdef REFERENCE_DEBUG_TRACKING
				pObj->LatestQueueProcessResult = "In Queue again";
#endif
				continue;
			}

			// Lock
			int itryCount = 0;
			SharedObject::SharedObjectState prevState = SharedObject::SharedObjectState::Instanced;
			do
			{
				itryCount++;
				Assert(itryCount < 500);
				if ((itryCount % 5) == 0)
					Sleep(1);
				else
					Sleep(0);

				switch (pObj->m_SharedObjectState)
				{
				case SharedObject::SharedObjectState::Instanced:
					prevState = SharedObject::SharedObjectState::Instanced;
					break;
				case SharedObject::SharedObjectState::Disposed:
					prevState = SharedObject::SharedObjectState::Disposed; // we can try one more time,
					break;
				case SharedObject::SharedObjectState::LockedForSharedReferencing:
					// Wait
					continue;
				case SharedObject::SharedObjectState::Disposing:
				default:
					AssertRel(!"Invalid shared Object state");
#ifdef REFERENCE_DEBUG_TRACKING
					pObj->LatestQueueProcessResult = "Invalid shared Object state";
#endif
					continue;
				}

				if (Interlocked::CompareExchange((long&)pObj->m_SharedObjectState, (long)SharedObject::SharedObjectState::LockedForSharedReferencing, (long)prevState))
					break;

			} while (true);


			// skip not released pointers
			if (pObj->GetReferenceCount() > 0)
			{
				// back to normal state
				itryCount = 0;
				do
				{
					itryCount++;
					Assert(itryCount < 10);
				} while (!Interlocked::CompareExchange((long&)pObj->m_SharedObjectState, (long)SharedObject::SharedObjectState::Instanced, (long)SharedObject::SharedObjectState::LockedForSharedReferencing));
				continue;
			}

			// No one using it, dispose
			itryCount = 0;
			do
			{
				itryCount++;
				AssertRel(itryCount < 10);
			} while (!Interlocked::CompareExchange((long&)pObj->m_SharedObjectState, (long)SharedObject::SharedObjectState::Disposing, (long)SharedObject::SharedObjectState::LockedForSharedReferencing));


			if (prevState == SharedObject::SharedObjectState::Instanced)
			{
				// dispose if main reference counter becme zero
				pObj->Dispose();
			}

			// Counter checking order is important. see WeakPointer release
			if (   pObj->GetWeakReferenceCount() > 0        // If somebody is still accessing this object pointer
				|| pObj->m_ManageCount > 0                  // This will be enqueued or something will be happened
				|| pObj->GetManagerReferenceCount() > 0)    // final check to prevent ABA reference count problem
			{
				itryCount = 0;
				do
				{
					itryCount++;
					AssertRel(itryCount < 10);
				} while (!Interlocked::CompareExchange((long&)pObj->m_SharedObjectState, (long)SharedObject::SharedObjectState::Disposed, (long)SharedObject::SharedObjectState::Disposing));

				AssertRel(pObj->m_SharedObjectState == SharedObject::SharedObjectState::Disposed);

#ifdef REFERENCE_DEBUG_TRACKING
				pObj->LatestQueueProcessResult = "Still referenced";
#endif

				continue;
			}
			else
			{
				itryCount = 0;
				do
				{
					itryCount++;
					Assert(itryCount < 10);
				} while (!Interlocked::CompareExchange((long&)pObj->m_SharedObjectState, (long)SharedObject::SharedObjectState::Deleted, (long)SharedObject::SharedObjectState::Disposing));

#ifdef REFERENCE_DEBUG_TRACKING
				pObj->LatestQueueProcessResult = "Deleted";
#endif
			}


#ifdef REFERENCE_DEBUG_TRACKING

			AssertRel(pObj->GetReferenceCount() == 0);
			AssertRel(pObj->m_SharedObjectState == SharedObject::SharedObjectState::Disposed || pObj->m_SharedObjectState == SharedObject::SharedObjectState::Deleted);
			pObj->m_ReferenceManagerObject = nullptr;

			Interlocked::Increment(DeletedObjects);


			// no one is using this object pointer. We are good to release it
			AssertRel(pObj->GetReferenceCount() == 0 && pObj->GetWeakReferenceCount() == 0 && pObj->GetManagerReferenceCount() == 0);
			m_PendingFreeObjects.unsafe_erase(pObj->TestID);
			_ReadWriteBarrier();
#endif
			delete pObj;
		}


	}

}


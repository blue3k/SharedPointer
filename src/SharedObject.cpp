////////////////////////////////////////////////////////////////////////////////
// 
// CopyRight (c) 2014 MadK
// 
// Author : KyungKun Ko
//
// Description : Shared pointer
//
////////////////////////////////////////////////////////////////////////////////


//#include "StdAfx.h"
#include "Thread/Synchronize.h"
#include "SharedReferenceManager.h"
#include "SharedPointer.h"


namespace BR
{
	///////////////////////////////////////////////////////////////////////////
	//
	//
	//

	void SharedObject::AddReference() const
	{
		m_ReferenceCount.fetch_add(1, std::memory_order_relaxed);
		//Interlocked::Increment(m_ReferenceCount);
	}

	void SharedObject::ReleaseReference() const
	{
		if (m_ReferenceManagerObject != nullptr)
		{
			ReleaseReference_ByManager(m_ReferenceCount
#ifdef REFERENCE_DEBUG_TRACKING
				, __FILE__, __LINE__
#endif
				);
		}
		else
			ReleaseReference_ByItself(m_ReferenceCount);
	}

	void SharedObject::AddWeakReference() const
	{
		m_WeakReferenceCount.fetch_add(1, std::memory_order_relaxed);
	}

	void SharedObject::ReleaseWeakReference() const
	{
		auto state = m_SharedObjectState.load(std::memory_order_relaxed);
		AssertRel(state != SharedObject::SharedObjectState::Deleted);

		if (m_ReferenceManagerObject != nullptr)
		{
			ReleaseReference_ByManager(m_WeakReferenceCount
#ifdef REFERENCE_DEBUG_TRACKING
				, __FILE__, __LINE__
#endif
				);
		}
		else
			ReleaseReference_ByItself(m_WeakReferenceCount);
	}

	void SharedObject::ReleaseReference_ByManager(SyncCounter& referenceCounter
#ifdef REFERENCE_DEBUG_TRACKING
		, const char* fileName, int lineNumber
#endif
		) const
	{
		AssertRel(m_ReferenceManagerObject != nullptr);
		//Assert(referenceCounter > 0);

		//ScopeCounter localCount((Interlocked::CounterType&)m_ManageCount);
		m_ManageCount.fetch_add(1, std::memory_order_acquire);

		auto localRef = m_ReferenceManagerObject;
		auto decValue = referenceCounter.fetch_sub(1, std::memory_order_acquire) - 1;
		if (decValue <= 0)
		{
			//Sleep(1);
			localRef->FreeSharedReference(const_cast<SharedObject*>(this)
#ifdef REFERENCE_DEBUG_TRACKING
				, fileName, lineNumber
#endif
				);
		}

		m_ManageCount.fetch_sub(1, std::memory_order_release);
	}

	void SharedObject::ReleaseReference_ByItself(SyncCounter& referenceCounter) const
	{
		assert(referenceCounter > 0);

		m_ManageCount.fetch_add(1, std::memory_order_acquire);
		//ScopeCounter localCount((Interlocked::CounterType&)m_ManageCount);

		auto decValue = referenceCounter.fetch_sub(1, std::memory_order_acquire) - 1;
		if (decValue <= 0)
		{
			int iTry = 0;
			SharedObjectState initialValue = m_SharedObjectState.load(std::memory_order_acquire);
			do
			{
				iTry++;
				if ((iTry % 5) == 0)
					Sleep(5);

				switch (initialValue)
				{
				case SharedObjectState::Instanced:
					//initialValue = (long)SharedObjectState::Instanced;
					break;
				case SharedObjectState::Disposed:
					//initialValue = (long)SharedObjectState::Disposed; // ?
					//AssertRel(!"Invalid shared object state");
					break;
				case SharedObjectState::LockedForSharedReferencing:
					initialValue = m_SharedObjectState.load(std::memory_order_acquire);
					continue;
				case SharedObjectState::Disposing:
				case SharedObjectState::Deleted:
					//case SharedObjectState::Disposed:
				default:
					AssertRel(!"Invalid shared object state");
					return;
				}

				if (m_SharedObjectState.compare_exchange_weak(initialValue, SharedObjectState::LockedForSharedReferencing, std::memory_order_release, std::memory_order_relaxed))
					break;

			} while (true);
			//} while (!Interlocked::CompareExchange((long&)m_SharedObjectState, (long)SharedObjectState::LockedForSharedReferencing, initialValue));

			if (m_ReferenceCount <= 0)
			{
				// We don't need lock process, It's already locked
				const_cast<SharedObject*>(this)->Dispose();

				// Delete will be done at outside
				// Variable checking order is significant
				if (m_WeakReferenceCount <= 0
					&& m_ManageCount == 0)
					delete this;
				else
					m_SharedObjectState.store(initialValue, std::memory_order_release);
			}
			else
				m_SharedObjectState.store(initialValue, std::memory_order_release);
		}

		m_ManageCount.fetch_sub(1, std::memory_order_release);
	}


	void SharedObject::GetSharedPointer(SharedPointer& shardPointer) const
	{
		shardPointer = nullptr;

		// get the state lock
		int iTry = 0;
		auto initialValue = m_SharedObjectState.load(std::memory_order_relaxed);
		do{
			iTry++;
			if ((iTry % 2) == 0)
				Sleep(2);

			switch (initialValue)
			{
			case SharedObjectState::Instanced:
				// Can get the lock
				break;
			case SharedObjectState::LockedForSharedReferencing:
				initialValue = m_SharedObjectState.load(std::memory_order_relaxed);
				continue;
			default:
				return;
			}

			if (GetReferenceCount() == 0)
				return;

			//if (Interlocked::CompareExchange((long&)m_SharedObjectState, (long)SharedObjectState::LockedForSharedReferencing, (long)SharedObjectState::Instanced))
			//	break;

			if (m_SharedObjectState.compare_exchange_weak(initialValue, SharedObjectState::LockedForSharedReferencing, std::memory_order_release, std::memory_order_relaxed))
				break;

		} while (true);// (!m_SharedObjectState.compare_exchange_weak(initialValue, SharedObjectState::LockedForSharedReferencing, std::memory_order_release, std::memory_order_relaxed));

		//AssertRel(m_SharedObjectState != );
		shardPointer.SetPointer(const_cast<SharedObject*>(this));

		// back to normal sate
		iTry = 0;
		//while (!Interlocked::CompareExchange((long&)m_SharedObjectState, (long)SharedObjectState::Instanced, (long)SharedObjectState::LockedForSharedReferencing))
		do
		{
			iTry++;
			if ((iTry % 2) == 0)
				Sleep(2);

			initialValue = SharedObjectState::LockedForSharedReferencing;
			//Assert(m_SharedObjectState == SharedObjectState::LockedForSharedReferencing);
			//Sleep(0);//
		} while (!m_SharedObjectState.compare_exchange_weak(initialValue, SharedObjectState::Instanced, std::memory_order_relaxed, std::memory_order_relaxed));
	}

	///////////////////////////////////////////////////////////////////////////
	//
	//
	//


	SharedPointer SharedPointer::NullValue;
	WeakPointer WeakPointer::NullValue;


	void SharedPointer::GetWeakPointer(WeakPointer& pointer)
	{
		pointer = m_pObject;
	}


}


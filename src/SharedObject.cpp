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
		Interlocked::Increment(m_ReferenceCount);
	}

	void SharedObject::ReleaseReference() const
	{
		if (m_ReferenceManagerObject != nullptr)
		{
			ReleaseReference_ByManager(const_cast<volatile Interlocked::CounterType&>(m_ReferenceCount)
#ifdef REFERENCE_DEBUG_TRACKING
				, __FILE__, __LINE__
#endif
				);
		}
		else
			ReleaseReference_ByItself(const_cast<volatile Interlocked::CounterType&>(m_ReferenceCount));
	}

	void SharedObject::AddWeakReference() const
	{
		Interlocked::Increment(m_WeakReferenceCount);
	}

	void SharedObject::ReleaseWeakReference() const
	{
		auto state = m_SharedObjectState;
		AssertRel(state != SharedObject::SharedObjectState::Deleted);

		if (m_ReferenceManagerObject != nullptr)
		{
			ReleaseReference_ByManager(const_cast<volatile Interlocked::CounterType&>(m_WeakReferenceCount)
#ifdef REFERENCE_DEBUG_TRACKING
				, __FILE__, __LINE__
#endif
				);
		}
		else
			ReleaseReference_ByItself(const_cast<volatile Interlocked::CounterType&>(m_WeakReferenceCount));
	}

	void SharedObject::ReleaseReference_ByManager(volatile Interlocked::CounterType& referenceCounter
#ifdef REFERENCE_DEBUG_TRACKING
		, const char* fileName, int lineNumber
#endif
		) const
	{
		AssertRel(m_ReferenceManagerObject != nullptr);
		Assert(referenceCounter > 0);

		ScopeCounter localCount((Interlocked::CounterType&)m_ManageCount);

		auto localRef = m_ReferenceManagerObject;
		auto decValue = Interlocked::Decrement(referenceCounter);
		if (decValue <= 0)
		{
			//Sleep(1);
			localRef->FreeSharedReference(const_cast<SharedObject*>(this)
#ifdef REFERENCE_DEBUG_TRACKING
				, fileName, lineNumber
#endif
				);
		}
	}

	void SharedObject::ReleaseReference_ByItself(volatile Interlocked::CounterType& referenceCounter) const
	{
		assert(referenceCounter > 0);

		ScopeCounter localCount((Interlocked::CounterType&)m_ManageCount);

		auto decValue = Interlocked::Decrement(referenceCounter);
		if (decValue <= 0)
		{
			int iTry = 0;
			long initialValue = (long)SharedObjectState::Instanced;
			do
			{
				iTry++;
				if ((iTry % 5) == 0)
					Sleep(5);

				switch (m_SharedObjectState)
				{
				case SharedObjectState::Instanced:
					initialValue = (long)SharedObjectState::Instanced;
					break;
				case SharedObjectState::Disposed:
					initialValue = (long)SharedObjectState::Disposed; // ?
					//AssertRel(!"Invalid shared object state");
					break;
				case SharedObjectState::LockedForSharedReferencing:
					continue;
				case SharedObjectState::Disposing:
				case SharedObjectState::Deleted:
					//case SharedObjectState::Disposed:
				default:
					AssertRel(!"Invalid shared object state");
					return;
				}

			} while (!Interlocked::CompareExchange((long&)m_SharedObjectState, (long)SharedObjectState::LockedForSharedReferencing, initialValue));

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
					m_SharedObjectState = (SharedObjectState)initialValue;
			}
			else
				m_SharedObjectState = (SharedObjectState)initialValue;
		}
	}


	void SharedObject::GetSharedPointer(SharedPointer& shardPointer) const
	{
		shardPointer = nullptr;

		// get the state lock
		int iTry = 0;
		do{
			iTry++;
			if ((iTry % 2) == 0)
				Sleep(2);

			switch (m_SharedObjectState)
			{
			case SharedObjectState::Instanced:
				// Can get the lock
				break;
			case SharedObjectState::LockedForSharedReferencing:
				continue;
			default:
				return;
			}

			if (GetReferenceCount() == 0)
				return;

			if (Interlocked::CompareExchange((long&)m_SharedObjectState, (long)SharedObjectState::LockedForSharedReferencing, (long)SharedObjectState::Instanced))
				break;

		} while (true);

		//AssertRel(m_SharedObjectState != );
		shardPointer.SetPointer(const_cast<SharedObject*>(this));

		// back to normal sate
		iTry = 0;
		while (!Interlocked::CompareExchange((long&)m_SharedObjectState, (long)SharedObjectState::Instanced, (long)SharedObjectState::LockedForSharedReferencing))
		{
			iTry++;
			if ((iTry % 2) == 0)
				Sleep(2);

			Assert(m_SharedObjectState == SharedObjectState::LockedForSharedReferencing);
			Sleep(0);
		}
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


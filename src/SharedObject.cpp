////////////////////////////////////////////////////////////////////////////////
// 
// CopyRight (c) 2014 Blue3k
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
		auto org = m_ReferenceCount.fetch_add(1, std::memory_order_relaxed);
		if (org == 0)
		{
			// First shared reference should have one designated weak reference
			Assert(GetWeakReferenceCount() == 1);
		}
	}

	void SharedObject::ReleaseReference() const
	{
		auto decValue = m_ReferenceCount.fetch_sub(1, std::memory_order_acquire) - 1;
		if (decValue <= 0)
		{
			const_cast<SharedObject*>(this)->Dispose();

			ReleaseWeakReference();
		}
	}

	void SharedObject::AddWeakReference() const
	{
		m_WeakReferenceCount.fetch_add(1, std::memory_order_relaxed);
	}

	void SharedObject::ReleaseWeakReference() const
	{
		if (m_ReferenceManagerObject != nullptr)
		{
			auto localRef = m_ReferenceManagerObject;
			Assert(localRef != nullptr);
			auto decValue = m_WeakReferenceCount.fetch_sub(1, std::memory_order_release);
			if (decValue <= 1)
			{
				localRef->FreeSharedReference(const_cast<SharedObject*>(this)
#ifdef REFERENCE_DEBUG_TRACKING
					, fileName, lineNumber
#endif
					);
			}
		}
		else
		{
			auto decValue = m_WeakReferenceCount.fetch_sub(1, std::memory_order_release);
			if (decValue <= 1)
			{
				delete this;
			}
		}
	}


	void SharedObject::GetSharedPointer(SharedPointer& shardPointer) const
	{
		ReferenceCounterType curReference;
		do
		{
			curReference = m_ReferenceCount.load(std::memory_order_consume);
			if (curReference <= 0)
			{
				shardPointer = nullptr;
				return;
			}
		} while (!m_ReferenceCount.compare_exchange_weak(curReference, curReference + 1, std::memory_order_relaxed, std::memory_order_relaxed));

		shardPointer.SetPointer(const_cast<SharedObject*>(this));
	}

	///////////////////////////////////////////////////////////////////////////
	//
	//
	//


	SharedPointer SharedPointer::NullValue;
	WeakPointer WeakPointer::NullValue;

}


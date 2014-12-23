////////////////////////////////////////////////////////////////////////////////
// 
// CopyRight (c) 2014 MadK
// 
// Author : KyungKun Ko
//
// Description : Shared pointer
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Thread/Synchronize.h"
//#include "Common/PageQueue.h"
//#include "Common/MemoryPool.h"
#include <concurrent_queue.h>
#include <concurrent_unordered_map.h>


namespace BR
{
	class SharedReferenceManager;
	class SharedPointer;

	// Shared object base
	class SharedObject
	{
	public:

		enum class SharedObjectState : LONG
		{
			None,
			Instanced,
			LockedForSharedReferencing,
			Disposing,
			Disposed,
			Deleted
		};

	private:
		// reference counter for shared references
		mutable SyncCounter m_ReferenceCount;

		// reference counter for weak references
		mutable SyncCounter m_WeakReferenceCount;

		// reference counter for manager reference
		mutable SyncCounter m_ManageCount;
		mutable SyncCounter m_ManagerReferenceCount;

		// Object statue
		volatile mutable SharedObjectState m_SharedObjectState;

		// reference manager object
		SharedReferenceManager *m_ReferenceManagerObject;

	public:

#ifdef REFERENCE_DEBUG_TRACKING

		long DeletedObjects = 0;

		volatile const char* LatestReleaseFile;
		volatile int LatestReleaseLine;
		volatile mutable SharedObjectState LatestReleaseState;

		volatile mutable const char* LatestQueueProcessResult;

#endif

		SharedObject()
			: m_ReferenceCount(0)
			, m_WeakReferenceCount(0)
			, m_ManageCount(0)
			, m_ManagerReferenceCount(0)
			, m_SharedObjectState(SharedObjectState::Instanced)
			, m_ReferenceManagerObject(nullptr)
		{

		}

		virtual ~SharedObject()
		{
			assert(GetReferenceCount() == 0 && GetWeakReferenceCount() == 0);
		}

		inline bool							GetIsDisposed() const					{ return m_SharedObjectState == SharedObjectState::Disposed || m_SharedObjectState == SharedObjectState::Disposing; }

		inline Interlocked::CounterType		GetReferenceCount() const				{ return m_ReferenceCount; }
		inline Interlocked::CounterType		GetWeakReferenceCount() const			{ return m_WeakReferenceCount; }
		inline Interlocked::CounterType		GetManagerReferenceCount() const		{ return m_ManagerReferenceCount; }

		inline SharedReferenceManager*		GetReferenceManager()					{ return m_ReferenceManagerObject; }

		long TestID;

		virtual void Dispose() {}

	private:

		void AddReference() const;
		void ReleaseReference() const;

		void AddWeakReference() const;
		void ReleaseWeakReference() const;

		void ReleaseReference_ByManager(SyncCounter& referenceCounter
#ifdef REFERENCE_DEBUG_TRACKING
			, const char* fileName, int lineNumber
#endif
			) const;
		void ReleaseReference_ByItself(SyncCounter& referenceCounter) const;

		void GetSharedPointer(SharedPointer& shardPointer) const;

		//void Dispose_Inter();

		friend class SharedReferenceManager;
		friend class SharedPointer;
		friend class WeakPointer;
	};

	//extern template class PageQueue<SharedObject*>;


	///////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	Shared object pointer manager
	//

	class SharedReferenceManager
	{
	private:

		// Free item queue
		concurrency::concurrent_queue<SharedObject*> m_FreeQueue;

		// Linked Object counter 
		Interlocked::CounterType m_ObjectCount;

	public:
		Concurrency::concurrent_unordered_map<UINT, SharedObject*> m_PendingFreeObjects;

	public:

		SharedReferenceManager()
			: m_ObjectCount(0)
		{

		}

		~SharedReferenceManager()
		{

		}

		// Register shared object
		void RegisterSharedObject(SharedObject* pSharedObject);

		// Free shared object
		void FreeSharedReference(SharedObject* pObj
#ifdef REFERENCE_DEBUG_TRACKING
			, const char* fileName, int lineNumber
#endif
			);

		// Garbagte Collect free Pointers
		void UpdateReferenceManager();
	};


}




#include "synchronize.h"
#include <queue>


namespace BR
{
	class SharedReferenceManager;

	class SharedObject
	{
	private:
		// reference counter for shared references
		Interlocked::CounterType m_ReferenceCount;

		// reference counter for weak references
		Interlocked::CounterType m_WeakReferenceCount;

		// reference counter for manager reference
		Interlocked::CounterType m_ManagerReferenceCount;

		// Is disposed
		bool m_IsDisposed;

		// reference manager object
		SharedReferenceManager *m_ReferenceManagerObject;

	public:

		SharedObject()
			: m_ReferenceCount(0)
			, m_WeakReferenceCount(0)
			, m_ManagerReferenceCount(0)
			, m_IsDisposed(false)
			, m_ReferenceManagerObject(nullptr)
		{

		}

		inline Interlocked::CounterType		GetReferenceCount()					{ return m_ReferenceCount; }
		inline Interlocked::CounterType		GetWeakReferenceCount()				{ return m_WeakReferenceCount; }
		inline Interlocked::CounterType		GetManagerReferenceCount()			{ return m_ManagerReferenceCount; }

		inline SharedReferenceManager*		GetReferenceManager()				{ return m_ReferenceManagerObject; }


		virtual void Dispose()
		{
			m_IsDisposed = true;
		}

		friend class SharedReferenceManager;
		friend class SharedPointer;
		friend class WeakPointer;
	};


	///////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	Shared object pointer manager
	//

	class SharedReferenceManager
	{
	private:

		// Mutex for thread safe
		Mutex m_Mutex;

		// STD Queue, If you wnat to use optimal solution, you should change the queue and mutex implementation for you
		std::queue<SharedObject*> m_FreeQueue;

		// Linked Object counter 
		Interlocked::CounterType m_ObjectCount;

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
		void FreeSharedReference(SharedObject* pObj);

		// free shared object
		void FreeWeakReference(SharedObject* pObj);

		// Garbagte Collect free Pointers
		void FreePendingDelete();
	};

}


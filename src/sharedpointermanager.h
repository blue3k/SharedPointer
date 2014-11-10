

#include "synchronize.h"


namespace BR
{

	struct ObjectRef
	{
		// reference counter for shared references
		Interlocked::CounterType ReferenceCount;

		// reference counter for weak references
		Interlocked::CounterType WeakReferenceCount;

		// reference counter for manager reference
		Interlocked::CounterType ManagerReferenceCount;

		// Is disposed
		bool IsDisposed;

		virtual void Dispose()
		{
			IsDisposed = true;
		}
	};


	// 
	class SharedPointerManager
	{
	public:
		// Free object
		// The object will be added to release queue
		static void FreeSharedReference(ObjectRef* pObj);
		static void FreeWeakReference(ObjectRef* pObj);

		static void InitializeSharedPointerManager();
		static void TerminateSharedPointerManager();

		// Garbagte Collect free Pointers
		static void UpdateSharedPointerManager();

	//private:
	//	static void UpdateSharedPointerManager_Internal();

	private:

		static class SharedPointerManager_Impl* m_pInstance;

	};

}


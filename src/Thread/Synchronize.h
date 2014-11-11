
#pragma once

#include <assert.h>

namespace BR
{


	///////////////////////////////////////////////////////////////////////////////
	//
	//	Critical Section
	//

	class MutexBase
	{
	public:
		virtual void Lock() = 0;
		virtual void UnLock() = 0;
	};


	// Scope locker
	class MutexScopeLock
	{
	public:
		MutexScopeLock(MutexBase& lock) : m_Mutex(lock)
		{
			m_Mutex.Lock();
		}

		~MutexScopeLock()
		{
			m_Mutex.UnLock();
		}

	private:
		MutexBase &m_Mutex;
	};


}


#if defined(WIN32) || defined(WIN64)
#include "synchronize_win.inl"
#else
#include "synchronize_linux.inl"
#endif


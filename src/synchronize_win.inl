
#pragma once


#include "intrin.h"
#include "windows.h"


namespace BR
{
	namespace Interlocked
	{

		inline long Increment(long volatile& value)						{ return _InterlockedIncrement(&value); }
		inline short Increment(short volatile& value)					{ return _InterlockedIncrement16(&value); }
		inline __int64 Increment(__int64 volatile& value)				{ return _InterlockedIncrement64(&value); }

		inline long Decrement(long volatile& value)						{ return _InterlockedDecrement(&value); }
		inline short Decrement(short volatile& value)					{ return _InterlockedDecrement16(&value); }
		inline __int64 Decrement(__int64 volatile& value)				{ return _InterlockedDecrement64(&value); }


#ifdef _M_X64

		typedef __int64 CounterType;


#elif defined(_M_IX86)

		typedef long CounterType;

#else

#pragma comment(user, "Not Defined platform")

#endif

	}




	///////////////////////////////////////////////////////////////////////////////
	//
	//	Critical Section
	//

	class CriticalSection : public MutexBase
	{
	public:
		CriticalSection()
		{
			InitializeCriticalSection(&m_CriticalSection);
		}
		~CriticalSection()
		{
			DeleteCriticalSection(&m_CriticalSection);
		}

		virtual void Lock()
		{
			EnterCriticalSection(&m_CriticalSection);
		}

		virtual void UnLock()
		{
			LeaveCriticalSection(&m_CriticalSection);
		}

	private:
		CRITICAL_SECTION m_CriticalSection;
	};

	class Mutex : public MutexBase
	{
	public:
		Mutex()
		{
			m_Mutex = CreateMutex(
				NULL,              // default security attributes
				FALSE,             // initially not owned
				NULL);
		}

		~Mutex()
		{
			CloseHandle(m_Mutex);
		}

		virtual void Lock() override
		{
			DWORD dwWaitResult = WaitForSingleObject(
				m_Mutex,    // handle to mutex
				INFINITE);
			if (dwWaitResult != WAIT_OBJECT_0)
			{
				assert(false);
				// error
			}
			else
			{
				// succeeded
				return;
			}
		}

		virtual void UnLock() override
		{
			ReleaseMutex(m_Mutex);
		}

	private:

		HANDLE m_Mutex;
	};


}


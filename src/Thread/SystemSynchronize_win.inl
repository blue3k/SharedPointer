////////////////////////////////////////////////////////////////////////////////
// 
// CopyRight (c) 2013 MadK
// 
// Author : KyungKun Ko
//
// Description : Thread Synchronize
//
////////////////////////////////////////////////////////////////////////////////

#pragma once


#include "windows.h"

namespace BR
{

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


	class Event
	{
	public:

		Event(bool isInitialySet = false, bool autoReset = true)
		{
			m_hEvent = CreateEvent(NULL, !autoReset, isInitialySet, NULL);
		}

		~Event()
		{
			if (m_hEvent)
				CloseHandle(m_hEvent);
		}

		void Reset()
		{
			ResetEvent(m_hEvent);
		}

		void Set()
		{
			SetEvent(m_hEvent);
		}

		bool WaitEvent(UINT uiWaitTimeMs)
		{
			DWORD dwWaitRes = WaitForSingleObject(m_hEvent, uiWaitTimeMs);
			return dwWaitRes == WAIT_OBJECT_0;
		}

	private:
		HANDLE	m_hEvent;
	};

}


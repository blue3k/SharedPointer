
#pragma once


#include <pthread.h>
#include <assert.h>
#include <semaphore.h>


namespace BR
{
	namespace Interlocked
	{

		inline long Increment(long volatile& value)						{ return __sync_add_and_fetch(&value, 1); }
		inline short Increment(short volatile& value)					{ return __sync_add_and_fetch(&value, 1); }
		inline __int64 Increment(__int64 volatile& value)				{ return __sync_add_and_fetch(&value, 1); }

		inline long Decrement(long volatile& value)						{ return __sync_sub_and_fetch(&value, 1); }
		inline short Decrement(short volatile& value)					{ return __sync_sub_and_fetch(&value, 1); }
		inline __int64 Decrement(__int64 volatile& value)				{ return __sync_sub_and_fetch(&value, 1); }


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
			pthread_mutexattr_t mAttr;
			pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE_NP);
			pthread_mutex_init(&m, &mAttr);
		}
		~CriticalSection()
		{
			pthread_mutexattr_destroy(&m_CriticalSection);
		}

		virtual void Lock()
		{
			pthread_mutex_lock(&m_CriticalSection);
		}

		virtual void UnLock()
		{
			pthread_mutex_unlock(&m_CriticalSection);
		}

	private:
		pthread_mutex_t m_CriticalSection;
	};



	class Mutex : public MutexBase
	{
	public:
		Mutex()
		{
			pthread_mutexattr_t mAttr;
			pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE);
			pthread_mutex_init(&m, &mAttr);
		}

		~Mutex()
		{
			pthread_mutexattr_destroy(&m_CriticalSection);
		}

		virtual void Lock() override
		{
			pthread_mutex_lock(&m_CriticalSection);
		}

		virtual void UnLock() override
		{
			pthread_mutex_unlock(&m_CriticalSection);
		}

	private:

		pthread_mutex_t m_CriticalSection;
	};



	class Event
	{
	public:

		Event(bool isInitialySet = false, bool autoReset = true)
			:m_AutoReset(autoReset)
		{
			sem_init(&m_hEvent, 0, isInitialySet ? 1 : 0);
		}

		~Event()
		{
			if (m_hEvent)
				sem_destroy(&m_hEvent);
		}

		void Reset()
		{
			timespec waitTime;
			memset(&waitTime, 0, sizeof(waitTime));

			if (clock_gettime(CLOCK_REALTIME, &waitTime) == -1)
				return;

			waitTime.tv_nsec += 1;
			sem_timedwait(&m_hEvent, &waitTime);
		}

		void Set()
		{
			if (sem_getvalue(&m_hEvent) == 1)
				return;

			sem_post(&m_hEvent);
		}

		bool WaitEvent(UINT uiWaitTimeMs)
		{
			// we need mutex version
			timespec waitTime;
			memset(&waitTime, 0, sizeof(waitTime));

			if (clock_gettime(CLOCK_REALTIME, &waitTime) == -1)
				return false;

			waitTime.tv_sec += uiWaitTimeMs / 1000;
			waitTime.tv_nsec += 1000000 * (uiWaitTimeMs % 1000);
			int waitRes = sem_timedwait(&m_hEvent, &waitTime);
			if (waitRes == ETIMEDOUT)
				return false;
			else if (waitRes == EAGAIN)
				return false;

			if (!m_AutoReset)
				sem_post(&m_hEvent);

			return true;
		}

	private:
		sem_t	m_hEvent;
		bool	m_AutoReset;
	};

}


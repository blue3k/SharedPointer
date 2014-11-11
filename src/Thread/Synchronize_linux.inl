
#pragma once


#include <pthread.h>
#include <assert.h>


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


}


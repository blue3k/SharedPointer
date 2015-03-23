////////////////////////////////////////////////////////////////////////////////
// 
// CopyRight (c) 2013 Blue3k
// 
// Author : KyungKun Ko
//
// Description : Thread utilities
//
////////////////////////////////////////////////////////////////////////////////


#pragma once


#include "Common/BrAssert.h"
#include "Thread/SystemSynchronize.h"


namespace BR
{


	//////////////////////////////////////////////////////////////////////////////////
	//
	//	Basic multithread class
	//
	class Thread
	{
	public:
		typedef enum tag_PRIORITY
		{
			PRIORITY_TIME_CRITICAL	= THREAD_PRIORITY_TIME_CRITICAL,	// Indicates 3 points above normal priority. 
			PRIORITY_HIGHEST		= THREAD_PRIORITY_HIGHEST,			// Indicates 2 points above normal priority. 
			PRIORITY_ABOVE_NORMAL	= THREAD_PRIORITY_ABOVE_NORMAL,		// Indicates 1 point above normal priority. 
			PRIORITY_NORMAL			= THREAD_PRIORITY_NORMAL,			// Indicates normal priority. 
			PRIORITY_BELOW_NORMAL	= THREAD_PRIORITY_BELOW_NORMAL,		// Indicates 1 point below normal priority. 
			PRIORITY_LOWEST			= THREAD_PRIORITY_LOWEST,			// Indicates 2 points below normal priority. 
			PRIORITY_IDLE			= THREAD_PRIORITY_IDLE,				// Indicates 4 points below normal priority. 
		} PRIORITY;

	private:
		// Thread handle
		volatile uintptr_t m_uiThread;

		// Thread ID
		UINT m_uiThreadID;

		// working thread priority
		PRIORITY m_threadPriority;

		// Event handles for thread control
		BR::Event	m_KillEvent;

		// Calculate sleep interval
		ULONG	m_ulPreTime;

		// thread function for run
		static unsigned int __stdcall ThreadFunc( void* arg );

	protected:


	public:
		Thread();
		virtual ~Thread();

		// Get thread
		inline uintptr_t GetThread();
		inline UINT GetThreadID();

		// Get/Set Thread Priority
		virtual void SetPriority( PRIORITY priority );
		inline PRIORITY GetPriority();

		// Get end event handle
		inline BR::Event& GetKillEvent();


		// Calculate sleep interval
		ULONG UpdateInterval( ULONG ulSleepInterval );

		// Check kill event for give wait time
		bool CheckKillEvent( DWORD dwWaitTime );


		// thread Controlling
		virtual void Start();
		virtual void Stop( bool bSendKillEvt = false );
		virtual void Resume();
		virtual void Pause();

		// run process, if return value is false then thread will stop
		virtual bool OnStart() {  return true;}
		virtual bool Run() = 0;
		virtual bool OnEnd() {return true;}
	};


	//////////////////////////////////////////////////////////////////////////////////
	//
	//	Functor execution thread
	//

	class FunctorThread : public Thread
	{
	public: 

	private:
		std::function<void()> m_Functor;

	public:
		FunctorThread( std::function<void()> Functor )
			:m_Functor(Functor)
		{
		}
		virtual bool Run()
		{
			m_Functor();
			return true;
		}
	};

	// Create Functor thread helper function
	inline FunctorThread* CreateFunctorThread( std::function<void()> function )
	{
		return new FunctorThread(function);
	}





#include "Thread.inl"


}; // namespace BR


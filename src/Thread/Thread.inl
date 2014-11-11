////////////////////////////////////////////////////////////////////////////////
// 
// CopyRight (c) 2013 The Braves
// 
// Author : KyungKun Ko
//
// Description : Thread utilities
//
////////////////////////////////////////////////////////////////////////////////



#pragma intrinsic (_InterlockedIncrement, _InterlockedDecrement, _InterlockedCompareExchange, _WriteBarrier, _ReadBarrier)

//////////////////////////////////////////////////////////////////////////////////
//
//	Basic multithread class
//



// Get thread
uintptr_t Thread::GetThread()
{
	return m_uiThread;
}

UINT Thread::GetThreadID()
{
	return m_uiThreadID;
}

// Get/Set Thread Priority
void Thread::SetPriority( PRIORITY priority )
{
	m_threadPriority = priority;
	if( m_uiThread )
		::SetThreadPriority( (HANDLE)m_uiThread, GetPriority() );
}

Thread::PRIORITY Thread::GetPriority()
{
	return m_threadPriority;
}


// Get end event handle
HANDLE Thread::GetKillEvent()
{
	return m_hKillEvent;
}






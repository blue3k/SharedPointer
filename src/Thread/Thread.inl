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

Thread::PRIORITY Thread::GetPriority()
{
	return m_threadPriority;
}


// Get end event handle
Event& Thread::GetKillEvent()
{
	return m_KillEvent;
}






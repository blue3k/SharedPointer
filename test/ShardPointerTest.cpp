// ShardPointerTest.cpp : Defines the entry point for the console application.
//

#include "windows.h"
#include "stdio.h"
#include "tchar.h"
#include "SharedPointer.h"
#include "Thread/Thread.h"


class ThreadGroupOne : BR::Thread
{
	virtual bool Run() override
	{

	}
};

class ThreadGroupTwo : BR::Thread
{
	virtual bool Run() override
	{

	}
};


int _tmain(int argc, _TCHAR* argv[])
{
	//m_uiThread = _beginthreadex(NULL, 0, ThreadFunc, this, 0, &m_uiThreadID);

	return 0;
}

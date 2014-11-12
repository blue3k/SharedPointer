// ShardPointerTest.cpp : Defines the entry point for the console application.
//


#include "Common\Typedefs.h"
#include "SharedPointer.h"
#include "Thread/Thread.h"

#include "windows.h"
#include "stdio.h"
#include "tchar.h"
#include <unordered_map>



class SharedObjectType : public BR::SharedObject
{
public:
	long identity;

	SharedObjectType()
	{

	}

	~SharedObjectType()
	{

	}
};



class ThreadGroupOne : BR::Thread
{
public:
	static std::unordered_map<unsigned, BR::SharedPointerT<SharedObjectType>> m_PointerStorage;

	virtual bool Run() override
	{
		// this thread group can create/delete pointer
	}
};

std::unordered_map<unsigned, BR::SharedPointerT<SharedObjectType>> ThreadGroupOne::m_PointerStorage;


class ThreadGroupTwo : BR::Thread
{
public:
	static std::unordered_map<unsigned, BR::WeakPointerT<SharedObjectType>> m_PointerStorage;

	virtual bool Run() override
	{

	}
};

std::unordered_map<unsigned, BR::WeakPointerT<SharedObjectType>> ThreadGroupTwo::m_PointerStorage;


int _tmain(int argc, _TCHAR* argv[])
{
	

	return 0;
}



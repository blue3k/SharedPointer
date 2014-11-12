// ShardPointerTest.cpp : Defines the entry point for the console application.
//


#include "Common\Typedefs.h"
#include "SharedPointer.h"
#include "Thread/Thread.h"

#include "windows.h"
#include "stdio.h"
#include "tchar.h"
#include <unordered_map>



#define MAX_TEST_ID      50
#define MAX_ACCESS_COUNT 10
#define MAX_GROUP_ONE    5
#define MAX_GROUP_TWO    10

class SharedObjectType : public BR::SharedObject
{
public:
	long Identity;
	long UsedCount;
	long WeakUsedCount;
	long WeakReleasedCount;

	SharedObjectType(long identity)
		:Identity(identity)
		, UsedCount(0)
		, WeakUsedCount(0)
		, WeakReleasedCount(0)
	{
	}

	~SharedObjectType()
	{

	}
};


BR::SharedReferenceManager referenceManager;


// this thread group can create/delete pointer
class ThreadGroupOne : public BR::Thread
{
public:
	static BR::Mutex stm_StorageLock;
	static std::unordered_map<long, BR::SharedPointerT<SharedObjectType>> stm_PointerStorage;

	virtual bool Run() override
	{
		while (1)
		{
			//_CrtCheckMemory();

			BR::SharedPointerT<SharedObjectType> localShared;
			{
				BR::MutexScopeLock scopeLock(stm_StorageLock);
				long randID = rand() % MAX_TEST_ID;
				auto itFound = stm_PointerStorage.find(randID);
				if (itFound == stm_PointerStorage.end())
				{
					localShared = new SharedObjectType(randID);
					referenceManager.RegisterSharedObject(localShared);
					stm_PointerStorage.insert(std::make_pair(randID, localShared));
				}
				else
				{
					localShared = itFound->second;
					auto usedCount = BR::Interlocked::Increment(localShared->UsedCount);
					if (usedCount > MAX_ACCESS_COUNT)
						stm_PointerStorage.erase(itFound);
				}
			}

			// Pretend to use
			Sleep(50);
		}
	}
};

BR::Mutex ThreadGroupOne::stm_StorageLock;
std::unordered_map<long, BR::SharedPointerT<SharedObjectType>> ThreadGroupOne::stm_PointerStorage;



// this thread group can query pointer
class ThreadGroupTwo : public BR::Thread
{
public:
	static BR::Mutex stm_StorageLock;
	static std::unordered_map<long, BR::WeakPointerT<SharedObjectType>> stm_PointerStorage;

	virtual bool Run() override
	{
		while (1)
		{
			BR::WeakPointerT<SharedObjectType> localShared;
			{
				BR::MutexScopeLock scopeLock(stm_StorageLock);
				long randID = rand() % MAX_TEST_ID;
				auto itFound = stm_PointerStorage.find(randID);
				if (itFound == stm_PointerStorage.end())
				{
					BR::MutexScopeLock scopeLock(ThreadGroupOne::stm_StorageLock);
					auto sharedItFound = ThreadGroupOne::stm_PointerStorage.find(randID);
					if (sharedItFound != ThreadGroupOne::stm_PointerStorage.end())
					{
						localShared = sharedItFound->second;
						stm_PointerStorage.insert(std::make_pair(randID, localShared));
						_CrtCheckMemory();
					}
				}
				else
				{
					localShared = itFound->second;
					auto shared = localShared.ToShared();
					if (shared != nullptr)
					{
						auto usedCount = BR::Interlocked::Increment(shared->WeakUsedCount);
						if (usedCount > MAX_ACCESS_COUNT)
						{
							BR::Interlocked::Increment(shared->WeakReleasedCount);
							stm_PointerStorage.erase(itFound);
						}
						_CrtCheckMemory();
					}
				}
			}

			// Pretend to use
			Sleep(10);
		}
	}
};

BR::Mutex ThreadGroupTwo::stm_StorageLock;
std::unordered_map<long, BR::WeakPointerT<SharedObjectType>> ThreadGroupTwo::stm_PointerStorage;


int _tmain(int argc, _TCHAR* argv[])
{
	{
		BR::SharedPointerT<SharedObjectType> sharedPtr;
		BR::SharedPointerT<SharedObjectType> sharedPtr2;
		{
			sharedPtr = new SharedObjectType(1);
			referenceManager.RegisterSharedObject(sharedPtr);
			sharedPtr2 = sharedPtr;
			BR::SharedPointerT<SharedObjectType> sharedPtr3 = sharedPtr;
		}
		{
			BR::SharedPointerT<SharedObjectType> sharedPtr4(sharedPtr);
		}
		{
			BR::SharedPointerT<SharedObjectType> sharedPtr5((SharedObjectType*)sharedPtr);
		}
	}
	referenceManager.FreePendingDelete();

	std::vector<BR::Thread*> Threads;
	for (int iThread = 0; iThread < MAX_GROUP_ONE; iThread++)
	{
		auto thread = new ThreadGroupOne;
		Threads.push_back(thread);
		thread->Start();
	}

	for (int iThread = 0; iThread < MAX_GROUP_TWO; iThread++)
	{
		auto thread = new ThreadGroupTwo;
		Threads.push_back(thread);
		thread->Start();
	}

	while (1)
	{
		referenceManager.FreePendingDelete();
		Sleep(100);
	}

	return 0;
}



// ShardPointerTest.cpp : Defines the entry point for the console application.
//


#include "Common\Typedefs.h"
#include "SharedPointer.h"
#include "Thread/Thread.h"

#include "windows.h"
#include "stdio.h"
#include "tchar.h"
#include <unordered_map>



#define MAX_TEST_ID      1000
#define MAX_ACCESS_COUNT 10

class SharedObjectType : public BR::SharedObject
{
public:
	static long stm_IdentityGen;

	long Identity;
	long UsedCount;
	long WeakUsedCount;

	SharedObjectType(long identity)
		:Identity(identity)
		, UsedCount(0)
		, WeakUsedCount(0)
	{
	}

	~SharedObjectType()
	{

	}
};
long SharedObjectType::stm_IdentityGen = 0;




// this thread group can create/delete pointer
class ThreadGroupOne : BR::Thread
{
public:
	static BR::Mutex stm_StorageLock;
	static std::unordered_map<long, BR::SharedPointerT<SharedObjectType>> stm_PointerStorage;

	virtual bool Run() override
	{
		while (1)
		{
			BR::SharedPointerT<SharedObjectType> localShared;
			{
				BR::MutexScopeLock scopeLock(stm_StorageLock);
				long randID = rand() % MAX_TEST_ID;
				auto itFound = stm_PointerStorage.find(randID);
				if (itFound == stm_PointerStorage.end())
				{
					localShared = new SharedObjectType(randID);
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
class ThreadGroupTwo : BR::Thread
{
public:
	static BR::Mutex stm_StorageLock;
	static std::unordered_map<long, BR::WeakPointerT<SharedObjectType>> stm_PointerStorage;

	virtual bool Run() override
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
					localShared = BR::WeakPointerT<SharedObjectType>((SharedObjectType*)sharedItFound->second);
					stm_PointerStorage.insert(std::make_pair(randID, localShared));
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
						stm_PointerStorage.erase(itFound);
				}
			}
		}

		// Pretend to use
		Sleep(50);
	}
};

BR::Mutex ThreadGroupTwo::stm_StorageLock;
std::unordered_map<long, BR::WeakPointerT<SharedObjectType>> ThreadGroupTwo::stm_PointerStorage;


int _tmain(int argc, _TCHAR* argv[])
{
	

	return 0;
}



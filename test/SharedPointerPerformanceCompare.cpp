////////////////////////////////////////////////////////////////////////////////
// 
// CopyRight (c) 2013 MadK
// 
// Author : KyungKun Ko
//
// Description : Test performance
//
////////////////////////////////////////////////////////////////////////////////


#include "Common\Typedefs.h"
#include "SharedPointer.h"
#include "SharedReferenceManager.h"
#include "Thread/SystemSynchronize.h"
#include "Thread/Thread.h"

#include "windows.h"
#include "stdio.h"
#include "tchar.h"
#include <unordered_map>
#include <memory>

#include <chrono>

#include "SharedPointerTest.h"


template<class TestType, class TestSharedType>
void TestSharedCreate(long long testCount)
{
	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();

	static TestSharedType testVar;
	for (long long iTest = 0; iTest < testCount; iTest++)
	{
		testVar = TestSharedType(new TestType);
		testVar = TestSharedType();
	}

	end = std::chrono::system_clock::now();
	printf("	TestSharedCreate       %10d\n", (end - start).count());
}

template<class TestSharedType>
void TestSharedAssign(long long testCount, TestSharedType shared)
{
	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();

	static TestSharedType testVar;
	for (long long iTest = 0; iTest < testCount; iTest++)
	{
		testVar = shared;
		testVar = TestSharedType();
	}

	end = std::chrono::system_clock::now();
	printf("	TestSharedAssign       %10d\n", (end - start).count());
}

template<class TestSharedType, class TestWeakType>
void TestWeakAssign(long long testCount, TestSharedType shared)
{
	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();

	static TestWeakType weak = shared;
	for (long long iTest = 0; iTest < testCount; iTest++)
	{
		weak = shared;
		weak = TestWeakType();
	}

	end = std::chrono::system_clock::now();
	printf("	TestWeakAssign         %10d\n", (end - start).count());
}

template<class TestSharedType, class TestWeakType>
void TestWeakToSharedAssign(long long testCount, TestSharedType shared, std::function<void(TestWeakType&, TestSharedType&)> ToShared)
{
	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();

	static TestWeakType weak = shared;
	static TestSharedType testShared;
	for (long long iTest = 0; iTest < testCount; iTest++)
	{
		ToShared(weak, testShared);
		testShared = TestSharedType();
	}

	end = std::chrono::system_clock::now();
	printf("	TestWeakToSharedAssign %10d\n", (end - start).count());
}


void SharedPointerPerformanceCompare()
{
	//const long long TestCount = 99;
	const long long TestCount = 9999999;

	// C++ STL shared pointer
	printf("STD shared pointer test\n");

	auto std_shared = std::shared_ptr<SharedObjectType>(new SharedObjectType(1));
	std::weak_ptr<SharedObjectType > std_weak = std_shared;

	std_shared = nullptr;
	std_shared = std_weak.lock();
	std_shared = std::shared_ptr<SharedObjectType>(new SharedObjectType(1));

	TestSharedCreate<SharedObjectType, std::shared_ptr<SharedObjectType>>(TestCount);
	TestSharedAssign(TestCount, std_shared);
	TestWeakAssign<decltype(std_shared), std::weak_ptr<SharedObjectType>>(TestCount, std_shared);
	TestWeakToSharedAssign<decltype(std_shared), std::weak_ptr<SharedObjectType>>(TestCount, std_shared, [](std::weak_ptr<SharedObjectType>& from, decltype(std_shared)& to)
	{
		to = from.lock(); 
	});
	std_shared = nullptr;

	// lock-free shared pointer
	printf("Lock-free shared pointer test\n");

	auto new_shared = BR::SharedPointerT<SharedObjectType>(new SharedObjectType(1));
	TestSharedCreate<SharedObjectType, BR::SharedPointerT<SharedObjectType>>(TestCount);
	TestSharedAssign(TestCount, new_shared);
	TestWeakAssign<decltype(new_shared), BR::WeakPointerT<SharedObjectType>>(TestCount, new_shared);
	TestWeakToSharedAssign<decltype(new_shared), BR::WeakPointerT<SharedObjectType>>(TestCount, new_shared, [](BR::WeakPointerT<SharedObjectType>& from, decltype(new_shared)& to) { from.GetSharedPointer(to); });
}



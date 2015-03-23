////////////////////////////////////////////////////////////////////////////////
// 
// CopyRight (c) 2013 Blue3k
// 
// Author : KyungKun Ko
//
// Description : Test performance
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

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

#define TestScale 6



class SharedObjectType : public BR::SharedObject
{
public:
	long Identity;
	BR::SyncCounter WeakReleasedCount;

	SharedObjectType()
		:Identity(12)
		, WeakReleasedCount(0)
	{
		pItem = new TestData;
		TestID = Identity;
	}

	SharedObjectType(long identity)
		:Identity(identity)
		, WeakReleasedCount(0)
	{
		pItem = new TestData;
		TestID = Identity;
	}

	~SharedObjectType()
	{

	}


	struct TestData {
		BR::SyncCounter UsedCount;
		BR::SyncCounter WeakUsedCount;
	};
	TestData *pItem;

	virtual void Dispose() override
	{
		delete pItem;
		pItem = nullptr;
	}
};



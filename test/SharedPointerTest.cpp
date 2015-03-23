////////////////////////////////////////////////////////////////////////////////
// 
// CopyRight (c) 2013 Blue3k
// 
// Author : KyungKun Ko
//
// Description : Test main
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

#include <chrono>




extern void SharedPointerTest1();
extern void SharedPointerTest2();
extern void SharedPointerPerformanceCompare();


int _tmain(int argc, _TCHAR* argv[])
{
	SharedPointerTest1();

	_CrtDumpMemoryLeaks();

	SharedPointerTest2();

	_CrtDumpMemoryLeaks();

	SharedPointerPerformanceCompare();

	return 0;
}



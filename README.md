Lock-free SharedPointer
=============
Lock-free shared pointer without extra allocation for reference counts. Nowadays, STL shared pointer implementation uses lock-free and it's performance became incrediable.
However, its basic design can't allow to remove extra memory allocation for the counter. It's bad for not only performance but also memory fragmentation.

One more thing I added here is SharedReferenceManager and Dispose. You can do retained delete with SharedReferenceManager. 
All memory deallocation will be happened when you call SharedReferenceManager::UpdateReferenceManager(). However, Dispose will be called when the reference count become zero so that you can do immediate clean-up of important system resources.

Note: 

1. SharedObject itself is thread safe, but SharedPointer and SharedReferenceManager::UpdateReferenceManager() aren't thread safe. You should make sure they are used only on one thread at the same time. 

2. If you use SharedReferenceManager, SharedReferenceManager::UpdateReferenceManager() should be called occasionally such as in tick update, otherwise memory won't be released.

3. Thread and timer implementation only support windows platform.

Now it uses C++11 atomic.

This implementation is tested only on Windows x64 with Visual Studio 2013.



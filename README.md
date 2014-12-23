Lock-free SharedPointer
=============
Lock-free shared pointer without extra allocation for reference counts. Nowadays, STL shared pointer implementation uses lock-free and it's performance became incrediable.
However, its basic design can't allow to remove extra memory allocation for the counter. It's bad for not only performance but also memory fragmentation.

One more thing I added here is SharedReferenceManager. You can do retained delete with SharedReferenceManager. 
All memory deallocation will be happened when you call SharedReferenceManager::UpdateReferenceManager()
Just rememer that SharedReferenceManager::UpdateReferenceManager() should be called occasionally such as in tick update, and it's not thread safe.

Now it uses C++11 atomic.

This implementation is tested only on Windows x64 with Visual Studio 2013.



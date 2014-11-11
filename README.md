SharedPointer
=============

!!!UNDERCONSTRUCTION!!!

I was wondering about performance of sharder pointer implementation in STL. Shared Pointer is such a great idea so that it can help many other programmers.
However, it also has significant down side with performance. To make it works in multithread environment, STL shared pointer uses mutex lock/unlock for reference counting, and memory allocations for shared reference menagement.

Shard pointer without OS level critical section such as mutex and critical section.


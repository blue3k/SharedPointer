Lock-free SharedPointer
=============
Lock-free shared pointer without OS level critical section such as mutex and critical section.

Now it uses C++11 atomic

This implementation is tested only on Windows x64 with Visual Studio 2013.

I wanted to get rid of performance lose of STD shared_ptr implemention, and this is the result of it.
I use lock-free instructions to avoid system kernel switching. I have to use several counter and state variable so it will consume more memory than other implementation.
However, by sacrificing more memory, it will give you faster shared pointer implementation.

NOTE: SharedReferenceManager::UpdateReferenceManager() should be called frequently. It's the one who releases memory. One more thing you need to care is it's not thread safe.
You should make it sure that only one thread calls it at the same time.



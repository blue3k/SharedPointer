////////////////////////////////////////////////////////////////////////////////
// 
// CopyRight (c) 2013 MadK
// 
// Author : KyungKun Ko
//
// Description : Thread Synchronize
//
////////////////////////////////////////////////////////////////////////////////



#pragma once

#include "Common/Typedefs.h"
#include "Common/BrAssert.h"
//#include "Common/Memory.h"
//#include "Common/Utility.h"


#if defined(WIN32) || defined(WIN64)
#include "Intrinsic_win.inl"
#else
#include "Intrinsic_linux.inl"
#endif


namespace BR
{


	//////////////////////////////////////////////////////////////////////////////////
	//
	//	Synchronization trait
	//

	// Synchronization trait, not thread safe
	struct ThreadSyncTraitNone
	{
	};

	// Synchronization trait, thread safe
	struct ThreadSyncTraitMT
	{
		bool ThreadSafe;
	};



	//////////////////////////////////////////////////////////////////////////////////
	//
	//	Sync counter class
	//

	class SyncCounter
	{
	public:
		volatile CounterType	m_Counter;

		SyncCounter() : m_Counter(0) {}
		SyncCounter( CounterType src ) : m_Counter(src) {}
		~SyncCounter(){}

		operator CounterType() volatile const
		{
			return m_Counter;
		}

		CounterType operator = ( const SyncCounter& src ) volatile
		{
			m_Counter = src.m_Counter;
			return m_Counter;
		}

		// increment
		inline CounterType Increment() volatile;

		// decrement
		inline CounterType Decrement() volatile;
	};



	// Scope counter
	class ScopeCounter
	{
	public:
		ScopeCounter(CounterType& counterInstance) : m_CounterInstance(counterInstance)
		{
			Interlocked::Increment(m_CounterInstance);
		}

		~ScopeCounter()
		{
			Interlocked::Decrement(m_CounterInstance);
		}

	private:
		CounterType &m_CounterInstance;
	};

	
	//////////////////////////////////////////////////////////////////////////////////
	//
	//	Spinlock class
	//
	
	class SpinLock
	{
	public:
		// Lock state
		enum {
			STATE_FREE,
			STATE_LOCKED,
		} LOCK_STATE;
	private:

		// Lock state
		volatile LONG m_LockValue; 
   
	public:
		SpinLock() : m_LockValue(STATE_FREE) { } 
		~SpinLock() {}

	public:
		// Lock
		inline void Lock();
	
		// Unlock
		inline void UnLock();

		// Try lock bit a while
		inline bool TryLock(int iTryCount = 100);

		inline LONG	GetLockCount() { return m_LockValue; }
	};




	//////////////////////////////////////////////////////////////////////////////////
	//
	//	Scopelock class
	//

	class ScopeLock
	{
	private:
		// Spin lock to controllered by scope
		SpinLock& m_SpinLock;


	public:
		// Constructor/destructor
		ScopeLock(SpinLock& nLock)
			:m_SpinLock(nLock)
		{
			m_SpinLock.Lock();
		}
	
		~ScopeLock()
		{
			m_SpinLock.UnLock();
		}
	};




	//////////////////////////////////////////////////////////////////////////////////
	//
	//	Ticketing class
	//

	class Ticketing
	{
	public:
		typedef CounterType Ticket;

		// working thread count...until now
		SyncCounter m_Working;

		// woking complete thread count..
		SyncCounter m_Worked;

		Ticketing() {}
		~Ticketing() {}
		

		// Reset ticket
		inline void Reset();


		// getting a ticket
		inline Ticket AcquireTicket();

		// using done the ticket
		inline Ticket ReleaseTicket();

		// getting my waiting order
		inline Ticket GetMyWaitingOrder(Ticket) const;

		// getting total waiting thread count
		Ticket GetTotalWaitingCount() const;

		// getting working thread count...until now;
		inline Ticket GetNowWorkingCount() const;

		// getting worked thread count...until now;
		inline Ticket GetWorkingCompleteCount() const;
	};



	//////////////////////////////////////////////////////////////////////////////////
	//
	//	Ticket Lock Class with mode support
	//

	class TicketLock
	{
	public:
		enum LockMode 
		{ 
			LOCK_FREE = 0, 
			LOCK_NONEXCLUSIVE,
			LOCK_EXCLUSIVE,
		};

	private:
		// read lock count
		volatile ULONG	m_OpMode;
		Ticketing		m_Ticketing;
		volatile SyncCounter m_NonExclusiveCount;

	public:
		inline TicketLock();
		inline ~TicketLock();

		// Exclusive lock/unlock
		inline void ExLock();
		inline void ExUnlock();

		// Non-Exclusive lock/unlock
		inline void NonExLock();
		inline void NonExUnlock();

		// Query status
		inline CounterType GetTicketCount() const;
		inline CounterType GetNonExclusiveCount() const;
		inline bool	IsLocked() const;
	};

	
	class FakeLock
	{
	public:
	private:

	public:
		FakeLock(){}
		~FakeLock(){}

		// Exclusive lock/unlock
		inline void ExLock() {}
		inline void ExUnlock() {}

		// Non-Exclusive lock/unlock
		inline void NonExLock() {}
		inline void NonExUnlock() {}

		// Query status
		inline CounterType GetTicketCount() const {return 0;}
		inline CounterType GetNonExclusiveCount() const {return 0;}
		inline bool	IsLocked() const {return false;}
	};





	//////////////////////////////////////////////////////////////////////////////////
	//
	//	Scope TicketLock Class
	//

	template< class TicketLockType >
	class TicketScopeLockT
	{
	private:
		TicketLock::LockMode	m_LockMode;
		TicketLockType &m_TicketLock;

	public:

		TicketScopeLockT( TicketLock::LockMode lockMode, TicketLockType &ticketLock );	
		~TicketScopeLockT();

	};

	typedef TicketScopeLockT<TicketLock> TicketScopeLock;



	extern template class TicketScopeLockT<TicketLock>;
	extern template class TicketScopeLockT<FakeLock>;



#include "Synchronize.inl"


}; // namespace BR


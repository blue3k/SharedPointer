////////////////////////////////////////////////////////////////////////////////
// 
// CopyRight (c) 2013 MadK
// 
// Author : KyungKun Ko
//
// Description : Thread Synchronize
//
////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////
//
//	Sync counter class
//

// increment
CounterType SyncCounter::Increment() volatile
{
	return Interlocked::Increment(m_Counter);
}

// decrement
CounterType SyncCounter::Decrement() volatile
{
	return Interlocked::Decrement(m_Counter);
}



//////////////////////////////////////////////////////////////////////////////////
//
//	Spinlock class
//
	

// Lock
void SpinLock::Lock()
{
	int iLockTry = 0;
	while(_InterlockedCompareExchange(&m_LockValue, STATE_LOCKED, STATE_FREE) != STATE_FREE) // lock value가 1이면 진입
	{ 
		iLockTry++;
		if( iLockTry%5 )
		{
			Sleep(0);
		}
	}
}

// Unlock
void SpinLock::UnLock()
{
	m_LockValue = STATE_FREE;
	_WriteBarrier();
}

// Try lock bit a while
bool SpinLock::TryLock( int iTryCount )
{
	int iLockTry = 0;
	//while(_InterlockedCompareExchange(&m_LockValue, STATE_LOCKED, STATE_FREE) != STATE_FREE) // lock value가 1이면 진입
	while ( !Interlocked::CompareExchange(m_LockValue, STATE_LOCKED, STATE_FREE) ) // lock value가 1이면 진입
	{ 
		iLockTry++;

		if( iLockTry > iTryCount )
			return false;

		if( iLockTry%5 )
		{
			Sleep(0);
		}
	}

	return true;
}

// Reset ticket
void Ticketing::Reset()
{
	m_Working = m_Worked = 0;
}

// Ticketing
Ticketing::Ticket Ticketing::AcquireTicket()
{
	return m_Working.Increment();
}

Ticketing::Ticket Ticketing::ReleaseTicket()
{
	return m_Worked.Increment();
}

Ticketing::Ticket Ticketing::GetMyWaitingOrder(Ticket myTicket) const
{
	SignedCounterType Diff = (SignedCounterType)(myTicket - m_Worked.m_Counter);
	if( Diff < 0 ) Diff = 0;
	return (Ticket)Diff;
}

Ticketing::Ticket Ticketing::GetNowWorkingCount() const
{
	return m_Working.m_Counter;
}

Ticketing::Ticket Ticketing::GetWorkingCompleteCount() const
{
	return m_Worked.m_Counter;
}






//////////////////////////////////////////////////////////////////////////////////
//
//	Ticket Lock Class
//

TicketLock::TicketLock()
	:m_NonExclusiveCount(0),
	m_OpMode(LOCK_FREE)
{
}

TicketLock::~TicketLock()
{
}

// Exclusive lock/unlock
void TicketLock::ExLock()
{
	Ticketing::Ticket myTicket = m_Ticketing.AcquireTicket();

	// Wait my ticket
	CounterType WaitOrder;
	while( (WaitOrder = m_Ticketing.GetMyWaitingOrder(myTicket)) > 1 )
	{
		if( WaitOrder > 4 )
			Sleep(0);
	}

	// flush non exclusive lock
	while( m_NonExclusiveCount > 0 )
	{
		Sleep(0);
	}

	m_OpMode = LOCK_EXCLUSIVE;
}

void TicketLock::ExUnlock()
{
	//AssertRel( m_OpMode == LOCK_EXCLUSIVE );
	m_OpMode = LOCK_FREE;
	m_Ticketing.ReleaseTicket();
}

// Non-Exclusive lock/unlock
void TicketLock::NonExLock()
{
	Ticketing::Ticket myTicket = m_Ticketing.AcquireTicket();

	// Wait my ticket
	CounterType WaitOrder;
	while( (WaitOrder = m_Ticketing.GetMyWaitingOrder(myTicket)) > 1 )
	{
		if( WaitOrder > 4 )
			Sleep(0);
	}

	//// wait if Write mode, will not occure
	//while( m_OpMode == LOCK_EXCLUSIVE )
	//{
	//	Sleep(0);
	//}
	AssertRel( m_OpMode != LOCK_EXCLUSIVE );

	m_OpMode = LOCK_NONEXCLUSIVE;
	SignedCounterType count = (SignedCounterType)m_NonExclusiveCount.Increment();
	AssertRel(count > 0 );
	m_Ticketing.ReleaseTicket();
}

void TicketLock::NonExUnlock()
{
	SignedCounterType count = (SignedCounterType)m_NonExclusiveCount.Decrement();
	AssertRel(count >= 0 );
}

// Query status
CounterType TicketLock::GetTicketCount() const
{
	return m_Ticketing.GetTotalWaitingCount();
}

CounterType TicketLock::GetNonExclusiveCount() const
{
	return m_NonExclusiveCount;
}

bool TicketLock::IsLocked() const
{
	return !( (m_Ticketing.GetTotalWaitingCount()) == 0 && m_NonExclusiveCount == 0 );
}




//////////////////////////////////////////////////////////////////////////////////
//
//	Scope TicketLock Class
//

template< class TicketLockType >
TicketScopeLockT<TicketLockType>::TicketScopeLockT( TicketLock::LockMode lockMode, TicketLockType &ticketLock )
:m_LockMode(lockMode),
m_TicketLock(ticketLock)
{
	Assert( m_LockMode == TicketLock::LOCK_EXCLUSIVE || m_LockMode == TicketLock::LOCK_NONEXCLUSIVE );
	if( m_LockMode == LOCK_EXCLUSIVE )
		m_TicketLock.ExLock();
	else
		m_TicketLock.NonExLock();
}

template< class TicketLockType >
TicketScopeLockT<TicketLockType>::~TicketScopeLockT()
{
	if( m_LockMode == LOCK_EXCLUSIVE )
		m_TicketLock.ExUnlock();
	else
		m_TicketLock.NonExUnlock();
}

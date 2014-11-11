////////////////////////////////////////////////////////////////////////////////
// 
// CopyRight (c) 2013 The Braves
// 
// Author : KyungKun Ko
//
// Description : Time Utility library
//	
//
////////////////////////////////////////////////////////////////////////////////



TimeStampTimer::TimeStampTimer()
	:m_ulTimeToExpire(0)
{
}

TimeStampTimer::~TimeStampTimer()
{
}

// Set timer delegate
void	TimeStampTimer::SetTimerFunc( std::function<void()> funcOnExpired )
{
	m_delOnExpired = funcOnExpired;
}

// clear timer
void	TimeStampTimer::ClearTimer()
{
	m_ulTimeToExpire = 0;
}

// check about timer is working
bool	TimeStampTimer::IsTimerWorking() const
{
	return m_ulTimeToExpire != 0;
}

// Timer check update
bool	TimeStampTimer::CheckTimer()
{
	bool bExpired = m_ulTimeToExpire != 0 && (LONG)(m_ulTimeToExpire - Time.GetTimeMs()) < 0;

	if( bExpired )
	{
		if( m_delOnExpired )
			m_delOnExpired();

		// Clear timer
		m_ulTimeToExpire = 0;
	}

	return bExpired;
}

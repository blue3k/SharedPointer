
#pragma once


#include "intrin.h"
#include "windows.h"

#ifdef WIN64
#pragma intrinsic (_InterlockedIncrement, _InterlockedDecrement, _InterlockedCompareExchangePointer, _InterlockedIncrement64, _InterlockedDecrement64, _InterlockedCompareExchange, _InterlockedCompareExchange64, _InterlockedCompareExchange128, _WriteBarrier, _ReadBarrier)
#else
#pragma intrinsic (_InterlockedIncrement, _InterlockedDecrement, _InterlockedCompareExchangePointer, _InterlockedIncrement64, _InterlockedDecrement64, _InterlockedCompareExchange, _InterlockedCompareExchange64, _InterlockedCompareExchange128, _WriteBarrier, _ReadBarrier)
#endif


namespace BR
{
	namespace Interlocked
	{
		typedef ::CounterType CounterType;

		inline long Increment(volatile long& value)									{ return _InterlockedIncrement(&value); }
		inline unsigned long Increment(volatile unsigned long& value)				{ return _InterlockedIncrement(&value); }
		inline short Increment(volatile short& value)								{ return _InterlockedIncrement16(&value); }
		inline unsigned short Increment(volatile unsigned short& value)				{ return _InterlockedIncrement16((short*)&value); }
		inline __int64 Increment(volatile __int64& value)							{ return _InterlockedIncrement64(&value); }
		inline unsigned __int64 Increment(volatile unsigned __int64& value)			{ return _InterlockedIncrement64((__int64*)&value); }
//		inline CounterType Increment(volatile CounterType& value)					{ return (CounterType)_InterlockedIncrement64((volatile __int64*)&value); }

		inline long Decrement(volatile long& value)									{ return _InterlockedDecrement(&value); }
		inline unsigned long Decrement(volatile unsigned long& value)				{ return _InterlockedDecrement((long*)&value); }
		inline short Decrement(volatile short& value)								{ return _InterlockedDecrement16(&value); }
		inline unsigned short Decrement(volatile unsigned short& value)				{ return _InterlockedDecrement16((short*)&value); }
		inline __int64 Decrement(volatile __int64& value)							{ return _InterlockedDecrement64(&value); }
		inline unsigned __int64 Decrement(volatile unsigned __int64& value)			{ return (unsigned __int64)_InterlockedDecrement64((volatile __int64*)& value); }


		inline bool CompareExchange(volatile long& dest, long value, long compare)
		{
			return _InterlockedCompareExchange(&dest, value, compare) == compare;
		}

		inline bool CompareExchange(volatile __int64& dest, __int64 value, __int64 compare)
		{
			return _InterlockedCompareExchange64(&dest, value, compare) == compare;
		}

		inline bool CompareExchange(volatile __int64* dest, const __int64* value, const __int64* compare)
		{
			__int64 compareTemp[2] = { compare[0], compare[1] };
			return _InterlockedCompareExchange128(dest, value[1], value[0], compareTemp) != FALSE;
		}


		inline void ReadBarrier()													{ _ReadBarrier(); }
		inline void WriteBarrier()													{ _WriteBarrier(); }

	}



}


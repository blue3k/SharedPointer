
#pragma once


#include <pthread.h>
#include <assert.h>
#include <semaphore.h>


namespace BR
{
	namespace Interlocked
	{

		inline long Increment(volatile long& value)						{ return __sync_add_and_fetch(&value, 1); }
		inline short Increment(volatile short& value)					{ return __sync_add_and_fetch(&value, 1); }
		inline __int64 Increment(volatile __int64& value)				{ return __sync_add_and_fetch(&value, 1); }

		inline long Decrement(volatile long& value)						{ return __sync_sub_and_fetch(&value, 1); }
		inline short Decrement(volatile short& value)					{ return __sync_sub_and_fetch(&value, 1); }
		inline __int64 Decrement(volatile __int64& value)				{ return __sync_sub_and_fetch(&value, 1); }

		inline bool CompareExchange(volatile long& dest, long value, long compare)
		{
			return __sync_bool_compare_and_swap(&dest, compare, value);
		}

		inline bool CompareExchange(volatile __int64& dest, __int64 value, __int64 compare)
		{
			return __sync_bool_compare_and_swap(&dest, compare, value);
		}

		inline bool CompareExchange(volatile __int64* dest, const __int64* value, const __int64* compare)
		{
			return __sync_bool_compare_and_swap(&dest, compare, value);
		}


		typedef ::CounterType CounterType;


	}




}


////////////////////////////////////////////////////////////////////////////////
// 
// CopyRight (c) 2013 The Braves
// 
// Author : MadK
//
// Description : Debug definitions
//	
//
////////////////////////////////////////////////////////////////////////////////


#pragma once

#include "Typedefs.h"



#ifdef _DEBUG

// Base Assert
#define Assert(expr) _ASSERT(expr)

// Assert even release mode
#define AssertRel(expr) _ASSERT(expr)


#else // #ifdef _DEBUG

#define Assert(condi)

// Assert even release mode
#define AssertRel(condi) \
				do{ \
					if( !(condi) ) {\
						__debugbreak(); \
					}else{}\
				}while(0); \


#endif





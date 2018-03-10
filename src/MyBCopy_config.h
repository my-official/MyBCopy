#pragma once


//#define MyBCopy_SHARED_LIB




#ifndef MyBCopy_API
#ifdef MyBCopy_SHARED_LIB
#ifdef MyBCopy_EXPORT
#define MyBCopy_API __declspec(dllexport)
#else
#define MyBCopy_API __declspec(dllimport)
#endif
#else		
#define MyBCopy_API		
#endif
#endif


#define MyUtils_API MyBCopy_API
#define MyException_API MyBCopy_API

#ifndef _COMMON_H_
#define _COMMON_H_

typedef unsigned int uint;
typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef int int32;

#ifdef __GNUC__
typedef long long int64;
typedef unsigned long long uint64;
#else
typedef __int64 int64;
typedef unsigned __int64 uint64;
#endif

#endif

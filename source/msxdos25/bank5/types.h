#ifndef __TYPES_H
#define __TYPES_H

#ifndef uint
typedef unsigned int uint;
#endif

#ifndef byte
typedef unsigned char byte;
#endif

#ifndef ulong
typedef unsigned long ulong;
#endif

#ifndef null
#define null ((void*)0)
#endif

typedef unsigned char bool;
#define false (0)
#define true (!(false))

#endif   //__TYPES_H
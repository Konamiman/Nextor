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

#ifndef bool
typedef unsigned char bool;
#endif

#ifndef false
#define false (0)
#endif

#ifndef true
#define true (!(false))
#endif

#ifndef null
#define null ((void*)0)
#endif

#endif   //__TYPES_H

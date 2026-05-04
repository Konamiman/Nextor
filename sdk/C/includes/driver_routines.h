/*
 * Driver header signature and jump table.
 *
 * Every Nextor v3 driver image is paged in at 4100h-7FFFh (the 4000h-40FFh
 * area is reserved and filled with zeros). The addresses below are where
 * each header field lives while the driver is paged in. The signature
 * must be the fixed zero-terminated string "NEXTORv3_DRIVER".
 */

#ifndef __DRIVER_ROUTINES_H
#define __DRIVER_ROUTINES_H

#define DRIVER_SIGNATURE                  0x4100  /* 16 bytes: "NEXTORv3_DRIVER",0 */
#define DRIVER_TIMI_ENTRY                 0x4110  /* jp <handler>  (timer interrupt) */
#define DRIVER_BASSTAT_ENTRY              0x4113  /* jp <handler>  (BASIC statement) */
#define DRIVER_BASDEV_ENTRY               0x4116  /* jp <handler>  (BASIC device) */
#define DRIVER_EXTBIO_ENTRY               0x4119  /* jp <handler>  (EXTBIO hook) */
#define DRIVER_DIRECT0_ENTRY              0x411C
#define DRIVER_DIRECT1_ENTRY              0x411F
#define DRIVER_DIRECT2_ENTRY              0x4122
#define DRIVER_DIRECT3_ENTRY              0x4125
#define DRIVER_DIRECT4_ENTRY              0x4128
#define DRIVER_DRIVER_QUERY_ENTRY         0x412B
#define DRIVER_DEVICE_QUERY_ENTRY         0x412E
#define DRIVER_CUSTOM_DRIVER_QUERY_ENTRY  0x4131
#define DRIVER_CUSTOM_DEVICE_QUERY_ENTRY  0x4134
#define DRIVER_READ_WRITE_ENTRY           0x4137
#define DRIVER_HEADER_END                 0x413A  /* First address after the header */

#endif   //__DRIVER_ROUTINES_H

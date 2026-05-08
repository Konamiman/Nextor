/*
 * Result codes returned in register A by the DRIVER_QUERY
 * and DEVICE_QUERY driver routines.
 */

#ifndef __DRIVER_RESULT_CODES_H
#define __DRIVER_RESULT_CODES_H

#define DRIVER_RESULT_OK                  0     /* Subfunction completed successfully */
#define DRIVER_RESULT_TRUNCATED_STRING    1     /* Returned string was truncated */
#define DRIVER_RESULT_INVALID_DEVICE      2     /* Specified device is invalid */
#define DRIVER_RESULT_INIT_ERROR          3     /* Driver initialization failed */
#define DRIVER_RESULT_NOT_IMPLEMENTED  0xFF     /* Subfunction not implemented
                                                 * (treated as success with default values) */

#endif   //__DRIVER_RESULT_CODES_H

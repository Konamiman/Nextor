/*
 * Subfunction codes for the DEVICE_QUERY driver routine.
 */

#ifndef __DRIVER_DEVICE_QUERIES_H
#define __DRIVER_DEVICE_QUERIES_H

/* Subfunction codes (passed in register A). */

#define DEVICE_QUERY_GET_STRING         1  /* Get device information string */
#define DEVICE_QUERY_GET_PARAMS         2  /* Get device parameters */
#define DEVICE_QUERY_GET_STATUS         3  /* Get device status */
#define DEVICE_QUERY_GET_AVAILABILITY   4  /* Get device availability */
#define DEVICE_QUERY_GET_FORMAT_CHOICES 5  /* Get format choices */
#define DEVICE_QUERY_DO_FORMAT          6  /* Format the device */
#define DEVICE_QUERY_STOP_MOTOR         7  /* Stop the device motor */


/* Sub-string codes for the GET_STRING query (passed in register B).
 * No asm-side counterpart yet - currently C-only. */

#define STRING_MEDIUM_NAME 2
#define STRING_DEVICE_NAME 4

#endif   //__DRIVER_DEVICE_QUERIES_H

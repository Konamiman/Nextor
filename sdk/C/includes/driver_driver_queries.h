/*
 * Subfunction codes for the DRIVER_QUERY driver routine.
 */

#ifndef __DRIVER_DRIVER_QUERIES_H
#define __DRIVER_DRIVER_QUERIES_H

/* Subfunction codes (passed in register A). */

#define DRIVER_QUERY_GET_VERSION         1  /* Get driver version */
#define DRIVER_QUERY_GET_STRING          2  /* Get driver string (see STR_* below) */
#define DRIVER_QUERY_GET_INIT_PARAMS_ROM 3  /* Get ROM-driver initialization parameters */
#define DRIVER_QUERY_INIT_ROM            4  /* Initialize a ROM-loaded driver */
#define DRIVER_QUERY_GET_MAX_DEVICE      5  /* Get maximum device number */
#define DRIVER_QUERY_INIT_RAM            6  /* Initialize a RAM-loaded driver */
#define DRIVER_QUERY_SHUTDOWN_RAM        7  /* Shut down a RAM-loaded driver */


/* Sub-string codes for the GET_STRING query (passed in register B). */

#define DRIVER_QUERY_STR_DRIVER_NAME     1  /* Driver name string */
#define DRIVER_QUERY_STR_DRIVER_AUTHOR   2  /* Driver author name */
#define DRIVER_QUERY_STR_HARDWARE_NAME   3  /* Hardware name */
#define DRIVER_QUERY_STR_HARDWARE_AUTHOR 4  /* Hardware author name */
#define DRIVER_QUERY_STR_SERIAL_NUMBER   5  /* Serial number */


/* Input flags for GET_INIT_PARAMS_ROM and INIT_ROM (passed in register C).
 * Both queries receive the same flag set; the kernel passes the user's
 * boot choices through to the driver so it can adapt its initialization. */

/* Bit 5: set if the user is requesting a reduced drive count
 * (e.g. by holding the 5 key during boot). */
#define DRIVER_QUERY_INFLAG_REDUCED_DRIVES   5
#define DRIVER_QUERY_INFLAGM_REDUCED_DRIVES  (1 << DRIVER_QUERY_INFLAG_REDUCED_DRIVES)


/* Output flags from GET_INIT_PARAMS_ROM (returned in register B).
 * The driver tells the kernel which system hooks it wants installed. */

/* Bit 0: set if the kernel should call the driver's TIMI entry
 * on every timer interrupt. */
#define DRIVER_QUERY_OUTFLAG_HOOK_TIMER   0
#define DRIVER_QUERY_OUTFLAGM_HOOK_TIMER  (1 << DRIVER_QUERY_OUTFLAG_HOOK_TIMER)

/* Bit 1: set if the kernel should chain the driver into the EXTBIO hook. */
#define DRIVER_QUERY_OUTFLAG_HOOK_EXTBIO  1
#define DRIVER_QUERY_OUTFLAGM_HOOK_EXTBIO (1 << DRIVER_QUERY_OUTFLAG_HOOK_EXTBIO)

#endif   //__DRIVER_DRIVER_QUERIES_H

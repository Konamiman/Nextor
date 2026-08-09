;-----------------------------------------------------------------------------
;
; Subfunction codes for the DRIVER_QUERY driver routine.
;
; Includers should wrap this file in `module DRIVER_QUERY ... endmod` so the
; constants are accessed externally as `DRIVER_QUERY.NAME` (avoiding
; collisions with the device-level query codes that share short names like
; GET_STRING).
;
;-----------------------------------------------------------------------------

	include ../macros/const.inc

	const	GET_VERSION,         1	;Get driver version
	const	GET_STRING,          2	;Get driver string
	const	GET_INIT_PARAMS_ROM, 3	;Get ROM driver initialization parameters
	const	INIT_ROM,            4	;Initialize a ROM-loaded driver
	const	GET_MAX_DEVICE,      5	;Get maximum device number
	const	INIT_RAM,            6	;Initialize a RAM-loaded driver
	const	SHUTDOWN_RAM,        7	;Shut down a RAM-loaded driver

;Sub-string codes for the GET_STRING query (in register B).

	const	STR_DRIVER_NAME,     1	;Driver name string
	const	STR_DRIVER_AUTHOR,   2	;Driver author name
	const	STR_HARDWARE_NAME,   3	;Hardware name
	const	STR_HARDWARE_AUTHOR, 4	;Hardware author name
	const	STR_SERIAL_NUMBER,   5	;Serial number

;-----------------------------------------------------------------------------
;
; Input flags for GET_INIT_PARAMS_ROM and INIT_ROM (in register C).
; Both queries receive the same flag set; the kernel passes the user's boot
; choices through to the driver so it can adapt its initialization.
;

;Bit 5: set if the user is requesting a reduced drive count (e.g. by holding
;the 5 key during boot).
	const	INFLAG_REDUCED_DRIVES,  5
	const	INFLAGM_REDUCED_DRIVES, 1 SHL INFLAG_REDUCED_DRIVES

;-----------------------------------------------------------------------------
;
; Output flags from GET_INIT_PARAMS_ROM (returned in register B).
; The driver tells the kernel which system hooks it wants installed.
;

;Bit 0: set if the kernel should call the driver's TIMI entry on every timer
;interrupt.
	const	OUTFLAG_HOOK_TIMER,  0
	const	OUTFLAGM_HOOK_TIMER, 1 SHL OUTFLAG_HOOK_TIMER

;Bit 1: set if the kernel should chain the driver into the EXTBIO hook.
	const	OUTFLAG_HOOK_EXTBIO,  1
	const	OUTFLAGM_HOOK_EXTBIO, 1 SHL OUTFLAG_HOOK_EXTBIO

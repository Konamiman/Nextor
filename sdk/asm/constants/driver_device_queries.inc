;-----------------------------------------------------------------------------
;
; Subfunction codes for the DEVICE_QUERY driver routine.
;
; Includers should wrap this file in `module DEVICE_QUERY ... endmod` so the
; constants are accessed externally as `DEVICE_QUERY.NAME` (avoiding
; collisions with the driver-level query codes that share short names like
; GET_STRING).
;
;-----------------------------------------------------------------------------

	include ../macros/const.inc

	const	GET_STRING,         1	;Get device information string
	const	GET_PARAMS,         2	;Get device parameters
	const	GET_STATUS,         3	;Get device status
	const	GET_AVAILABILITY,   4	;Get device availability
	const	GET_FORMAT_CHOICES, 5	;Get format choices
	const	DO_FORMAT,          6	;Format the device
	const	STOP_MOTOR,         7	;Stop the device motor

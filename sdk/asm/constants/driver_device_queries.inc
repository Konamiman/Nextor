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
	const	READ_BEFORE_INIT,   8	;Read device sectors before the driver is initialized

;Bits of the flags byte (at +7) returned by the GET_PARAMS query.

	const	PARAMF_REMOVABLE,   0	;The device is removable
	const	PARAMF_READ_ONLY,   1	;The device is read only
	const	PARAMF_FLOPPY,      2	;The device is a floppy disk drive
	const	PARAMF_NO_AUTOMAP,  3	;Don't use the device for partition automapping
	const	PARAMF_NO_PSTOR,    4	;Don't use the device for persistent storage

;Sub-string codes for the GET_STRING query (in register B).

	const	STR_MANUFACTURER,   1	;Manufacturer name
	const	STR_MEDIUM_NAME,    2	;Medium name (obtained from the medium itself)
	const	STR_SERIAL_NUMBER,  3	;Serial number
	const	STR_DEVICE_NAME,    4	;Device name (provided by the driver)

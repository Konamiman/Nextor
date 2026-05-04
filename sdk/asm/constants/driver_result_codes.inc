	.z80

	name('driver_result_codes')
	title	Nextor SDK - Public constants for query routine return codes

	.sall

;-----------------------------------------------------------------------------
;
; Result codes returned by the DRIVER/DEVICE_QUERY driver routines (in register A).
;
;-----------------------------------------------------------------------------

	include ../macros/const.inc

	const	RESULT_OK,                0	;Subfunction completed successfully
	const	RESULT_TRUNCATED_STRING,  1	;Returned string was truncated
	const	RESULT_INVALID_DEVICE,    2	;Specified device is invalid
	const	RESULT_INIT_ERROR,        3	;Driver initialization failed
	const	RESULT_NOT_IMPLEMENTED, 0FFh	;Subfunction not implemented
						; (treated as success with default values)

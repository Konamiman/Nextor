//-----------------------------------------------------------------------------	
// 
// Create the kernel recovery file.
// It's aheader that acts as an identifier and tells the size of the kernel.
// This file has to be saved in the SD card after formatting it,
// then it's loaded from the recovery menu.
//
// Manuel Pazos 13/01/2013
//
//-----------------------------------------------------------------------------	

		;----------------
	db	"MFRSD KERNEL 1.0"
	db	(end-start) / (8*1024)
	ds	512-$,#ff
start:
	incbin	"nextor2.rom"
	;incbin	"nextor.dsk"
end:
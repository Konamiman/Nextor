//-----------------------------------------------------------------------------	
// 
// Crea el fichero de recuperacion de la kernel
// Se trata de una cabecera que sirve de identificador e indica el tamaño de
// la kernel.
// Hay que grabar este fichero en la tarjeta SD tras formatearla.
// Luego se carga desde el menu recovery
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
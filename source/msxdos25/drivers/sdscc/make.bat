mknexrom ..\..\..\bin\dos250ba.dat nextor2.rom /d:driver.bin /m:Mapper.ASCII8.bin
sjasm makerecoverykernel.asm kernel.dat
copy kernel.dat ..\..\..\bin\DOS250.SDSCC.ROM



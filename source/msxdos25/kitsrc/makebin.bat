l80 /p:4000,dos2kit,/p:7405,drv,/p:7fd0,chgbnk2,b0/n/x/e
l80 /p:4000,/d:f237,dos1kit,drv,/p:7fd0,chgbnk1,b3/n:p/x/e

sbug /eb0.hex;r;eb0.bin;w4000,7fff;g0
sbug /eb0.hex;r;f4100,7fcf,ff;eb1.hex;r;eb1.bin;w4000,7fff;g0
sbug /eb0.hex;r;f4100,7fcf,ff;eb2.hex;r;eb2.bin;w4000,7fff;g0
sbug /eb3.hex;r;eb3.bin;w4000,7fff;g0

concat /b b0.bin+b1.bin+b2.bin+b3.bin dos2rom.bin
del b?.bin

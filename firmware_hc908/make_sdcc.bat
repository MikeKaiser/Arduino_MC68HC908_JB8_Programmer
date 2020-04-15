@echo off
if exist main.lk (
    rem link file already exists, use it
	sdcc     -c -mhc08 main.c
	sdas6808 -g -o -l reset.s
	sdld6808 -s -f main.lk
) else (
    rem link file does not exist, create it NOTE : This requires a compilable, self contained main.c
	sdcc -mhc08 main.c
	echo Please edit main.lk so the HOME address is 0xDC00
)

del *.asm
del *.cdb
del *.lst
del *.map
del *.rel
del *.rst
del *.sym
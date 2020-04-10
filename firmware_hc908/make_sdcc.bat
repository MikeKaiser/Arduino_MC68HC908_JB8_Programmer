@echo off
if exist main.lk (
    rem link file already exists, use it
	sdcc -c -mhc08 main.c
	sdld6808 -s -f main.lk
) else (
    rem link file does not exist, create it
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
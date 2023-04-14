@echo on
setlocal

cd ..

rem assemble boot sector
nasm.exe src/boot/boot.asm -o bin/boot.bin

rem build libc
rem src/libc/build.bat || (echo Build failed! & exit)

rem compile and assemble kernel
compileall src/kernel obj/kernel "-I src/libc/include" || (echo Build failed! & exit)

rem link kernel
linkall obj/kernel bin/kernel.bin "--oformat binary -Ttext 0x8000 -e 0x8000 -T linker.ld" src/libc/obj/globals || (echo Build failed! & exit)

rem build binaries
if not exist bin\boot.bin (echo Build failed! & exit)
if not exist bin\kernel.bin (echo Build failed! & exit)
copy /b bin\boot.bin+bin\kernel.bin=image.vhd
filepad.exe 0x30200 image.vhd || (echo Build failed! & exit)
rem filepad.exe 0x40000000 image.vhd
rem 1gb disk image - filepad.exe 0x40000000 image.vhd
rem 1mb disk image - 
filepad.exe 0x100000 image.vhd

cd src
run.bat
@echo off
setlocal

cd ..

rem status for building; 0 = success
set /A kernel = 0

rem compile and assemble kernel
compileall src/kernel obj/kernel "-I src/libc/include -mcmodel=kernel" || (set /A kernel = 1)

rem link kernel
if %kernel%==0 (linkall obj/kernel bin/kernel.bin "--oformat binary -Ttext 0xFFFFFFFF80010000 -e 0xFFFFFFFF80010000 -T linker.ld -Map bin/ptos.map" src/libc/obj/globals || (set /A kernel = 2))

rem install kernel in the filesystem
if %kernel%==0 (bin-install fs bootable\imageGPT_testing.vhd /ptos/sys/kernel.bin bin/kernel.bin || (set /A kernel = 3))

rem display errors
if %kernel%==1 (echo "Failed to compile or assemble the kernel!")
if %kernel%==2 (echo "Failed to link the kernel!")
if %kernel%==3 (echo "Failed to install the kernel into the disk image!")

rem exit
if %kernel%==0 (echo "Finished!" & exit /b 0) else (exit /b 1)
@echo off
setlocal

cd ..

rem status for building; 0 = success
set /A boot = 0
set /A kernel = 0

rem build bootloader stages
rem nasm.exe src/boot/boot.asm -o bin/boot.bin
nasm.exe src/boot/boot.asm -o bin/boot.bin || (set /A boot = 1)
nasm.exe src/boot/partition_vbr.asm -o bin/partition_vbr.bin || (set /A boot = 1)
nasm.exe src/boot/debug_module.asm -o bin/debug_module.bin || (set /A boot = 1)
nasm.exe src/boot/fsboot.asm -o bin/fsboot.bin || (set /A boot = 1)

rem install bootsector
if %boot%==0 (bin-install raw bootable\imageGPT_testing.vhd bin/boot.bin -slen 446 || (set /A boot = 3))
rem install fat vbr, part 1
if %boot%==0 (bin-install raw bootable\imageGPT_testing.vhd bin/partition_vbr.bin -slen 3 -toffs 22020096 || (set /A boot = 3))
rem install fat vbr, part 2
if %boot%==0 (bin-install raw bootable\imageGPT_testing.vhd bin/partition_vbr.bin -soffs 90 -toffs 22020186 || (set /A boot = 3))
rem install debug module after vbr
if %boot%==0 (bin-install raw bootable\imageGPT_testing.vhd bin/debug_module.bin -toffs 22021120 || (set /A boot = 3))
rem install bootx16 in the filesystem
if %boot%==0 (bin-install fs bootable\imageGPT_testing.vhd /ptos/sys/bootx16.bin bin/fsboot.bin  || (set /A boot = 3))

rem compile and assemble kernel
compileall src/kernel obj/kernel "-I src/libc/include" || (set /A kernel = 1)

rem link kernel
if %kernel%==0 (linkall obj/kernel bin/kernel.bin "--oformat binary -Ttext 0x8000 -e 0x8000 -T linker.ld -Map bin/ptos.map" src/libc/obj/globals || (set /A kernel = 2))

rem install kernel in the filesystem
if %kernel%==0 (bin-install fs bootable\imageGPT_testing.vhd /ptos/sys/kernel.bin bin/kernel.bin || (set /A kernel = 3))

rem display errors
if %boot%==1 (echo "Failed to build the bootoloader stages!")
if %boot%==2 (echo "The impossible happened building the bootloader stages, there is no linking!")
if %boot%==3 (echo "Failed to install the bootloader into the disk image!")
if %kernel%==1 (echo "Failed to compile or assemble the kernel!")
if %kernel%==2 (echo "Failed to link the kernel!")
if %kernel%==3 (echo "Failed to install the kernel into the disk image!")

rem exit
set /A result = %boot% + %kernel%
if %result%==0 (echo "Finished!" & exit 0) else (exit 1)

rem build libc
rem src/libc/build.bat || (echo Build failed! & exit)


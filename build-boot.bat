@echo off
setlocal

cd ..

rem status for building; 0 = success
set /A boot = 0

rem build bootloader stages
rem nasm.exe src/boot/boot.asm -o bin/boot.bin
nasm.exe src/boot/boot.asm -o bin/boot.bin || (set /A boot = 1)
nasm.exe src/boot/partition_vbr.asm -o bin/partition_vbr.bin || (set /A boot = 1)
nasm.exe src/boot/debug_module.asm -o bin/debug_module.bin || (set /A boot = 1)
nasm.exe src/boot/fsboot.asm -o bin/fsboot.bin || (set /A boot = 1)

rem install bootsector
if %boot%==0 (bin-install raw bootable\imageGPT.vhd bin/boot.bin -slen 446 || (set /A boot = 3))
rem install fat vbr, part 1
if %boot%==0 (bin-install raw bootable\imageGPT.vhd bin/partition_vbr.bin -slen 3 -toffs 22020096 || (set /A boot = 3))
rem install fat vbr, part 2
if %boot%==0 (bin-install raw bootable\imageGPT.vhd bin/partition_vbr.bin -soffs 90 -toffs 22020186 || (set /A boot = 3))
rem install debug module after vbr
if %boot%==0 (bin-install raw bootable\imageGPT.vhd bin/debug_module.bin -toffs 22021120 || (set /A boot = 3))
rem install bootx16 in the filesystem
if %boot%==0 (bin-install fs bootable\imageGPT.vhd /ptos/sys/bootx16.bin bin/fsboot.bin  || (set /A boot = 3))

rem display errors
if %boot%==1 (echo "Failed to build the bootoloader stages!")
if %boot%==2 (echo "The impossible happened building the bootloader stages, there is no linking!")
if %boot%==3 (echo "Failed to install the bootloader into the disk image!")

rem exit
if %boot%==0 (echo "Finished!" & exit 0) else (exit 1)

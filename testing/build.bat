@echo off
setlocal

set /A success = 1

rem build the test case files
x86_64-elf-g++.exe --language c++ tests/kernel/core/paging.cpp -ffreestanding -fno-exceptions -masm=intel -fno-rtti -I "../libc/include" -I "../kernel" -I "main" -c -o obj/tests/kernel/core/paging.cpp.o || (set /A success = 0)

if %success%==0 (echo "Failed to compile tests" & exit /b 1)

rem link the test case files
x86_64-elf-ld.exe -o bin/paging.bin obj/tests/kernel/core/paging.cpp.o --oformat binary -Ttext 0x100000 -e main -T linker.ld || (set /A success = 0)

if %success%==0 (echo "Failed to link tests" & exit /b 1)

rem install test case files
bin-install fs ../../bootable\imageGPT.vhd /programs/tests/paging.bin bin/paging.bin || (set /A success = 0)

if %success%==0 (echo "Failed to install the tests" & exit /b 1)

echo Build Finished!
exit
@echo on
setlocal

cd ..

rem ahci interface storage controller
rem call ..\qemu\qemu-system-x86_64.exe -monitor stdio -smp 2 -m 1G -cpu Nehalem -device ahci,id=ahci -drive format=raw,file=image.vhd,if=none,id=disk -device ide-hd,drive=disk,bus=ahci.0 -drive format=raw,file=bootable/image.vhd,index=1
rem ide storage controller
call ..\qemu\qemu-system-x86_64.exe -monitor stdio -smp 2 -m 1G -cpu Nehalem -drive format=raw,file=image.vhd -drive format=raw,file=bootable/image.vhd
rem call bochsrc.bxrc
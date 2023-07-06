@echo on
setlocal

cd ..

rem ahci interface storage controller
call ..\qemu\qemu-system-x86_64.exe -monitor stdio -smp 2 -m 1G -cpu Nehalem -device ahci,id=ahci -drive format=raw,file=image.vhd,if=none,id=osimg -device ide-hd,drive=osimg,bus=ahci.0 -drive format=raw,file=bootable/imageGPT.vhd,if=none,id=fsimg -device ide-hd,drive=fsimg,bus=ahci.1

rem both ahci controller and ide controller
rem call ..\qemu\qemu-system-x86_64.exe -monitor stdio -smp 2 -m 1G -cpu Nehalem -device ahci,id=ahci -drive format=raw,file=image.vhd,if=none,id=osimg -device ide-hd,drive=osimg,bus=ahci.0,bootindex=0 -drive format=raw,file=bootable/image.vhd,index=0

rem ide storage controller
rem call ..\qemu\qemu-system-x86_64.exe -monitor stdio -smp 2 -m 1G -cpu Nehalem -drive format=raw,file=image.vhd -drive format=raw,file=bootable/image.vhd -soundhw pcspk

rem call bochsrc.bxrc
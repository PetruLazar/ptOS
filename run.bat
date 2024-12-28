@echo off
setlocal

cd ..

rem ahci interface storage controller
rem call ..\qemu\qemu-system-x86_64.exe -monitor stdio -smp 2 -m 1G -cpu Nehalem -device ahci,id=ahci -drive format=raw,file=bootable/testing.vhd ,if=none,id=osimg -device ide-hd,drive=osimg,bus=ahci.0 -device intel-hda
    call ..\qemu\qemu-system-x86_64.exe -monitor stdio -smp 2 -m 1G -cpu Nehalem -device ahci,id=ahci -drive format=raw,file=bootable/imageGPT.vhd,if=none,id=osimg -device ide-hd,drive=osimg,bus=ahci.0 -device intel-hda

rem both ahci controller and ide controller - !!! - invalid disk image "bootable/image.vhd" for fsimg, hda
rem call ..\qemu\qemu-system-x86_64.exe -monitor stdio -smp 2 -m 1G -cpu Nehalem -device ahci,id=ahci -drive format=raw,file=bootable/imageGPT.vhd,if=none,id=osimg -device ide-hd,drive=osimg,bus=ahci.0,bootindex=0 -drive format=raw,file=bootable/image.vhd,index=0 -device intel-hda

rem ide storage controller
rem call ..\qemu\qemu-system-x86_64.exe -monitor stdio -smp 2 -m 1G -cpu Nehalem -drive format=raw,file=bootable/imageGPT.vhd -soundhw pcspk -device intel-hda

rem call bochsrc.bxrc
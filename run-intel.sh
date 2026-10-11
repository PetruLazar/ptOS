#!/usr/bin/bash

cd ..

# ahci interface storage controller
qemu-system-x86_64 -monitor stdio -smp 2 -m 1G -cpu Nehalem -device ahci,id=ahci -drive format=raw,file=bootable/imageGPT.vhd,if=none,id=osimg -device ide-hd,drive=osimg,bus=ahci.0 -device intel-hda

# both ahci controller and ide controller - !!! - invalid disk image "bootable/image.vhd" for fsimg, hda
# call qemu-system-x86_64.exe -monitor stdio -smp 2 -m 1G -cpu Nehalem -device ahci,id=ahci -drive format=raw,file=bootable/imageGPT.vhd,if=none,id=osimg -device ide-hd,drive=osimg,bus=ahci.0,bootindex=0 -drive format=raw,file=bootable/image.vhd,index=0 -device intel-hda

# ide storage controller
# call qemu-system-x86_64.exe -monitor stdio -smp 2 -m 1G -cpu Nehalem -drive format=raw,file=bootable/imageGPT.vhd -soundhw pcspk -device intel-hda

# call bochsrc.bxrc
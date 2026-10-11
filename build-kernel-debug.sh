#!/bin/bash

cd ..

# keep a status; 0 = success
status=0

# compile and assemble kernel
compileall src/kernel obj/kernel "-I src/libc/include -masm=intel -mcmodel=kernel -mno-red-zone -D_DEBUG -Wreturn-type" || status=1

# link kernel
if [ $status -eq 0 ]; then
  linkall obj/kernel bin/kernel.bin "--oformat binary -Ttext 0xFFFFFFFF80010000 -e 0xFFFFFFFF80010000 -T linker.ld -Map bin/ptos.map" src/libc/obj/globals || status=2
fi

# install kernel in the filesystem
if [ $status -eq 0 ]; then
  bin-install fs bootable/imageGPT.vhd /ptos/sys/kernel.bin bin/kernel.bin || status=3
fi

# display errors

if [ $status -eq 0 ]; then
  echo "Finished!"
elif [ $status -eq 1 ]; then
  echo "Failed to compile or assemble the kernel!"
elif [ $status -eq 2 ]; then
  echo "Failed to link the kernel!"
elif [ $status -eq 3 ]; then
  echo "Failed to install the kernel into the disk image!"
fi

exit $status
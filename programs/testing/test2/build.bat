compileall src obj "-I ../../../libc/include" || (echo Build failed! & pause & exit)
linkall obj bin/test2.bin "--oformat binary -Ttext 0x100000 -e 0x100000 -T linker.ld" "../../../libc/obj" || (echo Build failed! & pause & exit)
pause
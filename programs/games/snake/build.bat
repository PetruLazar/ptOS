compileall src obj "-I ../../../libc/include" || (echo Build failed! & pause & exit)
linkall obj bin/snake.bin "../../../libc/libc.o --oformat binary -Ttext 0x100000 -e 0x100000 -T linker.ld" || (echo Build failed! & pause & exit)
pause
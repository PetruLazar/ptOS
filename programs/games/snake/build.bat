compileall src obj "-I ../../../libc/include" || (echo Build failed! & pause & exit)
linkall obj bin/snake.bin "--oformat binary -Ttext 0x100000 -e entry -T linker.ld" "..\..\..\libc\obj" || (echo Build failed! & pause & exit)
pause
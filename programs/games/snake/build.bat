compileall src obj || (echo Build failed! & pause & exit)
linkall obj bin/snake.bin "-Ttext 0x100000 -e 0x100000 -T linker.ld" || (echo Build failed! & pause & exit)
pause
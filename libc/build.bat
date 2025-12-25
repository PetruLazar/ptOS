@echo off
setlocal

compileall src obj "-I include -masm=intel -mcmodel=kernel" || (echo Build failed! & exit)
echo Finished
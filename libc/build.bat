compileall src obj "-I include -mcmodel=kernel" || (echo Build failed! & exit)
echo Finished
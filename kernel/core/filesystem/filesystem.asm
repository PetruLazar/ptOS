global FATchecksum

;rdi - char shortName[11]
FATchecksum:
push rcx
xor al, al
mov rcx, 11
loopt:
ror al, 1
add al, [rdi]
inc rdi
loop loopt
pop rcx
ret
00000000  50                push rax
00000001  07                db 0x07
00000002  1000              adc [rax],al
00000004  0000              add [rax],al
00000006  0000              add [rax],al
00000008  55                push rbp
00000009  4889E5            mov rbp,rsp
0000000C  4883EC10          sub rsp,byte +0x10
00000010  E8840D0000        call 0xd99
00000015  8945FC            mov [rbp-0x4],eax
00000018  8B4DFC            mov ecx,[rbp-0x4]
0000001B  BACDCCCCCC        mov edx,0xcccccccd
00000020  89C8              mov eax,ecx
00000022  F7E2              mul edx
00000024  C1EA06            shr edx,byte 0x6
00000027  89D0              mov eax,edx
00000029  C1E002            shl eax,byte 0x2
0000002C  01D0              add eax,edx
0000002E  C1E004            shl eax,byte 0x4
00000031  29C1              sub ecx,eax
00000033  89CA              mov edx,ecx
00000035  8855FB            mov [rbp-0x5],dl
00000038  8B45FC            mov eax,[rbp-0x4]
0000003B  BACDCCCCCC        mov edx,0xcccccccd
00000040  F7E2              mul edx
00000042  89D1              mov ecx,edx
00000044  C1E906            shr ecx,byte 0x6
00000047  BA1F85EB51        mov edx,0x51eb851f
0000004C  89C8              mov eax,ecx
0000004E  F7E2              mul edx
00000050  C1EA03            shr edx,byte 0x3
00000053  89D0              mov eax,edx
00000055  C1E002            shl eax,byte 0x2
00000058  01D0              add eax,edx
0000005A  8D148500000000    lea edx,[rax*4+0x0]
00000061  01D0              add eax,edx
00000063  29C1              sub ecx,eax
00000065  89CA              mov edx,ecx
00000067  8855FA            mov [rbp-0x6],dl
0000006A  488B05CF120000    mov rax,[rel 0x1340]
00000071  0FB655FA          movzx edx,byte [rbp-0x6]
00000075  48C1E203          shl rdx,byte 0x3
00000079  4801D0            add rax,rdx
0000007C  488B10            mov rdx,[rax]
0000007F  0FB645FB          movzx eax,byte [rbp-0x5]
00000083  4801D0            add rax,rdx
00000086  0FB600            movzx eax,byte [rax]
00000089  84C0              test al,al
0000008B  7402              jz 0x8f
0000008D  EB81              jmp short 0x10
0000008F  488B05AA120000    mov rax,[rel 0x1340]
00000096  0FB655FA          movzx edx,byte [rbp-0x6]
0000009A  48C1E203          shl rdx,byte 0x3
0000009E  4801D0            add rax,rdx
000000A1  488B10            mov rdx,[rax]
000000A4  0FB645FB          movzx eax,byte [rbp-0x5]
000000A8  4801D0            add rax,rdx
000000AB  C60001            mov byte [rax],0x1
000000AE  90                nop
000000AF  C9                leave
000000B0  C3                ret
000000B1  55                push rbp
000000B2  4889E5            mov rbp,rsp
000000B5  4883EC10          sub rsp,byte +0x10
000000B9  C645FF00          mov byte [rbp-0x1],0x0
000000BD  807DFF4F          cmp byte [rbp-0x1],0x4f
000000C1  7741              ja 0x104
000000C3  C645FE00          mov byte [rbp-0x2],0x0
000000C7  807DFE18          cmp byte [rbp-0x2],0x18
000000CB  772B              ja 0xf8
000000CD  488B056C120000    mov rax,[rel 0x1340]
000000D4  0FB655FE          movzx edx,byte [rbp-0x2]
000000D8  48C1E203          shl rdx,byte 0x3
000000DC  4801D0            add rax,rdx
000000DF  488B10            mov rdx,[rax]
000000E2  0FB645FF          movzx eax,byte [rbp-0x1]
000000E6  4801D0            add rax,rdx
000000E9  C60000            mov byte [rax],0x0
000000EC  0FB645FE          movzx eax,byte [rbp-0x2]
000000F0  83C001            add eax,byte +0x1
000000F3  8845FE            mov [rbp-0x2],al
000000F6  EBCF              jmp short 0xc7
000000F8  0FB645FF          movzx eax,byte [rbp-0x1]
000000FC  83C001            add eax,byte +0x1
000000FF  8845FF            mov [rbp-0x1],al
00000102  EBB9              jmp short 0xbd
00000104  C745F800000000    mov dword [rbp-0x8],0x0
0000010B  817DF8C7000000    cmp dword [rbp-0x8],0xc7
00000112  7F0B              jg 0x11f
00000114  E8EFFEFFFF        call 0x8
00000119  8345F801          add dword [rbp-0x8],byte +0x1
0000011D  EBEC              jmp short 0x10b
0000011F  488B051A120000    mov rax,[rel 0x1340]
00000126  480588000000      add rax,0x88
0000012C  488B00            mov rax,[rax]
0000012F  4883C00F          add rax,byte +0xf
00000133  C60005            mov byte [rax],0x5
00000136  488B0503120000    mov rax,[rel 0x1340]
0000013D  480588000000      add rax,0x88
00000143  488B00            mov rax,[rax]
00000146  4883C010          add rax,byte +0x10
0000014A  C60005            mov byte [rax],0x5
0000014D  488B05EC110000    mov rax,[rel 0x1340]
00000154  480588000000      add rax,0x88
0000015A  488B00            mov rax,[rax]
0000015D  4883C011          add rax,byte +0x11
00000161  C60006            mov byte [rax],0x6
00000164  C605DE11000011    mov byte [rel 0x1349],0x11
0000016B  C605D611000011    mov byte [rel 0x1348],0x11
00000172  C605D211000011    mov byte [rel 0x134b],0x11
00000179  C605CA1100000F    mov byte [rel 0x134a],0xf
00000180  C605790E000003    mov byte [rel 0x1000],0x3
00000187  90                nop
00000188  C9                leave
00000189  C3                ret
0000018A  55                push rbp
0000018B  4889E5            mov rbp,rsp
0000018E  4883EC10          sub rsp,byte +0x10
00000192  C645FF00          mov byte [rbp-0x1],0x0
00000196  807DFF4F          cmp byte [rbp-0x1],0x4f
0000019A  0F8797000000      ja near 0x237
000001A0  C645FE00          mov byte [rbp-0x2],0x0
000001A4  807DFE18          cmp byte [rbp-0x2],0x18
000001A8  777E              ja 0x228
000001AA  488B058F110000    mov rax,[rel 0x1340]
000001B1  0FB655FE          movzx edx,byte [rbp-0x2]
000001B5  48C1E203          shl rdx,byte 0x3
000001B9  4801D0            add rax,rdx
000001BC  488B10            mov rdx,[rax]
000001BF  0FB645FF          movzx eax,byte [rbp-0x1]
000001C3  4801D0            add rax,rdx
000001C6  0FB600            movzx eax,byte [rax]
000001C9  84C0              test al,al
000001CB  7406              jz 0x1d3
000001CD  3C01              cmp al,0x1
000001CF  741A              jz 0x1eb
000001D1  EB30              jmp short 0x203
000001D3  0FB64DFF          movzx ecx,byte [rbp-0x1]
000001D7  0FB645FE          movzx eax,byte [rbp-0x2]
000001DB  BA00000000        mov edx,0x0
000001E0  89CE              mov esi,ecx
000001E2  89C7              mov edi,eax
000001E4  E8BC110000        call 0x13a5
000001E9  EB2E              jmp short 0x219
000001EB  0FB64DFF          movzx ecx,byte [rbp-0x1]
000001EF  0FB645FE          movzx eax,byte [rbp-0x2]
000001F3  BA04000000        mov edx,0x4
000001F8  89CE              mov esi,ecx
000001FA  89C7              mov edi,eax
000001FC  E8A4110000        call 0x13a5
00000201  EB16              jmp short 0x219
00000203  0FB64DFF          movzx ecx,byte [rbp-0x1]
00000207  0FB645FE          movzx eax,byte [rbp-0x2]
0000020B  BA02000000        mov edx,0x2
00000210  89CE              mov esi,ecx
00000212  89C7              mov edi,eax
00000214  E88C110000        call 0x13a5
00000219  0FB645FE          movzx eax,byte [rbp-0x2]
0000021D  83C001            add eax,byte +0x1
00000220  8845FE            mov [rbp-0x2],al
00000223  E97CFFFFFF        jmp 0x1a4
00000228  0FB645FF          movzx eax,byte [rbp-0x1]
0000022C  83C001            add eax,byte +0x1
0000022F  8845FF            mov [rbp-0x1],al
00000232  E95FFFFFFF        jmp 0x196
00000237  90                nop
00000238  C9                leave
00000239  C3                ret
0000023A  55                push rbp
0000023B  4889E5            mov rbp,rsp
0000023E  4883EC10          sub rsp,byte +0x10
00000242  C645FF00          mov byte [rbp-0x1],0x0
00000246  0FB605B30D0000    movzx eax,byte [rel 0x1000]
0000024D  3C01              cmp al,0x1
0000024F  746F              jz 0x2c0
00000251  3C01              cmp al,0x1
00000253  7215              jc 0x26a
00000255  3C02              cmp al,0x2
00000257  0F84B9000000      jz near 0x316
0000025D  3C03              cmp al,0x3
0000025F  0F8400010000      jz near 0x365
00000265  E954010000        jmp 0x3be
0000026A  488B05CF100000    mov rax,[rel 0x1340]
00000271  0FB615D1100000    movzx edx,byte [rel 0x1349]
00000278  0FB6D2            movzx edx,dl
0000027B  48C1E203          shl rdx,byte 0x3
0000027F  4801D0            add rax,rdx
00000282  488B10            mov rdx,[rax]
00000285  0FB605BC100000    movzx eax,byte [rel 0x1348]
0000028C  0FB6C0            movzx eax,al
0000028F  4801D0            add rax,rdx
00000292  C60002            mov byte [rax],0x2
00000295  0FB605AD100000    movzx eax,byte [rel 0x1349]
0000029C  83E801            sub eax,byte +0x1
0000029F  8805A4100000      mov [rel 0x1349],al
000002A5  0FB6059D100000    movzx eax,byte [rel 0x1349]
000002AC  3CFF              cmp al,0xff
000002AE  0F8500010000      jnz near 0x3b4
000002B4  C6058E10000018    mov byte [rel 0x1349],0x18
000002BB  E9F4000000        jmp 0x3b4
000002C0  488B0579100000    mov rax,[rel 0x1340]
000002C7  0FB6157B100000    movzx edx,byte [rel 0x1349]
000002CE  0FB6D2            movzx edx,dl
000002D1  48C1E203          shl rdx,byte 0x3
000002D5  4801D0            add rax,rdx
000002D8  488B10            mov rdx,[rax]
000002DB  0FB60566100000    movzx eax,byte [rel 0x1348]
000002E2  0FB6C0            movzx eax,al
000002E5  4801D0            add rax,rdx
000002E8  C60003            mov byte [rax],0x3
000002EB  0FB60557100000    movzx eax,byte [rel 0x1349]
000002F2  83C001            add eax,byte +0x1
000002F5  88054E100000      mov [rel 0x1349],al
000002FB  0FB60547100000    movzx eax,byte [rel 0x1349]
00000302  3C19              cmp al,0x19
00000304  0F85AD000000      jnz near 0x3b7
0000030A  C6053810000000    mov byte [rel 0x1349],0x0
00000311  E9A1000000        jmp 0x3b7
00000316  488B0523100000    mov rax,[rel 0x1340]
0000031D  0FB61525100000    movzx edx,byte [rel 0x1349]
00000324  0FB6D2            movzx edx,dl
00000327  48C1E203          shl rdx,byte 0x3
0000032B  4801D0            add rax,rdx
0000032E  488B10            mov rdx,[rax]
00000331  0FB60510100000    movzx eax,byte [rel 0x1348]
00000338  0FB6C0            movzx eax,al
0000033B  4801D0            add rax,rdx
0000033E  C60004            mov byte [rax],0x4
00000341  0FB60500100000    movzx eax,byte [rel 0x1348]
00000348  83E801            sub eax,byte +0x1
0000034B  8805F70F0000      mov [rel 0x1348],al
00000351  0FB605F00F0000    movzx eax,byte [rel 0x1348]
00000358  3CFF              cmp al,0xff
0000035A  755E              jnz 0x3ba
0000035C  C605E50F00004F    mov byte [rel 0x1348],0x4f
00000363  EB55              jmp short 0x3ba
00000365  488B05D40F0000    mov rax,[rel 0x1340]
0000036C  0FB615D60F0000    movzx edx,byte [rel 0x1349]
00000373  0FB6D2            movzx edx,dl
00000376  48C1E203          shl rdx,byte 0x3
0000037A  4801D0            add rax,rdx
0000037D  488B10            mov rdx,[rax]
00000380  0FB605C10F0000    movzx eax,byte [rel 0x1348]
00000387  0FB6C0            movzx eax,al
0000038A  4801D0            add rax,rdx
0000038D  C60005            mov byte [rax],0x5
00000390  0FB605B10F0000    movzx eax,byte [rel 0x1348]
00000397  83C001            add eax,byte +0x1
0000039A  8805A80F0000      mov [rel 0x1348],al
000003A0  0FB605A10F0000    movzx eax,byte [rel 0x1348]
000003A7  3C50              cmp al,0x50
000003A9  7512              jnz 0x3bd
000003AB  C605960F000000    mov byte [rel 0x1348],0x0
000003B2  EB09              jmp short 0x3bd
000003B4  90                nop
000003B5  EB07              jmp short 0x3be
000003B7  90                nop
000003B8  EB04              jmp short 0x3be
000003BA  90                nop
000003BB  EB01              jmp short 0x3be
000003BD  90                nop
000003BE  488B057B0F0000    mov rax,[rel 0x1340]
000003C5  0FB6157D0F0000    movzx edx,byte [rel 0x1349]
000003CC  0FB6D2            movzx edx,dl
000003CF  48C1E203          shl rdx,byte 0x3
000003D3  4801D0            add rax,rdx
000003D6  488B10            mov rdx,[rax]
000003D9  0FB605680F0000    movzx eax,byte [rel 0x1348]
000003E0  0FB6C0            movzx eax,al
000003E3  4801D0            add rax,rdx
000003E6  0FB600            movzx eax,byte [rax]
000003E9  84C0              test al,al
000003EB  7416              jz 0x403
000003ED  3C01              cmp al,0x1
000003EF  7506              jnz 0x3f7
000003F1  C645FF01          mov byte [rbp-0x1],0x1
000003F5  EB0D              jmp short 0x404
000003F7  C6054E0F000001    mov byte [rel 0x134c],0x1
000003FE  E927010000        jmp 0x52a
00000403  90                nop
00000404  807DFF00          cmp byte [rbp-0x1],0x0
00000408  740A              jz 0x414
0000040A  E8F9FBFFFF        call 0x8
0000040F  E916010000        jmp 0x52a
00000414  488B05250F0000    mov rax,[rel 0x1340]
0000041B  0FB615290F0000    movzx edx,byte [rel 0x134b]
00000422  0FB6D2            movzx edx,dl
00000425  48C1E203          shl rdx,byte 0x3
00000429  4801D0            add rax,rdx
0000042C  488B10            mov rdx,[rax]
0000042F  0FB605140F0000    movzx eax,byte [rel 0x134a]
00000436  0FB6C0            movzx eax,al
00000439  4801D0            add rax,rdx
0000043C  0FB600            movzx eax,byte [rax]
0000043F  8845FE            mov [rbp-0x2],al
00000442  488B05F70E0000    mov rax,[rel 0x1340]
00000449  0FB615FB0E0000    movzx edx,byte [rel 0x134b]
00000450  0FB6D2            movzx edx,dl
00000453  48C1E203          shl rdx,byte 0x3
00000457  4801D0            add rax,rdx
0000045A  488B10            mov rdx,[rax]
0000045D  0FB605E60E0000    movzx eax,byte [rel 0x134a]
00000464  0FB6C0            movzx eax,al
00000467  4801D0            add rax,rdx
0000046A  C60000            mov byte [rax],0x0
0000046D  0FB645FE          movzx eax,byte [rbp-0x2]
00000471  3C03              cmp al,0x3
00000473  741A              jz 0x48f
00000475  3C03              cmp al,0x3
00000477  7709              ja 0x482
00000479  3C02              cmp al,0x2
0000047B  747E              jz 0x4fb
0000047D  E9A8000000        jmp 0x52a
00000482  3C04              cmp al,0x4
00000484  742D              jz 0x4b3
00000486  3C05              cmp al,0x5
00000488  744D              jz 0x4d7
0000048A  E99B000000        jmp 0x52a
0000048F  0FB605B50E0000    movzx eax,byte [rel 0x134b]
00000496  83C001            add eax,byte +0x1
00000499  8805AC0E0000      mov [rel 0x134b],al
0000049F  0FB605A50E0000    movzx eax,byte [rel 0x134b]
000004A6  3C19              cmp al,0x19
000004A8  7576              jnz 0x520
000004AA  C6059A0E000000    mov byte [rel 0x134b],0x0
000004B1  EB6D              jmp short 0x520
000004B3  0FB605900E0000    movzx eax,byte [rel 0x134a]
000004BA  83E801            sub eax,byte +0x1
000004BD  8805870E0000      mov [rel 0x134a],al
000004C3  0FB605800E0000    movzx eax,byte [rel 0x134a]
000004CA  3CFF              cmp al,0xff
000004CC  7555              jnz 0x523
000004CE  C605750E00004F    mov byte [rel 0x134a],0x4f
000004D5  EB4C              jmp short 0x523
000004D7  0FB6056C0E0000    movzx eax,byte [rel 0x134a]
000004DE  83C001            add eax,byte +0x1
000004E1  8805630E0000      mov [rel 0x134a],al
000004E7  0FB6055C0E0000    movzx eax,byte [rel 0x134a]
000004EE  3C50              cmp al,0x50
000004F0  7534              jnz 0x526
000004F2  C605510E000000    mov byte [rel 0x134a],0x0
000004F9  EB2B              jmp short 0x526
000004FB  0FB605490E0000    movzx eax,byte [rel 0x134b]
00000502  83E801            sub eax,byte +0x1
00000505  8805400E0000      mov [rel 0x134b],al
0000050B  0FB605390E0000    movzx eax,byte [rel 0x134b]
00000512  3CFF              cmp al,0xff
00000514  7513              jnz 0x529
00000516  C6052E0E000018    mov byte [rel 0x134b],0x18
0000051D  90                nop
0000051E  EB09              jmp short 0x529
00000520  90                nop
00000521  EB07              jmp short 0x52a
00000523  90                nop
00000524  EB04              jmp short 0x52a
00000526  90                nop
00000527  EB01              jmp short 0x52a
00000529  90                nop
0000052A  C9                leave
0000052B  C3                ret
0000052C  55                push rbp
0000052D  4889E5            mov rbp,rsp
00000530  53                push rbx
00000531  4883EC28          sub rsp,byte +0x28
00000535  E8320E0000        call 0x136c
0000053A  BE2C051000        mov esi,0x10052c
0000053F  BF01101000        mov edi,0x101001
00000544  E8910F0000        call 0x14da
00000549  E8BE0E0000        call 0x140c
0000054E  BFC8000000        mov edi,0xc8
00000553  E894070000        call 0xcec
00000558  488905E10D0000    mov [rel 0x1340],rax
0000055F  C645EF00          mov byte [rbp-0x11],0x0
00000563  807DEF18          cmp byte [rbp-0x11],0x18
00000567  772C              ja 0x595
00000569  488B05D00D0000    mov rax,[rel 0x1340]
00000570  0FB655EF          movzx edx,byte [rbp-0x11]
00000574  48C1E203          shl rdx,byte 0x3
00000578  488D1C10          lea rbx,[rax+rdx]
0000057C  BF50000000        mov edi,0x50
00000581  E866070000        call 0xcec
00000586  488903            mov [rbx],rax
00000589  0FB645EF          movzx eax,byte [rbp-0x11]
0000058D  83C001            add eax,byte +0x1
00000590  8845EF            mov [rbp-0x11],al
00000593  EBCE              jmp short 0x563
00000595  C605B00D000000    mov byte [rel 0x134c],0x0
0000059C  E810FBFFFF        call 0xb1
000005A1  E8E4FBFFFF        call 0x18a
000005A6  E8990E0000        call 0x1444
000005AB  4889059E0D0000    mov [rel 0x1350],rax
000005B2  E86C0E0000        call 0x1423
000005B7  668945DE          mov [rbp-0x22],ax
000005BB  488D45DE          lea rax,[rbp-0x22]
000005BF  4889C7            mov rdi,rax
000005C2  E8910D0000        call 0x1358
000005C7  8845EE            mov [rbp-0x12],al
000005CA  0FB645EE          movzx eax,byte [rbp-0x12]
000005CE  83E80C            sub eax,byte +0xc
000005D1  3C4C              cmp al,0x4c
000005D3  0F87C4000000      ja near 0x69d
000005D9  0FB6C0            movzx eax,al
000005DC  488B04C518101000  mov rax,[rax*8+0x101018]
000005E4  FFE0              jmp rax
000005E6  0FB605130A0000    movzx eax,byte [rel 0x1000]
000005ED  3C01              cmp al,0x1
000005EF  0F849B000000      jz near 0x690
000005F5  C605040A000000    mov byte [rel 0x1000],0x0
000005FC  E98F000000        jmp 0x690
00000601  0FB605F8090000    movzx eax,byte [rel 0x1000]
00000608  3C03              cmp al,0x3
0000060A  0F8483000000      jz near 0x693
00000610  C605E909000002    mov byte [rel 0x1000],0x2
00000617  EB7A              jmp short 0x693
00000619  0FB605E0090000    movzx eax,byte [rel 0x1000]
00000620  84C0              test al,al
00000622  7472              jz 0x696
00000624  C605D509000001    mov byte [rel 0x1000],0x1
0000062B  EB69              jmp short 0x696
0000062D  0FB605CC090000    movzx eax,byte [rel 0x1000]
00000634  3C02              cmp al,0x2
00000636  7461              jz 0x699
00000638  C605C109000003    mov byte [rel 0x1000],0x3
0000063F  EB58              jmp short 0x699
00000641  0FB605040D0000    movzx eax,byte [rel 0x134c]
00000648  84C0              test al,al
0000064A  7450              jz 0x69c
0000064C  E81B0D0000        call 0x136c
00000651  E8B60D0000        call 0x140c
00000656  C605EF0C000000    mov byte [rel 0x134c],0x0
0000065D  E84FFAFFFF        call 0xb1
00000662  E823FBFFFF        call 0x18a
00000667  E8D80D0000        call 0x1444
0000066C  488905DD0C0000    mov [rel 0x1350],rax
00000673  EB27              jmp short 0x69c
00000675  E8F20C0000        call 0x136c
0000067A  BE0D000000        mov esi,0xd
0000067F  BF0F000000        mov edi,0xf
00000684  E8540D0000        call 0x13dd
00000689  B800000000        mov eax,0x0
0000068E  EB5C              jmp short 0x6ec
00000690  90                nop
00000691  EB0A              jmp short 0x69d
00000693  90                nop
00000694  EB07              jmp short 0x69d
00000696  90                nop
00000697  EB04              jmp short 0x69d
00000699  90                nop
0000069A  EB01              jmp short 0x69d
0000069C  90                nop
0000069D  0FB605A80C0000    movzx eax,byte [rel 0x134c]
000006A4  84C0              test al,al
000006A6  753E              jnz 0x6e6
000006A8  E8970D0000        call 0x1444
000006AD  488945E0          mov [rbp-0x20],rax
000006B1  488B05980C0000    mov rax,[rel 0x1350]
000006B8  488B55E0          mov rdx,[rbp-0x20]
000006BC  4829C2            sub rdx,rax
000006BF  4889D0            mov rax,rdx
000006C2  4883F802          cmp rax,byte +0x2
000006C6  0F86E6FEFFFF      jna near 0x5b2
000006CC  E869FBFFFF        call 0x23a
000006D1  E8B4FAFFFF        call 0x18a
000006D6  488B45E0          mov rax,[rbp-0x20]
000006DA  4889056F0C0000    mov [rel 0x1350],rax
000006E1  E9CCFEFFFF        jmp 0x5b2
000006E6  90                nop
000006E7  E9C6FEFFFF        jmp 0x5b2
000006EC  4883C428          add rsp,byte +0x28
000006F0  5B                pop rbx
000006F1  5D                pop rbp
000006F2  C3                ret
000006F3  55                push rbp
000006F4  4889E5            mov rbp,rsp
000006F7  4883EC10          sub rsp,byte +0x10
000006FB  897DFC            mov [rbp-0x4],edi
000006FE  8975F8            mov [rbp-0x8],esi
00000701  837DFC01          cmp dword [rbp-0x4],byte +0x1
00000705  7531              jnz 0x738
00000707  817DF8FFFF0000    cmp dword [rbp-0x8],0xffff
0000070E  7528              jnz 0x738
00000710  BA00000000        mov edx,0x0
00000715  BE00000000        mov esi,0x0
0000071A  BF48131000        mov edi,0x101348
0000071F  E8EC0D0000        call 0x1510
00000724  BA00000000        mov edx,0x0
00000729  BE00000000        mov esi,0x0
0000072E  BF4A131000        mov edi,0x10134a
00000733  E8D80D0000        call 0x1510
00000738  90                nop
00000739  C9                leave
0000073A  C3                ret
0000073B  55                push rbp
0000073C  4889E5            mov rbp,rsp
0000073F  BEFFFF0000        mov esi,0xffff
00000744  BF01000000        mov edi,0x1
00000749  E8A5FFFFFF        call 0x6f3
0000074E  5D                pop rbp
0000074F  C3                ret
00000750  55                push rbp
00000751  4889E5            mov rbp,rsp
00000754  BE00000100        mov esi,0x10000
00000759  48BF000000000080  mov rdi,0x800000000000
         -0000
00000763  E860100000        call 0x17c8
00000768  48890539110000    mov [rel 0x18a8],rax
0000076F  E8B8FDFFFF        call 0x52c
00000774  89C7              mov edi,eax
00000776  E88E100000        call 0x1809
0000077B  90                nop
0000077C  5D                pop rbp
0000077D  C3                ret
0000077E  55                push rbp
0000077F  4889E5            mov rbp,rsp
00000782  53                push rbx
00000783  4883EC18          sub rsp,byte +0x18
00000787  48897DE8          mov [rbp-0x18],rdi
0000078B  BE90121000        mov esi,0x101290
00000790  BF01101000        mov edi,0x101001
00000795  E80C180000        call 0x1fa6
0000079A  4889C3            mov rbx,rax
0000079D  488B45E8          mov rax,[rbp-0x18]
000007A1  4889C7            mov rdi,rax
000007A4  E8E7110000        call 0x1990
000007A9  4889C6            mov rsi,rax
000007AC  4889DF            mov rdi,rbx
000007AF  E8260D0000        call 0x14da
000007B4  BE0A000000        mov esi,0xa
000007B9  4889C7            mov rdi,rax
000007BC  E807180000        call 0x1fc8
000007C1  BE03000000        mov esi,0x3
000007C6  BF01101000        mov edi,0x101001
000007CB  E8281A0000        call 0x21f8
000007D0  488B45E8          mov rax,[rbp-0x18]
000007D4  8B401C            mov eax,[rax+0x1c]
000007D7  3D042B4A51        cmp eax,0x514a2b04
000007DC  744A              jz 0x828
000007DE  BEC5121000        mov esi,0x1012c5
000007E3  BF01101000        mov edi,0x101001
000007E8  E8B9170000        call 0x1fa6
000007ED  4889C2            mov rdx,rax
000007F0  488B45E8          mov rax,[rbp-0x18]
000007F4  8B401C            mov eax,[rax+0x1c]
000007F7  89C6              mov esi,eax
000007F9  4889D7            mov rdi,rdx
000007FC  E8E9170000        call 0x1fea
00000801  BEDC121000        mov esi,0x1012dc
00000806  4889C7            mov rdi,rax
00000809  E898170000        call 0x1fa6
0000080E  BE042B4A51        mov esi,0x514a2b04
00000813  4889C7            mov rdi,rax
00000816  E8CF170000        call 0x1fea
0000081B  BE0A000000        mov esi,0xa
00000820  4889C7            mov rdi,rax
00000823  E8A0170000        call 0x1fc8
00000828  488B45E8          mov rax,[rbp-0x18]
0000082C  4889C7            mov rdi,rax
0000082F  E8BA100000        call 0x18ee
00000834  8B00              mov eax,[rax]
00000836  3DCB5F4BA9        cmp eax,0xa94b5fcb
0000083B  0F95C0            setnz al
0000083E  84C0              test al,al
00000840  7451              jz 0x893
00000842  BEE9121000        mov esi,0x1012e9
00000847  BF01101000        mov edi,0x101001
0000084C  E855170000        call 0x1fa6
00000851  4889C3            mov rbx,rax
00000854  488B45E8          mov rax,[rbp-0x18]
00000858  4889C7            mov rdi,rax
0000085B  E88E100000        call 0x18ee
00000860  8B00              mov eax,[rax]
00000862  89C6              mov esi,eax
00000864  4889DF            mov rdi,rbx
00000867  E87E170000        call 0x1fea
0000086C  BEDC121000        mov esi,0x1012dc
00000871  4889C7            mov rdi,rax
00000874  E82D170000        call 0x1fa6
00000879  BECB5F4BA9        mov esi,0xa94b5fcb
0000087E  4889C7            mov rdi,rax
00000881  E864170000        call 0x1fea
00000886  BE0A000000        mov esi,0xa
0000088B  4889C7            mov rdi,rax
0000088E  E835170000        call 0x1fc8
00000893  BE02000000        mov esi,0x2
00000898  BF01101000        mov edi,0x101001
0000089D  E856190000        call 0x21f8
000008A2  BEFE121000        mov esi,0x1012fe
000008A7  BF01101000        mov edi,0x101001
000008AC  E8F5160000        call 0x1fa6
000008B1  488B45E8          mov rax,[rbp-0x18]
000008B5  BE00010000        mov esi,0x100
000008BA  4889C7            mov rdi,rax
000008BD  E84D1A0000        call 0x230f
000008C2  90                nop
000008C3  4883C418          add rsp,byte +0x18
000008C7  5B                pop rbx
000008C8  5D                pop rbp
000008C9  C3                ret
000008CA  55                push rbp
000008CB  4889E5            mov rbp,rsp
000008CE  4883EC20          sub rsp,byte +0x20
000008D2  48897DE8          mov [rbp-0x18],rdi
000008D6  488B45E8          mov rax,[rbp-0x18]
000008DA  488B4008          mov rax,[rax+0x8]
000008DE  488945F8          mov [rbp-0x8],rax
000008E2  48837DF800        cmp qword [rbp-0x8],byte +0x0
000008E7  0F8483000000      jz near 0x970
000008ED  488B45F8          mov rax,[rbp-0x8]
000008F1  488B4010          mov rax,[rax+0x10]
000008F5  488945F0          mov [rbp-0x10],rax
000008F9  BE0C131000        mov esi,0x10130c
000008FE  BF01101000        mov edi,0x101001
00000903  E89E160000        call 0x1fa6
00000908  4889C2            mov rdx,rax
0000090B  488B45F0          mov rax,[rbp-0x10]
0000090F  4889C6            mov rsi,rax
00000912  4889D7            mov rdi,rdx
00000915  E8EA170000        call 0x2104
0000091A  BE1B131000        mov esi,0x10131b
0000091F  4889C7            mov rdi,rax
00000922  E87F160000        call 0x1fa6
00000927  4889C2            mov rdx,rax
0000092A  488B45F8          mov rax,[rbp-0x8]
0000092E  4889C6            mov rsi,rax
00000931  4889D7            mov rdi,rdx
00000934  E8A10B0000        call 0x14da
00000939  BE25131000        mov esi,0x101325
0000093E  4889C7            mov rdi,rax
00000941  E860160000        call 0x1fa6
00000946  488B45F8          mov rax,[rbp-0x8]
0000094A  4889C7            mov rdi,rax
0000094D  E83E100000        call 0x1990
00000952  BE30000000        mov esi,0x30
00000957  4889C7            mov rdi,rax
0000095A  E8B0190000        call 0x230f
0000095F  488B45F8          mov rax,[rbp-0x8]
00000963  488B4008          mov rax,[rax+0x8]
00000967  488945F8          mov [rbp-0x8],rax
0000096B  E972FFFFFF        jmp 0x8e2
00000970  90                nop
00000971  C9                leave
00000972  C3                ret
00000973  90                nop
00000974  55                push rbp
00000975  4889E5            mov rbp,rsp
00000978  4883EC50          sub rsp,byte +0x50
0000097C  48897DC8          mov [rbp-0x38],rdi
00000980  488975C0          mov [rbp-0x40],rsi
00000984  488955B8          mov [rbp-0x48],rdx
00000988  488B55B8          mov rdx,[rbp-0x48]
0000098C  488B45C0          mov rax,[rbp-0x40]
00000990  4889D6            mov rsi,rdx
00000993  4889C7            mov rdi,rax
00000996  E8150F0000        call 0x18b0
0000099B  488945C0          mov [rbp-0x40],rax
0000099F  488B45C8          mov rax,[rbp-0x38]
000009A3  488B4008          mov rax,[rax+0x8]
000009A7  4885C0            test rax,rax
000009AA  0F8598000000      jnz near 0xa48
000009B0  488B45C8          mov rax,[rbp-0x38]
000009B4  4889C7            mov rdi,rax
000009B7  E826110000        call 0x1ae2
000009BC  488945D0          mov [rbp-0x30],rax
000009C0  488B55D0          mov rdx,[rbp-0x30]
000009C4  488B45C8          mov rax,[rbp-0x38]
000009C8  488B00            mov rax,[rax]
000009CB  488D3C02          lea rdi,[rdx+rax]
000009CF  488B4DB8          mov rcx,[rbp-0x48]
000009D3  488B55C0          mov rdx,[rbp-0x40]
000009D7  488D75D0          lea rsi,[rbp-0x30]
000009DB  488B45C8          mov rax,[rbp-0x38]
000009DF  4989C8            mov r8,rcx
000009E2  4889D1            mov rcx,rdx
000009E5  4889FA            mov rdx,rdi
000009E8  4889C7            mov rdi,rax
000009EB  E882100000        call 0x1a72
000009F0  83F001            xor eax,byte +0x1
000009F3  84C0              test al,al
000009F5  740A              jz 0xa01
000009F7  B800000000        mov eax,0x0
000009FC  E932020000        jmp 0xc33
00000A01  488B45D0          mov rax,[rbp-0x30]
00000A05  488B75C0          mov rsi,[rbp-0x40]
00000A09  B900000000        mov ecx,0x0
00000A0E  BA00000000        mov edx,0x0
00000A13  4889C7            mov rdi,rax
00000A16  E8F00E0000        call 0x190b
00000A1B  488945D8          mov [rbp-0x28],rax
00000A1F  488B45C8          mov rax,[rbp-0x38]
00000A23  488B55D8          mov rdx,[rbp-0x28]
00000A27  48895008          mov [rax+0x8],rdx
00000A2B  488B45C8          mov rax,[rbp-0x38]
00000A2F  488B55D8          mov rdx,[rbp-0x28]
00000A33  48895010          mov [rax+0x10],rdx
00000A37  488B45D8          mov rax,[rbp-0x28]
00000A3B  4889C7            mov rdi,rax
00000A3E  E84D0F0000        call 0x1990
00000A43  E9EB010000        jmp 0xc33
00000A48  488B45C8          mov rax,[rbp-0x38]
00000A4C  4889C7            mov rdi,rax
00000A4F  E88E100000        call 0x1ae2
00000A54  488945D0          mov [rbp-0x30],rax
00000A58  488B45C8          mov rax,[rbp-0x38]
00000A5C  488B5008          mov rdx,[rax+0x8]
00000A60  488B7DB8          mov rdi,[rbp-0x48]
00000A64  488B4DC0          mov rcx,[rbp-0x40]
00000A68  488D75D0          lea rsi,[rbp-0x30]
00000A6C  488B45C8          mov rax,[rbp-0x38]
00000A70  4989F8            mov r8,rdi
00000A73  4889C7            mov rdi,rax
00000A76  E8F70F0000        call 0x1a72
00000A7B  84C0              test al,al
00000A7D  7450              jz 0xacf
00000A7F  488B45C8          mov rax,[rbp-0x38]
00000A83  488B5008          mov rdx,[rax+0x8]
00000A87  488B45D0          mov rax,[rbp-0x30]
00000A8B  488B75C0          mov rsi,[rbp-0x40]
00000A8F  4889D1            mov rcx,rdx
00000A92  BA00000000        mov edx,0x0
00000A97  4889C7            mov rdi,rax
00000A9A  E86C0E0000        call 0x190b
00000A9F  488945E0          mov [rbp-0x20],rax
00000AA3  488B45C8          mov rax,[rbp-0x38]
00000AA7  488B4008          mov rax,[rax+0x8]
00000AAB  488B55E0          mov rdx,[rbp-0x20]
00000AAF  488910            mov [rax],rdx
00000AB2  488B45C8          mov rax,[rbp-0x38]
00000AB6  488B55E0          mov rdx,[rbp-0x20]
00000ABA  48895008          mov [rax+0x8],rdx
00000ABE  488B45E0          mov rax,[rbp-0x20]
00000AC2  4889C7            mov rdi,rax
00000AC5  E8C60E0000        call 0x1990
00000ACA  E964010000        jmp 0xc33
00000ACF  488B45C8          mov rax,[rbp-0x38]
00000AD3  488B4008          mov rax,[rax+0x8]
00000AD7  488945F8          mov [rbp-0x8],rax
00000ADB  488B45C8          mov rax,[rbp-0x38]
00000ADF  488B4010          mov rax,[rax+0x10]
00000AE3  483945F8          cmp [rbp-0x8],rax
00000AE7  0F8494000000      jz near 0xb81
00000AED  488B45F8          mov rax,[rbp-0x8]
00000AF1  4889C7            mov rdi,rax
00000AF4  E8790E0000        call 0x1972
00000AF9  488945D0          mov [rbp-0x30],rax
00000AFD  488B45F8          mov rax,[rbp-0x8]
00000B01  488B5008          mov rdx,[rax+0x8]
00000B05  488B7DB8          mov rdi,[rbp-0x48]
00000B09  488B4DC0          mov rcx,[rbp-0x40]
00000B0D  488D75D0          lea rsi,[rbp-0x30]
00000B11  488B45C8          mov rax,[rbp-0x38]
00000B15  4989F8            mov r8,rdi
00000B18  4889C7            mov rdi,rax
00000B1B  E8520F0000        call 0x1a72
00000B20  84C0              test al,al
00000B22  744C              jz 0xb70
00000B24  488B45F8          mov rax,[rbp-0x8]
00000B28  488B4808          mov rcx,[rax+0x8]
00000B2C  488B45D0          mov rax,[rbp-0x30]
00000B30  488B55F8          mov rdx,[rbp-0x8]
00000B34  488B75C0          mov rsi,[rbp-0x40]
00000B38  4889C7            mov rdi,rax
00000B3B  E8CB0D0000        call 0x190b
00000B40  488945F0          mov [rbp-0x10],rax
00000B44  488B45F8          mov rax,[rbp-0x8]
00000B48  488B55F0          mov rdx,[rbp-0x10]
00000B4C  48895008          mov [rax+0x8],rdx
00000B50  488B45F0          mov rax,[rbp-0x10]
00000B54  488B4008          mov rax,[rax+0x8]
00000B58  488B55F0          mov rdx,[rbp-0x10]
00000B5C  488910            mov [rax],rdx
00000B5F  488B45F0          mov rax,[rbp-0x10]
00000B63  4889C7            mov rdi,rax
00000B66  E8250E0000        call 0x1990
00000B6B  E9C3000000        jmp 0xc33
00000B70  488B45F8          mov rax,[rbp-0x8]
00000B74  488B4008          mov rax,[rax+0x8]
00000B78  488945F8          mov [rbp-0x8],rax
00000B7C  E95AFFFFFF        jmp 0xadb
00000B81  488B45C8          mov rax,[rbp-0x38]
00000B85  488B4010          mov rax,[rax+0x10]
00000B89  4889C7            mov rdi,rax
00000B8C  E8E10D0000        call 0x1972
00000B91  488945D0          mov [rbp-0x30],rax
00000B95  488B45C8          mov rax,[rbp-0x38]
00000B99  4889C7            mov rdi,rax
00000B9C  E8410F0000        call 0x1ae2
00000BA1  4889C2            mov rdx,rax
00000BA4  488B45C8          mov rax,[rbp-0x38]
00000BA8  488B00            mov rax,[rax]
00000BAB  488D3C02          lea rdi,[rdx+rax]
00000BAF  488B4DB8          mov rcx,[rbp-0x48]
00000BB3  488B55C0          mov rdx,[rbp-0x40]
00000BB7  488D75D0          lea rsi,[rbp-0x30]
00000BBB  488B45C8          mov rax,[rbp-0x38]
00000BBF  4989C8            mov r8,rcx
00000BC2  4889D1            mov rcx,rdx
00000BC5  4889FA            mov rdx,rdi
00000BC8  4889C7            mov rdi,rax
00000BCB  E8A20E0000        call 0x1a72
00000BD0  84C0              test al,al
00000BD2  744B              jz 0xc1f
00000BD4  488B45C8          mov rax,[rbp-0x38]
00000BD8  488B5010          mov rdx,[rax+0x10]
00000BDC  488B45D0          mov rax,[rbp-0x30]
00000BE0  488B75C0          mov rsi,[rbp-0x40]
00000BE4  B900000000        mov ecx,0x0
00000BE9  4889C7            mov rdi,rax
00000BEC  E81A0D0000        call 0x190b
00000BF1  488945E8          mov [rbp-0x18],rax
00000BF5  488B45C8          mov rax,[rbp-0x38]
00000BF9  488B4010          mov rax,[rax+0x10]
00000BFD  488B55E8          mov rdx,[rbp-0x18]
00000C01  48895008          mov [rax+0x8],rdx
00000C05  488B45C8          mov rax,[rbp-0x38]
00000C09  488B55E8          mov rdx,[rbp-0x18]
00000C0D  48895010          mov [rax+0x10],rdx
00000C11  488B45E8          mov rax,[rbp-0x18]
00000C15  4889C7            mov rdi,rax
00000C18  E8730D0000        call 0x1990
00000C1D  EB14              jmp short 0xc33
00000C1F  BE28131000        mov esi,0x101328
00000C24  BF01101000        mov edi,0x101001
00000C29  E878130000        call 0x1fa6
00000C2E  B800000000        mov eax,0x0
00000C33  C9                leave
00000C34  C3                ret
00000C35  55                push rbp
00000C36  4889E5            mov rbp,rsp
00000C39  4883EC10          sub rsp,byte +0x10
00000C3D  48897DF8          mov [rbp-0x8],rdi
00000C41  488B45F8          mov rax,[rbp-0x8]
00000C45  BE10000000        mov esi,0x10
00000C4A  4889C7            mov rdi,rax
00000C4D  E8080F0000        call 0x1b5a
00000C52  C9                leave
00000C53  C3                ret
00000C54  55                push rbp
00000C55  4889E5            mov rbp,rsp
00000C58  4883EC20          sub rsp,byte +0x20
00000C5C  48897DE8          mov [rbp-0x18],rdi
00000C60  488B45E8          mov rax,[rbp-0x18]
00000C64  BE10000000        mov esi,0x10
00000C69  4889C7            mov rdi,rax
00000C6C  E8E90E0000        call 0x1b5a
00000C71  488945F0          mov [rbp-0x10],rax
00000C75  48C16DE802        shr qword [rbp-0x18],byte 0x2
00000C7A  48C745F800000000  mov qword [rbp-0x8],0x0
00000C82  488B45F8          mov rax,[rbp-0x8]
00000C86  483B45E8          cmp rax,[rbp-0x18]
00000C8A  7320              jnc 0xcac
00000C8C  488B45F8          mov rax,[rbp-0x8]
00000C90  488D148500000000  lea rdx,[rax*4+0x0]
00000C98  488B45F0          mov rax,[rbp-0x10]
00000C9C  4801D0            add rax,rdx
00000C9F  C70000000000      mov dword [rax],0x0
00000CA5  488345F801        add qword [rbp-0x8],byte +0x1
00000CAA  EBD6              jmp short 0xc82
00000CAC  488B45F0          mov rax,[rbp-0x10]
00000CB0  C9                leave
00000CB1  C3                ret
00000CB2  55                push rbp
00000CB3  4889E5            mov rbp,rsp
00000CB6  4883EC10          sub rsp,byte +0x10
00000CBA  48897DF8          mov [rbp-0x8],rdi
00000CBE  488B45F8          mov rax,[rbp-0x8]
00000CC2  4889C7            mov rdi,rax
00000CC5  E8CF0E0000        call 0x1b99
00000CCA  90                nop
00000CCB  C9                leave
00000CCC  C3                ret
00000CCD  55                push rbp
00000CCE  4889E5            mov rbp,rsp
00000CD1  4883EC10          sub rsp,byte +0x10
00000CD5  48897DF8          mov [rbp-0x8],rdi
00000CD9  488B45F8          mov rax,[rbp-0x8]
00000CDD  BE10000000        mov esi,0x10
00000CE2  4889C7            mov rdi,rax
00000CE5  E8700E0000        call 0x1b5a
00000CEA  C9                leave
00000CEB  C3                ret
00000CEC  55                push rbp
00000CED  4889E5            mov rbp,rsp
00000CF0  4883EC10          sub rsp,byte +0x10
00000CF4  48897DF8          mov [rbp-0x8],rdi
00000CF8  488B45F8          mov rax,[rbp-0x8]
00000CFC  BE10000000        mov esi,0x10
00000D01  4889C7            mov rdi,rax
00000D04  E8510E0000        call 0x1b5a
00000D09  C9                leave
00000D0A  C3                ret
00000D0B  55                push rbp
00000D0C  4889E5            mov rbp,rsp
00000D0F  4883EC10          sub rsp,byte +0x10
00000D13  48897DF8          mov [rbp-0x8],rdi
00000D17  488B45F8          mov rax,[rbp-0x8]
00000D1B  4889C7            mov rdi,rax
00000D1E  E8760E0000        call 0x1b99
00000D23  90                nop
00000D24  C9                leave
00000D25  C3                ret
00000D26  55                push rbp
00000D27  4889E5            mov rbp,rsp
00000D2A  4883EC10          sub rsp,byte +0x10
00000D2E  48897DF8          mov [rbp-0x8],rdi
00000D32  488975F0          mov [rbp-0x10],rsi
00000D36  488B55F0          mov rdx,[rbp-0x10]
00000D3A  488B45F8          mov rax,[rbp-0x8]
00000D3E  4889D6            mov rsi,rdx
00000D41  4889C7            mov rdi,rax
00000D44  E8750E0000        call 0x1bbe
00000D49  90                nop
00000D4A  C9                leave
00000D4B  C3                ret
00000D4C  55                push rbp
00000D4D  4889E5            mov rbp,rsp
00000D50  4883EC10          sub rsp,byte +0x10
00000D54  48897DF8          mov [rbp-0x8],rdi
00000D58  488B45F8          mov rax,[rbp-0x8]
00000D5C  4889C7            mov rdi,rax
00000D5F  E8350E0000        call 0x1b99
00000D64  90                nop
00000D65  C9                leave
00000D66  C3                ret
00000D67  55                push rbp
00000D68  4889E5            mov rbp,rsp
00000D6B  4883EC10          sub rsp,byte +0x10
00000D6F  48897DF8          mov [rbp-0x8],rdi
00000D73  488975F0          mov [rbp-0x10],rsi
00000D77  488B45F8          mov rax,[rbp-0x8]
00000D7B  4889C7            mov rdi,rax
00000D7E  E8160E0000        call 0x1b99
00000D83  90                nop
00000D84  C9                leave
00000D85  C3                ret
00000D86  55                push rbp
00000D87  4889E5            mov rbp,rsp
00000D8A  897DFC            mov [rbp-0x4],edi
00000D8D  8B45FC            mov eax,[rbp-0x4]
00000D90  89053A1C0000      mov [rel 0x29d0],eax
00000D96  90                nop
00000D97  5D                pop rbp
00000D98  C3                ret
00000D99  55                push rbp
00000D9A  4889E5            mov rbp,rsp
00000D9D  8B052D1C0000      mov eax,[rel 0x29d0]
00000DA3  69C0E99C6B21      imul eax,eax,dword 0x216b9ce9
00000DA9  05016FB569        add eax,0x69b56f01
00000DAE  89051C1C0000      mov [rel 0x29d0],eax
00000DB4  8B05161C0000      mov eax,[rel 0x29d0]
00000DBA  5D                pop rbp
00000DBB  C3                ret
00000DBC  0000              add [rax],al
00000DBE  0000              add [rax],al
00000DC0  0000              add [rax],al
00000DC2  0000              add [rax],al
00000DC4  0000              add [rax],al
00000DC6  0000              add [rax],al
00000DC8  0000              add [rax],al
00000DCA  0000              add [rax],al
00000DCC  0000              add [rax],al
00000DCE  0000              add [rax],al
00000DD0  0000              add [rax],al
00000DD2  0000              add [rax],al
00000DD4  0000              add [rax],al
00000DD6  0000              add [rax],al
00000DD8  0000              add [rax],al
00000DDA  0000              add [rax],al
00000DDC  0000              add [rax],al
00000DDE  0000              add [rax],al
00000DE0  0000              add [rax],al
00000DE2  0000              add [rax],al
00000DE4  0000              add [rax],al
00000DE6  0000              add [rax],al
00000DE8  0000              add [rax],al
00000DEA  0000              add [rax],al
00000DEC  0000              add [rax],al
00000DEE  0000              add [rax],al
00000DF0  0000              add [rax],al
00000DF2  0000              add [rax],al
00000DF4  0000              add [rax],al
00000DF6  0000              add [rax],al
00000DF8  0000              add [rax],al
00000DFA  0000              add [rax],al
00000DFC  0000              add [rax],al
00000DFE  0000              add [rax],al
00000E00  0000              add [rax],al
00000E02  0000              add [rax],al
00000E04  0000              add [rax],al
00000E06  0000              add [rax],al
00000E08  0000              add [rax],al
00000E0A  0000              add [rax],al
00000E0C  0000              add [rax],al
00000E0E  0000              add [rax],al
00000E10  0000              add [rax],al
00000E12  0000              add [rax],al
00000E14  0000              add [rax],al
00000E16  0000              add [rax],al
00000E18  0000              add [rax],al
00000E1A  0000              add [rax],al
00000E1C  0000              add [rax],al
00000E1E  0000              add [rax],al
00000E20  0000              add [rax],al
00000E22  0000              add [rax],al
00000E24  0000              add [rax],al
00000E26  0000              add [rax],al
00000E28  0000              add [rax],al
00000E2A  0000              add [rax],al
00000E2C  0000              add [rax],al
00000E2E  0000              add [rax],al
00000E30  0000              add [rax],al
00000E32  0000              add [rax],al
00000E34  0000              add [rax],al
00000E36  0000              add [rax],al
00000E38  0000              add [rax],al
00000E3A  0000              add [rax],al
00000E3C  0000              add [rax],al
00000E3E  0000              add [rax],al
00000E40  0000              add [rax],al
00000E42  0000              add [rax],al
00000E44  0000              add [rax],al
00000E46  0000              add [rax],al
00000E48  0000              add [rax],al
00000E4A  0000              add [rax],al
00000E4C  0000              add [rax],al
00000E4E  0000              add [rax],al
00000E50  0000              add [rax],al
00000E52  0000              add [rax],al
00000E54  0000              add [rax],al
00000E56  0000              add [rax],al
00000E58  0000              add [rax],al
00000E5A  0000              add [rax],al
00000E5C  0000              add [rax],al
00000E5E  0000              add [rax],al
00000E60  0000              add [rax],al
00000E62  0000              add [rax],al
00000E64  0000              add [rax],al
00000E66  0000              add [rax],al
00000E68  0000              add [rax],al
00000E6A  0000              add [rax],al
00000E6C  0000              add [rax],al
00000E6E  0000              add [rax],al
00000E70  0000              add [rax],al
00000E72  0000              add [rax],al
00000E74  0000              add [rax],al
00000E76  0000              add [rax],al
00000E78  0000              add [rax],al
00000E7A  0000              add [rax],al
00000E7C  0000              add [rax],al
00000E7E  0000              add [rax],al
00000E80  0000              add [rax],al
00000E82  0000              add [rax],al
00000E84  0000              add [rax],al
00000E86  0000              add [rax],al
00000E88  0000              add [rax],al
00000E8A  0000              add [rax],al
00000E8C  0000              add [rax],al
00000E8E  0000              add [rax],al
00000E90  0000              add [rax],al
00000E92  0000              add [rax],al
00000E94  0000              add [rax],al
00000E96  0000              add [rax],al
00000E98  0000              add [rax],al
00000E9A  0000              add [rax],al
00000E9C  0000              add [rax],al
00000E9E  0000              add [rax],al
00000EA0  0000              add [rax],al
00000EA2  0000              add [rax],al
00000EA4  0000              add [rax],al
00000EA6  0000              add [rax],al
00000EA8  0000              add [rax],al
00000EAA  0000              add [rax],al
00000EAC  0000              add [rax],al
00000EAE  0000              add [rax],al
00000EB0  0000              add [rax],al
00000EB2  0000              add [rax],al
00000EB4  0000              add [rax],al
00000EB6  0000              add [rax],al
00000EB8  0000              add [rax],al
00000EBA  0000              add [rax],al
00000EBC  0000              add [rax],al
00000EBE  0000              add [rax],al
00000EC0  0000              add [rax],al
00000EC2  0000              add [rax],al
00000EC4  0000              add [rax],al
00000EC6  0000              add [rax],al
00000EC8  0000              add [rax],al
00000ECA  0000              add [rax],al
00000ECC  0000              add [rax],al
00000ECE  0000              add [rax],al
00000ED0  0000              add [rax],al
00000ED2  0000              add [rax],al
00000ED4  0000              add [rax],al
00000ED6  0000              add [rax],al
00000ED8  0000              add [rax],al
00000EDA  0000              add [rax],al
00000EDC  0000              add [rax],al
00000EDE  0000              add [rax],al
00000EE0  0000              add [rax],al
00000EE2  0000              add [rax],al
00000EE4  0000              add [rax],al
00000EE6  0000              add [rax],al
00000EE8  0000              add [rax],al
00000EEA  0000              add [rax],al
00000EEC  0000              add [rax],al
00000EEE  0000              add [rax],al
00000EF0  0000              add [rax],al
00000EF2  0000              add [rax],al
00000EF4  0000              add [rax],al
00000EF6  0000              add [rax],al
00000EF8  0000              add [rax],al
00000EFA  0000              add [rax],al
00000EFC  0000              add [rax],al
00000EFE  0000              add [rax],al
00000F00  0000              add [rax],al
00000F02  0000              add [rax],al
00000F04  0000              add [rax],al
00000F06  0000              add [rax],al
00000F08  0000              add [rax],al
00000F0A  0000              add [rax],al
00000F0C  0000              add [rax],al
00000F0E  0000              add [rax],al
00000F10  0000              add [rax],al
00000F12  0000              add [rax],al
00000F14  0000              add [rax],al
00000F16  0000              add [rax],al
00000F18  0000              add [rax],al
00000F1A  0000              add [rax],al
00000F1C  0000              add [rax],al
00000F1E  0000              add [rax],al
00000F20  0000              add [rax],al
00000F22  0000              add [rax],al
00000F24  0000              add [rax],al
00000F26  0000              add [rax],al
00000F28  0000              add [rax],al
00000F2A  0000              add [rax],al
00000F2C  0000              add [rax],al
00000F2E  0000              add [rax],al
00000F30  0000              add [rax],al
00000F32  0000              add [rax],al
00000F34  0000              add [rax],al
00000F36  0000              add [rax],al
00000F38  0000              add [rax],al
00000F3A  0000              add [rax],al
00000F3C  0000              add [rax],al
00000F3E  0000              add [rax],al
00000F40  0000              add [rax],al
00000F42  0000              add [rax],al
00000F44  0000              add [rax],al
00000F46  0000              add [rax],al
00000F48  0000              add [rax],al
00000F4A  0000              add [rax],al
00000F4C  0000              add [rax],al
00000F4E  0000              add [rax],al
00000F50  0000              add [rax],al
00000F52  0000              add [rax],al
00000F54  0000              add [rax],al
00000F56  0000              add [rax],al
00000F58  0000              add [rax],al
00000F5A  0000              add [rax],al
00000F5C  0000              add [rax],al
00000F5E  0000              add [rax],al
00000F60  0000              add [rax],al
00000F62  0000              add [rax],al
00000F64  0000              add [rax],al
00000F66  0000              add [rax],al
00000F68  0000              add [rax],al
00000F6A  0000              add [rax],al
00000F6C  0000              add [rax],al
00000F6E  0000              add [rax],al
00000F70  0000              add [rax],al
00000F72  0000              add [rax],al
00000F74  0000              add [rax],al
00000F76  0000              add [rax],al
00000F78  0000              add [rax],al
00000F7A  0000              add [rax],al
00000F7C  0000              add [rax],al
00000F7E  0000              add [rax],al
00000F80  0000              add [rax],al
00000F82  0000              add [rax],al
00000F84  0000              add [rax],al
00000F86  0000              add [rax],al
00000F88  0000              add [rax],al
00000F8A  0000              add [rax],al
00000F8C  0000              add [rax],al
00000F8E  0000              add [rax],al
00000F90  0000              add [rax],al
00000F92  0000              add [rax],al
00000F94  0000              add [rax],al
00000F96  0000              add [rax],al
00000F98  0000              add [rax],al
00000F9A  0000              add [rax],al
00000F9C  0000              add [rax],al
00000F9E  0000              add [rax],al
00000FA0  0000              add [rax],al
00000FA2  0000              add [rax],al
00000FA4  0000              add [rax],al
00000FA6  0000              add [rax],al
00000FA8  0000              add [rax],al
00000FAA  0000              add [rax],al
00000FAC  0000              add [rax],al
00000FAE  0000              add [rax],al
00000FB0  0000              add [rax],al
00000FB2  0000              add [rax],al
00000FB4  0000              add [rax],al
00000FB6  0000              add [rax],al
00000FB8  0000              add [rax],al
00000FBA  0000              add [rax],al
00000FBC  0000              add [rax],al
00000FBE  0000              add [rax],al
00000FC0  0000              add [rax],al
00000FC2  0000              add [rax],al
00000FC4  0000              add [rax],al
00000FC6  0000              add [rax],al
00000FC8  0000              add [rax],al
00000FCA  0000              add [rax],al
00000FCC  0000              add [rax],al
00000FCE  0000              add [rax],al
00000FD0  0000              add [rax],al
00000FD2  0000              add [rax],al
00000FD4  0000              add [rax],al
00000FD6  0000              add [rax],al
00000FD8  0000              add [rax],al
00000FDA  0000              add [rax],al
00000FDC  0000              add [rax],al
00000FDE  0000              add [rax],al
00000FE0  0000              add [rax],al
00000FE2  0000              add [rax],al
00000FE4  0000              add [rax],al
00000FE6  0000              add [rax],al
00000FE8  0000              add [rax],al
00000FEA  0000              add [rax],al
00000FEC  0000              add [rax],al
00000FEE  0000              add [rax],al
00000FF0  0000              add [rax],al
00000FF2  0000              add [rax],al
00000FF4  0000              add [rax],al
00000FF6  0000              add [rax],al
00000FF8  0000              add [rax],al
00000FFA  0000              add [rax],al
00000FFC  0000              add [rax],al
00000FFE  0000              add [rax],al
00001000  0302              add eax,[rdx]
00001002  660F1F440000      nop word [rax+rax+0x0]
00001008  50                push rax
00001009  190400            sbb [rax+rax],eax
0000100C  0000              add [rax],al
0000100E  0000              add [rax],al
00001010  0300              add eax,[rax]
00001012  0000              add [rax],al
00001014  0000              add [rax],al
00001016  0000              add [rax],al
00001018  7506              jnz 0x1020
0000101A  1000              adc [rax],al
0000101C  0000              add [rax],al
0000101E  0000              add [rax],al
00001020  9D                popf
00001021  06                db 0x06
00001022  1000              adc [rax],al
00001024  0000              add [rax],al
00001026  0000              add [rax],al
00001028  9D                popf
00001029  06                db 0x06
0000102A  1000              adc [rax],al
0000102C  0000              add [rax],al
0000102E  0000              add [rax],al
00001030  9D                popf
00001031  06                db 0x06
00001032  1000              adc [rax],al
00001034  0000              add [rax],al
00001036  0000              add [rax],al
00001038  9D                popf
00001039  06                db 0x06
0000103A  1000              adc [rax],al
0000103C  0000              add [rax],al
0000103E  0000              add [rax],al
00001040  9D                popf
00001041  06                db 0x06
00001042  1000              adc [rax],al
00001044  0000              add [rax],al
00001046  0000              add [rax],al
00001048  9D                popf
00001049  06                db 0x06
0000104A  1000              adc [rax],al
0000104C  0000              add [rax],al
0000104E  0000              add [rax],al
00001050  9D                popf
00001051  06                db 0x06
00001052  1000              adc [rax],al
00001054  0000              add [rax],al
00001056  0000              add [rax],al
00001058  9D                popf
00001059  06                db 0x06
0000105A  1000              adc [rax],al
0000105C  0000              add [rax],al
0000105E  0000              add [rax],al
00001060  9D                popf
00001061  06                db 0x06
00001062  1000              adc [rax],al
00001064  0000              add [rax],al
00001066  0000              add [rax],al
00001068  9D                popf
00001069  06                db 0x06
0000106A  1000              adc [rax],al
0000106C  0000              add [rax],al
0000106E  0000              add [rax],al
00001070  9D                popf
00001071  06                db 0x06
00001072  1000              adc [rax],al
00001074  0000              add [rax],al
00001076  0000              add [rax],al
00001078  9D                popf
00001079  06                db 0x06
0000107A  1000              adc [rax],al
0000107C  0000              add [rax],al
0000107E  0000              add [rax],al
00001080  9D                popf
00001081  06                db 0x06
00001082  1000              adc [rax],al
00001084  0000              add [rax],al
00001086  0000              add [rax],al
00001088  9D                popf
00001089  06                db 0x06
0000108A  1000              adc [rax],al
0000108C  0000              add [rax],al
0000108E  0000              add [rax],al
00001090  9D                popf
00001091  06                db 0x06
00001092  1000              adc [rax],al
00001094  0000              add [rax],al
00001096  0000              add [rax],al
00001098  9D                popf
00001099  06                db 0x06
0000109A  1000              adc [rax],al
0000109C  0000              add [rax],al
0000109E  0000              add [rax],al
000010A0  9D                popf
000010A1  06                db 0x06
000010A2  1000              adc [rax],al
000010A4  0000              add [rax],al
000010A6  0000              add [rax],al
000010A8  9D                popf
000010A9  06                db 0x06
000010AA  1000              adc [rax],al
000010AC  0000              add [rax],al
000010AE  0000              add [rax],al
000010B0  9D                popf
000010B1  06                db 0x06
000010B2  1000              adc [rax],al
000010B4  0000              add [rax],al
000010B6  0000              add [rax],al
000010B8  9D                popf
000010B9  06                db 0x06
000010BA  1000              adc [rax],al
000010BC  0000              add [rax],al
000010BE  0000              add [rax],al
000010C0  9D                popf
000010C1  06                db 0x06
000010C2  1000              adc [rax],al
000010C4  0000              add [rax],al
000010C6  0000              add [rax],al
000010C8  9D                popf
000010C9  06                db 0x06
000010CA  1000              adc [rax],al
000010CC  0000              add [rax],al
000010CE  0000              add [rax],al
000010D0  9D                popf
000010D1  06                db 0x06
000010D2  1000              adc [rax],al
000010D4  0000              add [rax],al
000010D6  0000              add [rax],al
000010D8  9D                popf
000010D9  06                db 0x06
000010DA  1000              adc [rax],al
000010DC  0000              add [rax],al
000010DE  0000              add [rax],al
000010E0  9D                popf
000010E1  06                db 0x06
000010E2  1000              adc [rax],al
000010E4  0000              add [rax],al
000010E6  0000              add [rax],al
000010E8  9D                popf
000010E9  06                db 0x06
000010EA  1000              adc [rax],al
000010EC  0000              add [rax],al
000010EE  0000              add [rax],al
000010F0  9D                popf
000010F1  06                db 0x06
000010F2  1000              adc [rax],al
000010F4  0000              add [rax],al
000010F6  0000              add [rax],al
000010F8  9D                popf
000010F9  06                db 0x06
000010FA  1000              adc [rax],al
000010FC  0000              add [rax],al
000010FE  0000              add [rax],al
00001100  9D                popf
00001101  06                db 0x06
00001102  1000              adc [rax],al
00001104  0000              add [rax],al
00001106  0000              add [rax],al
00001108  9D                popf
00001109  06                db 0x06
0000110A  1000              adc [rax],al
0000110C  0000              add [rax],al
0000110E  0000              add [rax],al
00001110  9D                popf
00001111  06                db 0x06
00001112  1000              adc [rax],al
00001114  0000              add [rax],al
00001116  0000              add [rax],al
00001118  9D                popf
00001119  06                db 0x06
0000111A  1000              adc [rax],al
0000111C  0000              add [rax],al
0000111E  0000              add [rax],al
00001120  9D                popf
00001121  06                db 0x06
00001122  1000              adc [rax],al
00001124  0000              add [rax],al
00001126  0000              add [rax],al
00001128  9D                popf
00001129  06                db 0x06
0000112A  1000              adc [rax],al
0000112C  0000              add [rax],al
0000112E  0000              add [rax],al
00001130  9D                popf
00001131  06                db 0x06
00001132  1000              adc [rax],al
00001134  0000              add [rax],al
00001136  0000              add [rax],al
00001138  9D                popf
00001139  06                db 0x06
0000113A  1000              adc [rax],al
0000113C  0000              add [rax],al
0000113E  0000              add [rax],al
00001140  9D                popf
00001141  06                db 0x06
00001142  1000              adc [rax],al
00001144  0000              add [rax],al
00001146  0000              add [rax],al
00001148  0106              add [rsi],eax
0000114A  1000              adc [rax],al
0000114C  0000              add [rax],al
0000114E  0000              add [rax],al
00001150  9D                popf
00001151  06                db 0x06
00001152  1000              adc [rax],al
00001154  0000              add [rax],al
00001156  0000              add [rax],al
00001158  9D                popf
00001159  06                db 0x06
0000115A  1000              adc [rax],al
0000115C  0000              add [rax],al
0000115E  0000              add [rax],al
00001160  2D06100000        sub eax,0x1006
00001165  0000              add [rax],al
00001167  009D06100000      add [rbp+0x1006],bl
0000116D  0000              add [rax],al
0000116F  009D06100000      add [rbp+0x1006],bl
00001175  0000              add [rax],al
00001177  009D06100000      add [rbp+0x1006],bl
0000117D  0000              add [rax],al
0000117F  009D06100000      add [rbp+0x1006],bl
00001185  0000              add [rax],al
00001187  009D06100000      add [rbp+0x1006],bl
0000118D  0000              add [rax],al
0000118F  009D06100000      add [rbp+0x1006],bl
00001195  0000              add [rax],al
00001197  009D06100000      add [rbp+0x1006],bl
0000119D  0000              add [rax],al
0000119F  009D06100000      add [rbp+0x1006],bl
000011A5  0000              add [rax],al
000011A7  009D06100000      add [rbp+0x1006],bl
000011AD  0000              add [rax],al
000011AF  009D06100000      add [rbp+0x1006],bl
000011B5  0000              add [rax],al
000011B7  009D06100000      add [rbp+0x1006],bl
000011BD  0000              add [rax],al
000011BF  009D06100000      add [rbp+0x1006],bl
000011C5  0000              add [rax],al
000011C7  009D06100000      add [rbp+0x1006],bl
000011CD  0000              add [rax],al
000011CF  004106            add [rcx+0x6],al
000011D2  1000              adc [rax],al
000011D4  0000              add [rax],al
000011D6  0000              add [rax],al
000011D8  1906              sbb [rsi],eax
000011DA  1000              adc [rax],al
000011DC  0000              add [rax],al
000011DE  0000              add [rax],al
000011E0  9D                popf
000011E1  06                db 0x06
000011E2  1000              adc [rax],al
000011E4  0000              add [rax],al
000011E6  0000              add [rax],al
000011E8  9D                popf
000011E9  06                db 0x06
000011EA  1000              adc [rax],al
000011EC  0000              add [rax],al
000011EE  0000              add [rax],al
000011F0  9D                popf
000011F1  06                db 0x06
000011F2  1000              adc [rax],al
000011F4  0000              add [rax],al
000011F6  0000              add [rax],al
000011F8  E605              out 0x5,al
000011FA  1000              adc [rax],al
000011FC  0000              add [rax],al
000011FE  0000              add [rax],al
00001200  9D                popf
00001201  06                db 0x06
00001202  1000              adc [rax],al
00001204  0000              add [rax],al
00001206  0000              add [rax],al
00001208  9D                popf
00001209  06                db 0x06
0000120A  1000              adc [rax],al
0000120C  0000              add [rax],al
0000120E  0000              add [rax],al
00001210  9D                popf
00001211  06                db 0x06
00001212  1000              adc [rax],al
00001214  0000              add [rax],al
00001216  0000              add [rax],al
00001218  9D                popf
00001219  06                db 0x06
0000121A  1000              adc [rax],al
0000121C  0000              add [rax],al
0000121E  0000              add [rax],al
00001220  9D                popf
00001221  06                db 0x06
00001222  1000              adc [rax],al
00001224  0000              add [rax],al
00001226  0000              add [rax],al
00001228  9D                popf
00001229  06                db 0x06
0000122A  1000              adc [rax],al
0000122C  0000              add [rax],al
0000122E  0000              add [rax],al
00001230  9D                popf
00001231  06                db 0x06
00001232  1000              adc [rax],al
00001234  0000              add [rax],al
00001236  0000              add [rax],al
00001238  9D                popf
00001239  06                db 0x06
0000123A  1000              adc [rax],al
0000123C  0000              add [rax],al
0000123E  0000              add [rax],al
00001240  9D                popf
00001241  06                db 0x06
00001242  1000              adc [rax],al
00001244  0000              add [rax],al
00001246  0000              add [rax],al
00001248  9D                popf
00001249  06                db 0x06
0000124A  1000              adc [rax],al
0000124C  0000              add [rax],al
0000124E  0000              add [rax],al
00001250  9D                popf
00001251  06                db 0x06
00001252  1000              adc [rax],al
00001254  0000              add [rax],al
00001256  0000              add [rax],al
00001258  9D                popf
00001259  06                db 0x06
0000125A  1000              adc [rax],al
0000125C  0000              add [rax],al
0000125E  0000              add [rax],al
00001260  E605              out 0x5,al
00001262  1000              adc [rax],al
00001264  0000              add [rax],al
00001266  0000              add [rax],al
00001268  1906              sbb [rsi],eax
0000126A  1000              adc [rax],al
0000126C  0000              add [rax],al
0000126E  0000              add [rax],al
00001270  0106              add [rsi],eax
00001272  1000              adc [rax],al
00001274  0000              add [rax],al
00001276  0000              add [rax],al
00001278  2D06100000        sub eax,0x1006
0000127D  0000              add [rax],al
0000127F  005019            add [rax+0x19],dl
00001282  0450              add al,0x50
00001284  190466            sbb [rsi],eax
00001287  90                nop
00001288  50                push rax
00001289  190420            sbb [rax],eax
0000128C  2000              and [rax],al
0000128E  2000              and [rax],al
00001290  48                rex.w
00001291  65                gs
00001292  61                db 0x61
00001293  7020              jo 0x12b5
00001295  63                db 0x63
00001296  6F                outsd
00001297  7272              jc 0x130b
00001299  7570              jnz 0x130b
0000129B  7469              jz 0x1306
0000129D  6F                outsd
0000129E  6E                outsb
0000129F  20646574          and [rbp+0x74],ah
000012A3  65                gs
000012A4  63                db 0x63
000012A5  7465              jz 0x130c
000012A7  64207768          and [fs:rdi+0x68],dh
000012AB  696C652074727969  imul ebp,[rbp+0x20],dword 0x69797274
000012B3  6E                outsb
000012B4  6720746F20        and [edi+ebp*2+0x20],dh
000012B9  64                fs
000012BA  65                gs
000012BB  61                db 0x61
000012BC  6C                insb
000012BD  6C                insb
000012BE  6F                outsd
000012BF  63                db 0x63
000012C0  61                db 0x61
000012C1  7465              jz 0x1328
000012C3  2000              and [rax],al
000012C5  466F              outsd
000012C7  756E              jnz 0x1337
000012C9  64207374          and [fs:rbx+0x74],dh
000012CD  61                db 0x61
000012CE  7274              jc 0x1344
000012D0  207369            and [rbx+0x69],dh
000012D3  676E              a32 outsb
000012D5  61                db 0x61
000012D6  7475              jz 0x134d
000012D8  7265              jc 0x133f
000012DA  2000              and [rax],al
000012DC  20696E            and [rcx+0x6e],ch
000012DF  7374              jnc 0x1355
000012E1  65                gs
000012E2  61                db 0x61
000012E3  64206F66          and [fs:rdi+0x66],ch
000012E7  2000              and [rax],al
000012E9  466F              outsd
000012EB  756E              jnz 0x135b
000012ED  6420656E          and [fs:rbp+0x6e],ah
000012F1  64207369          and [fs:rbx+0x69],dh
000012F5  676E              a32 outsb
000012F7  61                db 0x61
000012F8  7475              jz 0x136f
000012FA  7265              jc 0x1361
000012FC  2000              and [rax],al
000012FE  4D                rex.wrb
000012FF  656D              gs insd
00001301  6F                outsd
00001302  7279              jc 0x137d
00001304  2064756D          and [rbp+rsi*2+0x6d],ah
00001308  703A              jo 0x1344
0000130A  0A00              or al,[rax]
0000130C  416C              insb
0000130E  6C                insb
0000130F  6F                outsd
00001310  63                db 0x63
00001311  61                db 0x61
00001312  7469              jz 0x137d
00001314  6F                outsd
00001315  6E                outsb
00001316  206F66            and [rdi+0x66],ch
00001319  2000              and [rax],al
0000131B  62                db 0x62
0000131C  7974              jns 0x1392
0000131E  657320            gs jnc 0x1341
00001321  61                db 0x61
00001322  7420              jz 0x1344
00001324  003A              add [rdx],bh
00001326  0A00              or al,[rax]
00001328  57                push rdi
00001329  61                db 0x61
0000132A  726E              jc 0x139a
0000132C  696E673A204D65    imul ebp,[rsi+0x67],dword 0x654d203a
00001333  6D                insd
00001334  6F                outsd
00001335  7279              jc 0x13b0
00001337  206675            and [rsi+0x75],ah
0000133A  6C                insb
0000133B  6C                insb
0000133C  210A              and [rdx],ecx
0000133E  009000000000      add [rax+0x0],dl
00001344  0000              add [rax],al
00001346  0000              add [rax],al
00001348  0000              add [rax],al
0000134A  0000              add [rax],al
0000134C  0000              add [rax],al
0000134E  0000              add [rax],al
00001350  0000              add [rax],al
00001352  0000              add [rax],al
00001354  0000              add [rax],al
00001356  0000              add [rax],al
00001358  55                push rbp
00001359  4889E5            mov rbp,rsp
0000135C  48897DF8          mov [rbp-0x8],rdi
00001360  488B45F8          mov rax,[rbp-0x8]
00001364  0FB600            movzx eax,byte [rax]
00001367  83E07F            and eax,byte +0x7f
0000136A  5D                pop rbp
0000136B  C3                ret
0000136C  55                push rbp
0000136D  4889E5            mov rbp,rsp
00001370  53                push rbx
00001371  B801000000        mov eax,0x1
00001376  BA00000000        mov edx,0x0
0000137B  89D3              mov ebx,edx
0000137D  CD30              int 0x30
0000137F  90                nop
00001380  5B                pop rbx
00001381  5D                pop rbp
00001382  C3                ret
00001383  55                push rbp
00001384  4889E5            mov rbp,rsp
00001387  53                push rbx
00001388  48897DF0          mov [rbp-0x10],rdi
0000138C  B801000000        mov eax,0x1
00001391  B901000000        mov ecx,0x1
00001396  488B55F0          mov rdx,[rbp-0x10]
0000139A  89CB              mov ebx,ecx
0000139C  4889D7            mov rdi,rdx
0000139F  CD30              int 0x30
000013A1  90                nop
000013A2  5B                pop rbx
000013A3  5D                pop rbp
000013A4  C3                ret
000013A5  55                push rbp
000013A6  4889E5            mov rbp,rsp
000013A9  53                push rbx
000013AA  89F1              mov ecx,esi
000013AC  89D0              mov eax,edx
000013AE  89FA              mov edx,edi
000013B0  8855F4            mov [rbp-0xc],dl
000013B3  89CA              mov edx,ecx
000013B5  8855F0            mov [rbp-0x10],dl
000013B8  8845EC            mov [rbp-0x14],al
000013BB  B801000000        mov eax,0x1
000013C0  41B803000000      mov r8d,0x3
000013C6  0FB64DF4          movzx ecx,byte [rbp-0xc]
000013CA  0FB675F0          movzx esi,byte [rbp-0x10]
000013CE  0FB655EC          movzx edx,byte [rbp-0x14]
000013D2  4489C3            mov ebx,r8d
000013D5  89CF              mov edi,ecx
000013D7  CD30              int 0x30
000013D9  90                nop
000013DA  5B                pop rbx
000013DB  5D                pop rbp
000013DC  C3                ret
000013DD  55                push rbp
000013DE  4889E5            mov rbp,rsp
000013E1  53                push rbx
000013E2  89FA              mov edx,edi
000013E4  89F0              mov eax,esi
000013E6  8855F4            mov [rbp-0xc],dl
000013E9  8845F0            mov [rbp-0x10],al
000013EC  B805000000        mov eax,0x5
000013F1  41B800000000      mov r8d,0x0
000013F7  0FB655F4          movzx edx,byte [rbp-0xc]
000013FB  0FB64DF0          movzx ecx,byte [rbp-0x10]
000013FF  4489C3            mov ebx,r8d
00001402  89D7              mov edi,edx
00001404  89CE              mov esi,ecx
00001406  CD30              int 0x30
00001408  90                nop
00001409  5B                pop rbx
0000140A  5D                pop rbp
0000140B  C3                ret
0000140C  55                push rbp
0000140D  4889E5            mov rbp,rsp
00001410  53                push rbx
00001411  B805000000        mov eax,0x5
00001416  BA01000000        mov edx,0x1
0000141B  89D3              mov ebx,edx
0000141D  CD30              int 0x30
0000141F  90                nop
00001420  5B                pop rbx
00001421  5D                pop rbp
00001422  C3                ret
00001423  55                push rbp
00001424  4889E5            mov rbp,rsp
00001427  53                push rbx
00001428  B802000000        mov eax,0x2
0000142D  BA01000000        mov edx,0x1
00001432  89D3              mov ebx,edx
00001434  CD30              int 0x30
00001436  668945F6          mov [rbp-0xa],ax
0000143A  488D45F6          lea rax,[rbp-0xa]
0000143E  0FB700            movzx eax,word [rax]
00001441  5B                pop rbx
00001442  5D                pop rbp
00001443  C3                ret
00001444  55                push rbp
00001445  4889E5            mov rbp,rsp
00001448  53                push rbx
00001449  B806000000        mov eax,0x6
0000144E  BA00000000        mov edx,0x0
00001453  89D3              mov ebx,edx
00001455  CD30              int 0x30
00001457  488945F0          mov [rbp-0x10],rax
0000145B  488B45F0          mov rax,[rbp-0x10]
0000145F  5B                pop rbx
00001460  5D                pop rbp
00001461  C3                ret
00001462  55                push rbp
00001463  4889E5            mov rbp,rsp
00001466  897DFC            mov [rbp-0x4],edi
00001469  837DFC09          cmp dword [rbp-0x4],byte +0x9
0000146D  7F08              jg 0x1477
0000146F  8B45FC            mov eax,[rbp-0x4]
00001472  83C030            add eax,byte +0x30
00001475  EB06              jmp short 0x147d
00001477  8B45FC            mov eax,[rbp-0x4]
0000147A  83C037            add eax,byte +0x37
0000147D  5D                pop rbp
0000147E  C3                ret
0000147F  55                push rbp
00001480  4889E5            mov rbp,rsp
00001483  53                push rbx
00001484  4883EC28          sub rsp,byte +0x28
00001488  48897DD8          mov [rbp-0x28],rdi
0000148C  488975D0          mov [rbp-0x30],rsi
00001490  C745EC0F000000    mov dword [rbp-0x14],0xf
00001497  837DEC00          cmp dword [rbp-0x14],byte +0x0
0000149B  7829              js 0x14c6
0000149D  488B45D0          mov rax,[rbp-0x30]
000014A1  83E00F            and eax,byte +0xf
000014A4  8B55EC            mov edx,[rbp-0x14]
000014A7  4863CA            movsxd rcx,edx
000014AA  488B55D8          mov rdx,[rbp-0x28]
000014AE  488D1C11          lea rbx,[rcx+rdx]
000014B2  89C7              mov edi,eax
000014B4  E8A9FFFFFF        call 0x1462
000014B9  8803              mov [rbx],al
000014BB  48C16DD004        shr qword [rbp-0x30],byte 0x4
000014C0  836DEC01          sub dword [rbp-0x14],byte +0x1
000014C4  EBD1              jmp short 0x1497
000014C6  488B45D8          mov rax,[rbp-0x28]
000014CA  4883C010          add rax,byte +0x10
000014CE  C60000            mov byte [rax],0x0
000014D1  90                nop
000014D2  4883C428          add rsp,byte +0x28
000014D6  5B                pop rbx
000014D7  5D                pop rbp
000014D8  C3                ret
000014D9  90                nop
000014DA  55                push rbp
000014DB  4889E5            mov rbp,rsp
000014DE  4883EC30          sub rsp,byte +0x30
000014E2  48897DD8          mov [rbp-0x28],rdi
000014E6  488975D0          mov [rbp-0x30],rsi
000014EA  488B55D0          mov rdx,[rbp-0x30]
000014EE  488D45E0          lea rax,[rbp-0x20]
000014F2  4889D6            mov rsi,rdx
000014F5  4889C7            mov rdi,rax
000014F8  E882FFFFFF        call 0x147f
000014FD  488D45E0          lea rax,[rbp-0x20]
00001501  4889C7            mov rdi,rax
00001504  E87AFEFFFF        call 0x1383
00001509  488B45D8          mov rax,[rbp-0x28]
0000150D  C9                leave
0000150E  C3                ret
0000150F  90                nop
00001510  55                push rbp
00001511  4889E5            mov rbp,rsp
00001514  48897DF8          mov [rbp-0x8],rdi
00001518  89F1              mov ecx,esi
0000151A  89D0              mov eax,edx
0000151C  89CA              mov edx,ecx
0000151E  8855F4            mov [rbp-0xc],dl
00001521  8845F0            mov [rbp-0x10],al
00001524  488B45F8          mov rax,[rbp-0x8]
00001528  0FB655F4          movzx edx,byte [rbp-0xc]
0000152C  8810              mov [rax],dl
0000152E  488B45F8          mov rax,[rbp-0x8]
00001532  0FB655F0          movzx edx,byte [rbp-0x10]
00001536  885001            mov [rax+0x1],dl
00001539  90                nop
0000153A  5D                pop rbp
0000153B  C3                ret
0000153C  0F1F4000          nop dword [rax+0x0]
00001540  3B07              cmp eax,[rdi]
00001542  1000              adc [rax],al
00001544  0000              add [rax],al
00001546  0000              add [rax],al
00001548  1400              adc al,0x0
0000154A  0000              add [rax],al
0000154C  0000              add [rax],al
0000154E  0000              add [rax],al
00001550  017A52            add [rdx+0x52],edi
00001553  0001              add [rcx],al
00001555  7810              js 0x1567
00001557  011B              add [rbx],ebx
00001559  0C07              or al,0x7
0000155B  08900100001C      or [rax+0x1c000001],dl
00001561  0000              add [rax],al
00001563  001C00            add [rax+rax],bl
00001566  0000              add [rax],al
00001568  F0FD              lock std
0000156A  FF                db 0xff
0000156B  FF1400            call [rax+rax]
0000156E  0000              add [rax],al
00001570  00410E            add [rcx+0xe],al
00001573  108602430D06      adc [rsi+0x60d4302],al
00001579  4F0C07            o64 or al,0x7
0000157C  0800              or [rax],al
0000157E  0000              add [rax],al
00001580  1C00              sbb al,0x0
00001582  0000              add [rax],al
00001584  3C00              cmp al,0x0
00001586  0000              add [rax],al
00001588  E4FD              in al,0xfd
0000158A  FF                db 0xff
0000158B  FF17              call [rdi]
0000158D  0000              add [rax],al
0000158F  0000              add [rax],al
00001591  41                rex.b
00001592  0E                db 0x0e
00001593  108602430D06      adc [rsi+0x60d4302],al
00001599  41830351          add dword [r11],byte +0x51
0000159D  0C07              or al,0x7
0000159F  081C00            or [rax+rax],bl
000015A2  0000              add [rax],al
000015A4  5C                pop rsp
000015A5  0000              add [rax],al
000015A7  00DB              add bl,bl
000015A9  FD                std
000015AA  FF                db 0xff
000015AB  FF22              jmp [rdx]
000015AD  0000              add [rax],al
000015AF  0000              add [rax],al
000015B1  41                rex.b
000015B2  0E                db 0x0e
000015B3  108602430D06      adc [rsi+0x60d4302],al
000015B9  4183035C          add dword [r11],byte +0x5c
000015BD  0C07              or al,0x7
000015BF  081C00            or [rax+rax],bl
000015C2  0000              add [rax],al
000015C4  7C00              jl 0x15c6
000015C6  0000              add [rax],al
000015C8  DD                db 0xdd
000015C9  FD                std
000015CA  FF                db 0xff
000015CB  FF                db 0xff
000015CC  3800              cmp [rax],al
000015CE  0000              add [rax],al
000015D0  00410E            add [rcx+0xe],al
000015D3  108602430D06      adc [rsi+0x60d4302],al
000015D9  41830372          add dword [r11],byte +0x72
000015DD  0C07              or al,0x7
000015DF  081C00            or [rax+rax],bl
000015E2  0000              add [rax],al
000015E4  9C                pushf
000015E5  0000              add [rax],al
000015E7  00F5              add ch,dh
000015E9  FD                std
000015EA  FF                db 0xff
000015EB  FF2F              jmp dword far [rdi]
000015ED  0000              add [rax],al
000015EF  0000              add [rax],al
000015F1  41                rex.b
000015F2  0E                db 0x0e
000015F3  108602430D06      adc [rsi+0x60d4302],al
000015F9  41830369          add dword [r11],byte +0x69
000015FD  0C07              or al,0x7
000015FF  081C00            or [rax+rax],bl
00001602  0000              add [rax],al
00001604  BC00000004        mov esp,0x4000000
00001609  FE                db 0xfe
0000160A  FF                db 0xff
0000160B  FF17              call [rdi]
0000160D  0000              add [rax],al
0000160F  0000              add [rax],al
00001611  41                rex.b
00001612  0E                db 0x0e
00001613  108602430D06      adc [rsi+0x60d4302],al
00001619  41830351          add dword [r11],byte +0x51
0000161D  0C07              or al,0x7
0000161F  081C00            or [rax+rax],bl
00001622  0000              add [rax],al
00001624  DC00              fadd qword [rax]
00001626  0000              add [rax],al
00001628  FB                sti
00001629  FD                std
0000162A  FF                db 0xff
0000162B  FF21              jmp [rcx]
0000162D  0000              add [rax],al
0000162F  0000              add [rax],al
00001631  41                rex.b
00001632  0E                db 0x0e
00001633  108602430D06      adc [rsi+0x60d4302],al
00001639  4183035B          add dword [r11],byte +0x5b
0000163D  0C07              or al,0x7
0000163F  081C00            or [rax+rax],bl
00001642  0000              add [rax],al
00001644  FC                cld
00001645  0000              add [rax],al
00001647  00FC              add ah,bh
00001649  FD                std
0000164A  FF                db 0xff
0000164B  FF1E              call dword far [rsi]
0000164D  0000              add [rax],al
0000164F  0000              add [rax],al
00001651  41                rex.b
00001652  0E                db 0x0e
00001653  108602430D06      adc [rsi+0x60d4302],al
00001659  41830358          add dword [r11],byte +0x58
0000165D  0C07              or al,0x7
0000165F  081C00            or [rax+rax],bl
00001662  0000              add [rax],al
00001664  1C01              sbb al,0x1
00001666  0000              add [rax],al
00001668  FA                cli
00001669  FD                std
0000166A  FF                db 0xff
0000166B  FF1D00000000      call dword far [rel 0x1671]
00001671  41                rex.b
00001672  0E                db 0x0e
00001673  108602430D06      adc [rsi+0x60d4302],al
00001679  58                pop rax
0000167A  0C07              or al,0x7
0000167C  0800              or [rax],al
0000167E  0000              add [rax],al
00001680  2000              and [rax],al
00001682  0000              add [rax],al
00001684  3C01              cmp al,0x1
00001686  0000              add [rax],al
00001688  F7FD              idiv ebp
0000168A  FF                db 0xff
0000168B  FF5A00            call dword far [rdx+0x0]
0000168E  0000              add [rax],al
00001690  00410E            add [rcx+0xe],al
00001693  108602430D06      adc [rsi+0x60d4302],al
00001699  45830302          add dword [r11],byte +0x2
0000169D  50                push rax
0000169E  0C07              or al,0x7
000016A0  0800              or [rax],al
000016A2  0000              add [rax],al
000016A4  1C00              sbb al,0x0
000016A6  0000              add [rax],al
000016A8  60                db 0x60
000016A9  0100              add [rax],eax
000016AB  002E              add [rsi],ch
000016AD  FE                db 0xfe
000016AE  FF                db 0xff
000016AF  FF3500000000      push qword [rel 0x16b5]
000016B5  41                rex.b
000016B6  0E                db 0x0e
000016B7  108602430D06      adc [rsi+0x60d4302],al
000016BD  700C              jo 0x16cb
000016BF  07                db 0x07
000016C0  0800              or [rax],al
000016C2  0000              add [rax],al
000016C4  1C00              sbb al,0x0
000016C6  0000              add [rax],al
000016C8  800100            add byte [rcx],0x0
000016CB  003CE9            add [rcx+rbp*8],bh
000016CE  FF                db 0xff
000016CF  FFA900000000      jmp dword far [rcx+0x0]
000016D5  41                rex.b
000016D6  0E                db 0x0e
000016D7  108602430D06      adc [rsi+0x60d4302],al
000016DD  02A40C07080000    add ah,[rsp+rcx+0x807]
000016E4  1C00              sbb al,0x0
000016E6  0000              add [rax],al
000016E8  A0010000C5E9FFFF  mov al,[qword 0xd9ffffe9c5000001]
         -D9
000016F1  0000              add [rax],al
000016F3  0000              add [rax],al
000016F5  41                rex.b
000016F6  0E                db 0x0e
000016F7  108602430D06      adc [rsi+0x60d4302],al
000016FD  02D4              add dl,ah
000016FF  0C07              or al,0x7
00001701  0800              or [rax],al
00001703  001C00            add [rax+rax],bl
00001706  0000              add [rax],al
00001708  C00100            rol byte [rcx],byte 0x0
0000170B  007EEA            add [rsi-0x16],bh
0000170E  FF                db 0xff
0000170F  FFB000000000      push qword [rax+0x0]
00001715  41                rex.b
00001716  0E                db 0x0e
00001717  108602430D06      adc [rsi+0x60d4302],al
0000171D  02AB0C070800      add ch,[rbx+0x8070c]
00001723  001C00            add [rax+rax],bl
00001726  0000              add [rax],al
00001728  E001              loopne 0x172b
0000172A  0000              add [rax],al
0000172C  0E                db 0x0e
0000172D  EBFF              jmp short 0x172e
0000172F  FFF2              push rdx
00001731  0200              add al,[rax]
00001733  0000              add [rax],al
00001735  41                rex.b
00001736  0E                db 0x0e
00001737  108602430D06      adc [rsi+0x60d4302],al
0000173D  03ED              add ebp,ebp
0000173F  020C07            add cl,[rdi+rax]
00001742  0800              or [rax],al
00001744  2000              and [rax],al
00001746  0000              add [rax],al
00001748  0002              add [rdx],al
0000174A  0000              add [rax],al
0000174C  E0ED              loopne 0x173b
0000174E  FF                db 0xff
0000174F  FFC7              inc edi
00001751  0100              add [rax],eax
00001753  0000              add [rax],al
00001755  41                rex.b
00001756  0E                db 0x0e
00001757  108602430D06      adc [rsi+0x60d4302],al
0000175D  45830303          add dword [r11],byte +0x3
00001761  BD010C0708        mov ebp,0x8070c01
00001766  0000              add [rax],al
00001768  1C00              sbb al,0x0
0000176A  0000              add [rax],al
0000176C  2402              and al,0x2
0000176E  0000              add [rax],al
00001770  A0FDFFFF2C000000  mov al,[qword 0x2cfffffd]
         -00
00001779  41                rex.b
0000177A  0E                db 0x0e
0000177B  108602430D06      adc [rsi+0x60d4302],al
00001781  670C07            or al,0x7
00001784  0800              or [rax],al
00001786  0000              add [rax],al
00001788  1C00              sbb al,0x0
0000178A  0000              add [rax],al
0000178C  440200            add r8b,[rax]
0000178F  0063EF            add [rbx-0x11],ah
00001792  FF                db 0xff
00001793  FF4800            dec dword [rax+0x0]
00001796  0000              add [rax],al
00001798  00410E            add [rcx+0xe],al
0000179B  108602430D06      adc [rsi+0x60d4302],al
000017A1  02430C            add al,[rbx+0xc]
000017A4  07                db 0x07
000017A5  0800              or [rax],al
000017A7  001C00            add [rax+rax],bl
000017AA  0000              add [rax],al
000017AC  640200            add al,[fs:rax]
000017AF  008BEFFFFF15      add [rbx+0x15ffffef],cl
000017B5  0000              add [rax],al
000017B7  0000              add [rax],al
000017B9  41                rex.b
000017BA  0E                db 0x0e
000017BB  108602430D06      adc [rsi+0x60d4302],al
000017C1  50                push rax
000017C2  0C07              or al,0x7
000017C4  0800              or [rax],al
000017C6  0000              add [rax],al
000017C8  55                push rbp
000017C9  4889E5            mov rbp,rsp
000017CC  48897DE8          mov [rbp-0x18],rdi
000017D0  488975E0          mov [rbp-0x20],rsi
000017D4  488B45E8          mov rax,[rbp-0x18]
000017D8  488945F8          mov [rbp-0x8],rax
000017DC  488B45E0          mov rax,[rbp-0x20]
000017E0  488D50E8          lea rdx,[rax-0x18]
000017E4  488B45F8          mov rax,[rbp-0x8]
000017E8  488910            mov [rax],rdx
000017EB  488B45F8          mov rax,[rbp-0x8]
000017EF  48C7400800000000  mov qword [rax+0x8],0x0
000017F7  488B45F8          mov rax,[rbp-0x8]
000017FB  48C7401000000000  mov qword [rax+0x10],0x0
00001803  488B45F8          mov rax,[rbp-0x8]
00001807  5D                pop rbp
00001808  C3                ret
00001809  55                push rbp
0000180A  4889E5            mov rbp,rsp
0000180D  53                push rbx
0000180E  897DF4            mov [rbp-0xc],edi
00001811  B804000000        mov eax,0x4
00001816  B900000000        mov ecx,0x0
0000181B  8B55F4            mov edx,[rbp-0xc]
0000181E  89CB              mov ebx,ecx
00001820  89D7              mov edi,edx
00001822  CD30              int 0x30
00001824  90                nop
00001825  5B                pop rbx
00001826  5D                pop rbp
00001827  C3                ret
00001828  1400              adc al,0x0
0000182A  0000              add [rax],al
0000182C  0000              add [rax],al
0000182E  0000              add [rax],al
00001830  017A52            add [rdx+0x52],edi
00001833  0001              add [rcx],al
00001835  7810              js 0x1847
00001837  011B              add [rbx],ebx
00001839  0C07              or al,0x7
0000183B  08900100001C      or [rax+0x1c000001],dl
00001841  0000              add [rax],al
00001843  001C00            add [rax+rax],bl
00001846  0000              add [rax],al
00001848  80FFFF            cmp bh,0xff
0000184B  FF4100            inc dword [rcx+0x0]
0000184E  0000              add [rax],al
00001850  00410E            add [rcx+0xe],al
00001853  108602430D06      adc [rsi+0x60d4302],al
00001859  7C0C              jl 0x1867
0000185B  07                db 0x07
0000185C  0800              or [rax],al
0000185E  0000              add [rax],al
00001860  1C00              sbb al,0x0
00001862  0000              add [rax],al
00001864  3C00              cmp al,0x0
00001866  0000              add [rax],al
00001868  A1FFFFFF1F000000  mov eax,[qword 0x1fffffff]
         -00
00001871  41                rex.b
00001872  0E                db 0x0e
00001873  108602430D06      adc [rsi+0x60d4302],al
00001879  41830359          add dword [r11],byte +0x59
0000187D  0C07              or al,0x7
0000187F  081C00            or [rax+rax],bl
00001882  0000              add [rax],al
00001884  5C                pop rsp
00001885  0000              add [rax],al
00001887  00C8              add al,cl
00001889  EE                out dx,al
0000188A  FF                db 0xff
0000188B  FF2E              jmp dword far [rsi]
0000188D  0000              add [rax],al
0000188F  0000              add [rax],al
00001891  41                rex.b
00001892  0E                db 0x0e
00001893  108602430D06      adc [rsi+0x60d4302],al
00001899  690C0708000000    imul ecx,[rdi+rax],dword 0x8
000018A0  000F              add [rdi],cl
000018A2  1F                db 0x1f
000018A3  800000            add byte [rax],0x0
000018A6  0000              add [rax],al
000018A8  0000              add [rax],al
000018AA  0000              add [rax],al
000018AC  0000              add [rax],al
000018AE  0000              add [rax],al
000018B0  55                push rbp
000018B1  4889E5            mov rbp,rsp
000018B4  48897DE8          mov [rbp-0x18],rdi
000018B8  488975E0          mov [rbp-0x20],rsi
000018BC  488B45E8          mov rax,[rbp-0x18]
000018C0  BA00000000        mov edx,0x0
000018C5  48F775E0          div qword [rbp-0x20]
000018C9  488955F8          mov [rbp-0x8],rdx
000018CD  48837DF800        cmp qword [rbp-0x8],byte +0x0
000018D2  7414              jz 0x18e8
000018D4  488B45E8          mov rax,[rbp-0x18]
000018D8  482B45F8          sub rax,[rbp-0x8]
000018DC  4889C2            mov rdx,rax
000018DF  488B45E0          mov rax,[rbp-0x20]
000018E3  4801D0            add rax,rdx
000018E6  EB04              jmp short 0x18ec
000018E8  488B45E8          mov rax,[rbp-0x18]
000018EC  5D                pop rbp
000018ED  C3                ret
000018EE  55                push rbp
000018EF  4889E5            mov rbp,rsp
000018F2  48897DF8          mov [rbp-0x8],rdi
000018F6  488B45F8          mov rax,[rbp-0x8]
000018FA  488B4010          mov rax,[rax+0x10]
000018FE  488D5020          lea rdx,[rax+0x20]
00001902  488B45F8          mov rax,[rbp-0x8]
00001906  4801D0            add rax,rdx
00001909  5D                pop rbp
0000190A  C3                ret
0000190B  55                push rbp
0000190C  4889E5            mov rbp,rsp
0000190F  4883EC30          sub rsp,byte +0x30
00001913  48897DE8          mov [rbp-0x18],rdi
00001917  488975E0          mov [rbp-0x20],rsi
0000191B  488955D8          mov [rbp-0x28],rdx
0000191F  48894DD0          mov [rbp-0x30],rcx
00001923  488B45E8          mov rax,[rbp-0x18]
00001927  488945F8          mov [rbp-0x8],rax
0000192B  488B45F8          mov rax,[rbp-0x8]
0000192F  488B55D8          mov rdx,[rbp-0x28]
00001933  488910            mov [rax],rdx
00001936  488B45F8          mov rax,[rbp-0x8]
0000193A  488B55D0          mov rdx,[rbp-0x30]
0000193E  48895008          mov [rax+0x8],rdx
00001942  488B45F8          mov rax,[rbp-0x8]
00001946  488B55E0          mov rdx,[rbp-0x20]
0000194A  48895010          mov [rax+0x10],rdx
0000194E  488B45F8          mov rax,[rbp-0x8]
00001952  C7401C042B4A51    mov dword [rax+0x1c],0x514a2b04
00001959  488B45F8          mov rax,[rbp-0x8]
0000195D  4889C7            mov rdi,rax
00001960  E889FFFFFF        call 0x18ee
00001965  C700CB5F4BA9      mov dword [rax],0xa94b5fcb
0000196B  488B45F8          mov rax,[rbp-0x8]
0000196F  C9                leave
00001970  C3                ret
00001971  90                nop
00001972  55                push rbp
00001973  4889E5            mov rbp,rsp
00001976  48897DF8          mov [rbp-0x8],rdi
0000197A  488B45F8          mov rax,[rbp-0x8]
0000197E  488B4010          mov rax,[rax+0x10]
00001982  488D5024          lea rdx,[rax+0x24]
00001986  488B45F8          mov rax,[rbp-0x8]
0000198A  4801D0            add rax,rdx
0000198D  5D                pop rbp
0000198E  C3                ret
0000198F  90                nop
00001990  55                push rbp
00001991  4889E5            mov rbp,rsp
00001994  48897DF8          mov [rbp-0x8],rdi
00001998  488B45F8          mov rax,[rbp-0x8]
0000199C  4883C020          add rax,byte +0x20
000019A0  5D                pop rbp
000019A1  C3                ret
000019A2  55                push rbp
000019A3  4889E5            mov rbp,rsp
000019A6  4883EC10          sub rsp,byte +0x10
000019AA  48897DF8          mov [rbp-0x8],rdi
000019AE  488B45F8          mov rax,[rbp-0x8]
000019B2  8B401C            mov eax,[rax+0x1c]
000019B5  3D042B4A51        cmp eax,0x514a2b04
000019BA  7515              jnz 0x19d1
000019BC  488B45F8          mov rax,[rbp-0x8]
000019C0  4889C7            mov rdi,rax
000019C3  E826FFFFFF        call 0x18ee
000019C8  8B00              mov eax,[rax]
000019CA  3DCB5F4BA9        cmp eax,0xa94b5fcb
000019CF  7407              jz 0x19d8
000019D1  B801000000        mov eax,0x1
000019D6  EB05              jmp short 0x19dd
000019D8  B800000000        mov eax,0x0
000019DD  C9                leave
000019DE  C3                ret
000019DF  90                nop
000019E0  55                push rbp
000019E1  4889E5            mov rbp,rsp
000019E4  4883EC10          sub rsp,byte +0x10
000019E8  48897DF8          mov [rbp-0x8],rdi
000019EC  488B45F8          mov rax,[rbp-0x8]
000019F0  4889C7            mov rdi,rax
000019F3  E8AAFFFFFF        call 0x19a2
000019F8  84C0              test al,al
000019FA  7413              jz 0x1a0f
000019FC  488B45F8          mov rax,[rbp-0x8]
00001A00  4889C7            mov rdi,rax
00001A03  E876EDFFFF        call 0x77e
00001A08  B800000000        mov eax,0x0
00001A0D  EB60              jmp short 0x1a6f
00001A0F  488B45F8          mov rax,[rbp-0x8]
00001A13  C7401C00000000    mov dword [rax+0x1c],0x0
00001A1A  488B45F8          mov rax,[rbp-0x8]
00001A1E  4889C7            mov rdi,rax
00001A21  E8C8FEFFFF        call 0x18ee
00001A26  C70000000000      mov dword [rax],0x0
00001A2C  488B45F8          mov rax,[rbp-0x8]
00001A30  488B00            mov rax,[rax]
00001A33  4885C0            test rax,rax
00001A36  7413              jz 0x1a4b
00001A38  488B45F8          mov rax,[rbp-0x8]
00001A3C  488B00            mov rax,[rax]
00001A3F  488B55F8          mov rdx,[rbp-0x8]
00001A43  488B5208          mov rdx,[rdx+0x8]
00001A47  48895008          mov [rax+0x8],rdx
00001A4B  488B45F8          mov rax,[rbp-0x8]
00001A4F  488B4008          mov rax,[rax+0x8]
00001A53  4885C0            test rax,rax
00001A56  7412              jz 0x1a6a
00001A58  488B45F8          mov rax,[rbp-0x8]
00001A5C  488B4008          mov rax,[rax+0x8]
00001A60  488B55F8          mov rdx,[rbp-0x8]
00001A64  488B12            mov rdx,[rdx]
00001A67  488910            mov [rax],rdx
00001A6A  B801000000        mov eax,0x1
00001A6F  C9                leave
00001A70  C3                ret
00001A71  90                nop
00001A72  55                push rbp
00001A73  4889E5            mov rbp,rsp
00001A76  4883EC40          sub rsp,byte +0x40
00001A7A  48897DE8          mov [rbp-0x18],rdi
00001A7E  488975E0          mov [rbp-0x20],rsi
00001A82  488955D8          mov [rbp-0x28],rdx
00001A86  48894DD0          mov [rbp-0x30],rcx
00001A8A  4C8945C8          mov [rbp-0x38],r8
00001A8E  488B45E0          mov rax,[rbp-0x20]
00001A92  488B00            mov rax,[rax]
00001A95  488D5020          lea rdx,[rax+0x20]
00001A99  488B45C8          mov rax,[rbp-0x38]
00001A9D  4889C6            mov rsi,rax
00001AA0  4889D7            mov rdi,rdx
00001AA3  E808FEFFFF        call 0x18b0
00001AA8  4883E820          sub rax,byte +0x20
00001AAC  488945F8          mov [rbp-0x8],rax
00001AB0  488B55D0          mov rdx,[rbp-0x30]
00001AB4  488B45F8          mov rax,[rbp-0x8]
00001AB8  4801D0            add rax,rdx
00001ABB  488D5024          lea rdx,[rax+0x24]
00001ABF  488B45D8          mov rax,[rbp-0x28]
00001AC3  4839C2            cmp rdx,rax
00001AC6  7712              ja 0x1ada
00001AC8  488B55F8          mov rdx,[rbp-0x8]
00001ACC  488B45E0          mov rax,[rbp-0x20]
00001AD0  488910            mov [rax],rdx
00001AD3  B801000000        mov eax,0x1
00001AD8  EB05              jmp short 0x1adf
00001ADA  B800000000        mov eax,0x0
00001ADF  C9                leave
00001AE0  C3                ret
00001AE1  90                nop
00001AE2  55                push rbp
00001AE3  4889E5            mov rbp,rsp
00001AE6  48897DF8          mov [rbp-0x8],rdi
00001AEA  488B45F8          mov rax,[rbp-0x8]
00001AEE  4883C018          add rax,byte +0x18
00001AF2  5D                pop rbp
00001AF3  C3                ret
00001AF4  55                push rbp
00001AF5  4889E5            mov rbp,rsp
00001AF8  4883EC20          sub rsp,byte +0x20
00001AFC  48897DE8          mov [rbp-0x18],rdi
00001B00  488975E0          mov [rbp-0x20],rsi
00001B04  488B45E0          mov rax,[rbp-0x20]
00001B08  4883E820          sub rax,byte +0x20
00001B0C  488945F8          mov [rbp-0x8],rax
00001B10  488B45F8          mov rax,[rbp-0x8]
00001B14  4889C7            mov rdi,rax
00001B17  E8C4FEFFFF        call 0x19e0
00001B1C  488B45E8          mov rax,[rbp-0x18]
00001B20  488B4008          mov rax,[rax+0x8]
00001B24  483945F8          cmp [rbp-0x8],rax
00001B28  7510              jnz 0x1b3a
00001B2A  488B45F8          mov rax,[rbp-0x8]
00001B2E  488B5008          mov rdx,[rax+0x8]
00001B32  488B45E8          mov rax,[rbp-0x18]
00001B36  48895008          mov [rax+0x8],rdx
00001B3A  488B45E8          mov rax,[rbp-0x18]
00001B3E  488B4010          mov rax,[rax+0x10]
00001B42  483945F8          cmp [rbp-0x8],rax
00001B46  750F              jnz 0x1b57
00001B48  488B45F8          mov rax,[rbp-0x8]
00001B4C  488B10            mov rdx,[rax]
00001B4F  488B45E8          mov rax,[rbp-0x18]
00001B53  48895010          mov [rax+0x10],rdx
00001B57  90                nop
00001B58  C9                leave
00001B59  C3                ret
00001B5A  55                push rbp
00001B5B  4889E5            mov rbp,rsp
00001B5E  4883EC10          sub rsp,byte +0x10
00001B62  48897DF8          mov [rbp-0x8],rdi
00001B66  488975F0          mov [rbp-0x10],rsi
00001B6A  488B0537FDFFFF    mov rax,[rel 0x18a8]
00001B71  4885C0            test rax,rax
00001B74  741C              jz 0x1b92
00001B76  488B052BFDFFFF    mov rax,[rel 0x18a8]
00001B7D  488B55F0          mov rdx,[rbp-0x10]
00001B81  488B4DF8          mov rcx,[rbp-0x8]
00001B85  4889CE            mov rsi,rcx
00001B88  4889C7            mov rdi,rax
00001B8B  E8E4EDFFFF        call 0x974
00001B90  EB05              jmp short 0x1b97
00001B92  B800000000        mov eax,0x0
00001B97  C9                leave
00001B98  C3                ret
00001B99  55                push rbp
00001B9A  4889E5            mov rbp,rsp
00001B9D  4883EC10          sub rsp,byte +0x10
00001BA1  48897DF8          mov [rbp-0x8],rdi
00001BA5  488B05FCFCFFFF    mov rax,[rel 0x18a8]
00001BAC  488B55F8          mov rdx,[rbp-0x8]
00001BB0  4889D6            mov rsi,rdx
00001BB3  4889C7            mov rdi,rax
00001BB6  E839FFFFFF        call 0x1af4
00001BBB  90                nop
00001BBC  C9                leave
00001BBD  C3                ret
00001BBE  55                push rbp
00001BBF  4889E5            mov rbp,rsp
00001BC2  4883EC10          sub rsp,byte +0x10
00001BC6  48897DF8          mov [rbp-0x8],rdi
00001BCA  488975F0          mov [rbp-0x10],rsi
00001BCE  488B05D3FCFFFF    mov rax,[rel 0x18a8]
00001BD5  488B55F8          mov rdx,[rbp-0x8]
00001BD9  4889D6            mov rsi,rdx
00001BDC  4889C7            mov rdi,rax
00001BDF  E810FFFFFF        call 0x1af4
00001BE4  90                nop
00001BE5  C9                leave
00001BE6  C3                ret
00001BE7  55                push rbp
00001BE8  4889E5            mov rbp,rsp
00001BEB  48897DF8          mov [rbp-0x8],rdi
00001BEF  488975F0          mov [rbp-0x10],rsi
00001BF3  48837DF000        cmp qword [rbp-0x10],byte +0x0
00001BF8  7806              js 0x1c00
00001BFA  488B45F0          mov rax,[rbp-0x10]
00001BFE  EB38              jmp short 0x1c38
00001C00  488B45F8          mov rax,[rbp-0x8]
00001C04  488B00            mov rax,[rax]
00001C07  488D4801          lea rcx,[rax+0x1]
00001C0B  488B55F8          mov rdx,[rbp-0x8]
00001C0F  48890A            mov [rdx],rcx
00001C12  C6002D            mov byte [rax],0x2d
00001C15  48B8000000000000  mov rax,0x8000000000000000
         -0080
00001C1F  483945F0          cmp [rbp-0x10],rax
00001C23  750C              jnz 0x1c31
00001C25  48B8000000000000  mov rax,0x8000000000000000
         -0080
00001C2F  EB07              jmp short 0x1c38
00001C31  488B45F0          mov rax,[rbp-0x10]
00001C35  48F7D8            neg rax
00001C38  5D                pop rbp
00001C39  C3                ret
00001C3A  55                push rbp
00001C3B  4889E5            mov rbp,rsp
00001C3E  4883EC20          sub rsp,byte +0x20
00001C42  48897DE8          mov [rbp-0x18],rdi
00001C46  488975E0          mov [rbp-0x20],rsi
00001C4A  C645FF00          mov byte [rbp-0x1],0x0
00001C4E  48837DE000        cmp qword [rbp-0x20],byte +0x0
00001C53  7506              jnz 0x1c5b
00001C55  C645FF01          mov byte [rbp-0x1],0x1
00001C59  EB20              jmp short 0x1c7b
00001C5B  488B45E0          mov rax,[rbp-0x20]
00001C5F  488945F0          mov [rbp-0x10],rax
00001C63  48837DF000        cmp qword [rbp-0x10],byte +0x0
00001C68  7411              jz 0x1c7b
00001C6A  0FB645FF          movzx eax,byte [rbp-0x1]
00001C6E  83C001            add eax,byte +0x1
00001C71  8845FF            mov [rbp-0x1],al
00001C74  48C16DF002        shr qword [rbp-0x10],byte 0x2
00001C79  EBE8              jmp short 0x1c63
00001C7B  0FB645FF          movzx eax,byte [rbp-0x1]
00001C7F  480145E8          add [rbp-0x18],rax
00001C83  488B45E8          mov rax,[rbp-0x18]
00001C87  C60000            mov byte [rax],0x0
00001C8A  807DFF00          cmp byte [rbp-0x1],0x0
00001C8E  742C              jz 0x1cbc
00001C90  488B45E0          mov rax,[rbp-0x20]
00001C94  83E001            and eax,byte +0x1
00001C97  48836DE801        sub qword [rbp-0x18],byte +0x1
00001C9C  89C7              mov edi,eax
00001C9E  E8BFF7FFFF        call 0x1462
00001CA3  89C2              mov edx,eax
00001CA5  488B45E8          mov rax,[rbp-0x18]
00001CA9  8810              mov [rax],dl
00001CAB  0FB645FF          movzx eax,byte [rbp-0x1]
00001CAF  83E801            sub eax,byte +0x1
00001CB2  8845FF            mov [rbp-0x1],al
00001CB5  48C16DE002        shr qword [rbp-0x20],byte 0x2
00001CBA  EBCE              jmp short 0x1c8a
00001CBC  90                nop
00001CBD  C9                leave
00001CBE  C3                ret
00001CBF  55                push rbp
00001CC0  4889E5            mov rbp,rsp
00001CC3  4883EC10          sub rsp,byte +0x10
00001CC7  48897DF8          mov [rbp-0x8],rdi
00001CCB  488975F0          mov [rbp-0x10],rsi
00001CCF  488B55F0          mov rdx,[rbp-0x10]
00001CD3  488D45F8          lea rax,[rbp-0x8]
00001CD7  4889D6            mov rsi,rdx
00001CDA  4889C7            mov rdi,rax
00001CDD  E805FFFFFF        call 0x1be7
00001CE2  4889C2            mov rdx,rax
00001CE5  488B45F8          mov rax,[rbp-0x8]
00001CE9  4889D6            mov rsi,rdx
00001CEC  4889C7            mov rdi,rax
00001CEF  E846FFFFFF        call 0x1c3a
00001CF4  90                nop
00001CF5  C9                leave
00001CF6  C3                ret
00001CF7  55                push rbp
00001CF8  4889E5            mov rbp,rsp
00001CFB  4883EC20          sub rsp,byte +0x20
00001CFF  48897DE8          mov [rbp-0x18],rdi
00001D03  488975E0          mov [rbp-0x20],rsi
00001D07  C645FF00          mov byte [rbp-0x1],0x0
00001D0B  48837DE000        cmp qword [rbp-0x20],byte +0x0
00001D10  7506              jnz 0x1d18
00001D12  C645FF01          mov byte [rbp-0x1],0x1
00001D16  EB20              jmp short 0x1d38
00001D18  488B45E0          mov rax,[rbp-0x20]
00001D1C  488945F0          mov [rbp-0x10],rax
00001D20  48837DF000        cmp qword [rbp-0x10],byte +0x0
00001D25  7411              jz 0x1d38
00001D27  0FB645FF          movzx eax,byte [rbp-0x1]
00001D2B  83C001            add eax,byte +0x1
00001D2E  8845FF            mov [rbp-0x1],al
00001D31  48C16DF003        shr qword [rbp-0x10],byte 0x3
00001D36  EBE8              jmp short 0x1d20
00001D38  0FB645FF          movzx eax,byte [rbp-0x1]
00001D3C  480145E8          add [rbp-0x18],rax
00001D40  488B45E8          mov rax,[rbp-0x18]
00001D44  C60000            mov byte [rax],0x0
00001D47  807DFF00          cmp byte [rbp-0x1],0x0
00001D4B  742C              jz 0x1d79
00001D4D  488B45E0          mov rax,[rbp-0x20]
00001D51  83E007            and eax,byte +0x7
00001D54  48836DE801        sub qword [rbp-0x18],byte +0x1
00001D59  89C7              mov edi,eax
00001D5B  E802F7FFFF        call 0x1462
00001D60  89C2              mov edx,eax
00001D62  488B45E8          mov rax,[rbp-0x18]
00001D66  8810              mov [rax],dl
00001D68  0FB645FF          movzx eax,byte [rbp-0x1]
00001D6C  83E801            sub eax,byte +0x1
00001D6F  8845FF            mov [rbp-0x1],al
00001D72  48C16DE003        shr qword [rbp-0x20],byte 0x3
00001D77  EBCE              jmp short 0x1d47
00001D79  90                nop
00001D7A  C9                leave
00001D7B  C3                ret
00001D7C  55                push rbp
00001D7D  4889E5            mov rbp,rsp
00001D80  4883EC10          sub rsp,byte +0x10
00001D84  48897DF8          mov [rbp-0x8],rdi
00001D88  488975F0          mov [rbp-0x10],rsi
00001D8C  488B55F0          mov rdx,[rbp-0x10]
00001D90  488D45F8          lea rax,[rbp-0x8]
00001D94  4889D6            mov rsi,rdx
00001D97  4889C7            mov rdi,rax
00001D9A  E848FEFFFF        call 0x1be7
00001D9F  4889C2            mov rdx,rax
00001DA2  488B45F8          mov rax,[rbp-0x8]
00001DA6  4889D6            mov rsi,rdx
00001DA9  4889C7            mov rdi,rax
00001DAC  E846FFFFFF        call 0x1cf7
00001DB1  90                nop
00001DB2  C9                leave
00001DB3  C3                ret
00001DB4  55                push rbp
00001DB5  4889E5            mov rbp,rsp
00001DB8  4883EC20          sub rsp,byte +0x20
00001DBC  48897DE8          mov [rbp-0x18],rdi
00001DC0  488975E0          mov [rbp-0x20],rsi
00001DC4  C645FF00          mov byte [rbp-0x1],0x0
00001DC8  48837DE000        cmp qword [rbp-0x20],byte +0x0
00001DCD  7506              jnz 0x1dd5
00001DCF  C645FF01          mov byte [rbp-0x1],0x1
00001DD3  EB20              jmp short 0x1df5
00001DD5  488B45E0          mov rax,[rbp-0x20]
00001DD9  488945F0          mov [rbp-0x10],rax
00001DDD  48837DF000        cmp qword [rbp-0x10],byte +0x0
00001DE2  7411              jz 0x1df5
00001DE4  0FB645FF          movzx eax,byte [rbp-0x1]
00001DE8  83C001            add eax,byte +0x1
00001DEB  8845FF            mov [rbp-0x1],al
00001DEE  48C16DF004        shr qword [rbp-0x10],byte 0x4
00001DF3  EBE8              jmp short 0x1ddd
00001DF5  0FB645FF          movzx eax,byte [rbp-0x1]
00001DF9  480145E8          add [rbp-0x18],rax
00001DFD  488B45E8          mov rax,[rbp-0x18]
00001E01  C60000            mov byte [rax],0x0
00001E04  807DFF00          cmp byte [rbp-0x1],0x0
00001E08  742C              jz 0x1e36
00001E0A  488B45E0          mov rax,[rbp-0x20]
00001E0E  83E00F            and eax,byte +0xf
00001E11  48836DE801        sub qword [rbp-0x18],byte +0x1
00001E16  89C7              mov edi,eax
00001E18  E845F6FFFF        call 0x1462
00001E1D  89C2              mov edx,eax
00001E1F  488B45E8          mov rax,[rbp-0x18]
00001E23  8810              mov [rax],dl
00001E25  0FB645FF          movzx eax,byte [rbp-0x1]
00001E29  83E801            sub eax,byte +0x1
00001E2C  8845FF            mov [rbp-0x1],al
00001E2F  48C16DE004        shr qword [rbp-0x20],byte 0x4
00001E34  EBCE              jmp short 0x1e04
00001E36  90                nop
00001E37  C9                leave
00001E38  C3                ret
00001E39  55                push rbp
00001E3A  4889E5            mov rbp,rsp
00001E3D  4883EC10          sub rsp,byte +0x10
00001E41  48897DF8          mov [rbp-0x8],rdi
00001E45  488975F0          mov [rbp-0x10],rsi
00001E49  488B55F0          mov rdx,[rbp-0x10]
00001E4D  488D45F8          lea rax,[rbp-0x8]
00001E51  4889D6            mov rsi,rdx
00001E54  4889C7            mov rdi,rax
00001E57  E88BFDFFFF        call 0x1be7
00001E5C  4889C2            mov rdx,rax
00001E5F  488B45F8          mov rax,[rbp-0x8]
00001E63  4889D6            mov rsi,rdx
00001E66  4889C7            mov rdi,rax
00001E69  E846FFFFFF        call 0x1db4
00001E6E  90                nop
00001E6F  C9                leave
00001E70  C3                ret
00001E71  55                push rbp
00001E72  4889E5            mov rbp,rsp
00001E75  4883EC30          sub rsp,byte +0x30
00001E79  48897DE8          mov [rbp-0x18],rdi
00001E7D  488975E0          mov [rbp-0x20],rsi
00001E81  8955DC            mov [rbp-0x24],edx
00001E84  C645FF00          mov byte [rbp-0x1],0x0
00001E88  48837DE000        cmp qword [rbp-0x20],byte +0x0
00001E8D  7506              jnz 0x1e95
00001E8F  C645FF01          mov byte [rbp-0x1],0x1
00001E93  EB31              jmp short 0x1ec6
00001E95  488B45E0          mov rax,[rbp-0x20]
00001E99  488945F0          mov [rbp-0x10],rax
00001E9D  48837DF000        cmp qword [rbp-0x10],byte +0x0
00001EA2  7422              jz 0x1ec6
00001EA4  0FB645FF          movzx eax,byte [rbp-0x1]
00001EA8  83C001            add eax,byte +0x1
00001EAB  8845FF            mov [rbp-0x1],al
00001EAE  8B45DC            mov eax,[rbp-0x24]
00001EB1  4863F0            movsxd rsi,eax
00001EB4  488B45F0          mov rax,[rbp-0x10]
00001EB8  BA00000000        mov edx,0x0
00001EBD  48F7F6            div rsi
00001EC0  488945F0          mov [rbp-0x10],rax
00001EC4  EBD7              jmp short 0x1e9d
00001EC6  0FB645FF          movzx eax,byte [rbp-0x1]
00001ECA  480145E8          add [rbp-0x18],rax
00001ECE  488B45E8          mov rax,[rbp-0x18]
00001ED2  C60000            mov byte [rax],0x0
00001ED5  807DFF00          cmp byte [rbp-0x1],0x0
00001ED9  744B              jz 0x1f26
00001EDB  8B45DC            mov eax,[rbp-0x24]
00001EDE  4863C8            movsxd rcx,eax
00001EE1  488B45E0          mov rax,[rbp-0x20]
00001EE5  BA00000000        mov edx,0x0
00001EEA  48F7F1            div rcx
00001EED  4889D0            mov rax,rdx
00001EF0  48836DE801        sub qword [rbp-0x18],byte +0x1
00001EF5  89C7              mov edi,eax
00001EF7  E866F5FFFF        call 0x1462
00001EFC  89C2              mov edx,eax
00001EFE  488B45E8          mov rax,[rbp-0x18]
00001F02  8810              mov [rax],dl
00001F04  0FB645FF          movzx eax,byte [rbp-0x1]
00001F08  83E801            sub eax,byte +0x1
00001F0B  8845FF            mov [rbp-0x1],al
00001F0E  8B45DC            mov eax,[rbp-0x24]
00001F11  4863C8            movsxd rcx,eax
00001F14  488B45E0          mov rax,[rbp-0x20]
00001F18  BA00000000        mov edx,0x0
00001F1D  48F7F1            div rcx
00001F20  488945E0          mov [rbp-0x20],rax
00001F24  EBAF              jmp short 0x1ed5
00001F26  90                nop
00001F27  C9                leave
00001F28  C3                ret
00001F29  55                push rbp
00001F2A  4889E5            mov rbp,rsp
00001F2D  4883EC10          sub rsp,byte +0x10
00001F31  48897DF8          mov [rbp-0x8],rdi
00001F35  488975F0          mov [rbp-0x10],rsi
00001F39  488B4DF0          mov rcx,[rbp-0x10]
00001F3D  488B45F8          mov rax,[rbp-0x8]
00001F41  BA0A000000        mov edx,0xa
00001F46  4889CE            mov rsi,rcx
00001F49  4889C7            mov rdi,rax
00001F4C  E820FFFFFF        call 0x1e71
00001F51  90                nop
00001F52  C9                leave
00001F53  C3                ret
00001F54  55                push rbp
00001F55  4889E5            mov rbp,rsp
00001F58  4883EC10          sub rsp,byte +0x10
00001F5C  48897DF8          mov [rbp-0x8],rdi
00001F60  488975F0          mov [rbp-0x10],rsi
00001F64  488B55F0          mov rdx,[rbp-0x10]
00001F68  488D45F8          lea rax,[rbp-0x8]
00001F6C  4889D6            mov rsi,rdx
00001F6F  4889C7            mov rdi,rax
00001F72  E870FCFFFF        call 0x1be7
00001F77  4889C1            mov rcx,rax
00001F7A  488B45F8          mov rax,[rbp-0x8]
00001F7E  BA0A000000        mov edx,0xa
00001F83  4889CE            mov rsi,rcx
00001F86  4889C7            mov rdi,rax
00001F89  E8E3FEFFFF        call 0x1e71
00001F8E  90                nop
00001F8F  C9                leave
00001F90  C3                ret
00001F91  90                nop
00001F92  55                push rbp
00001F93  4889E5            mov rbp,rsp
00001F96  48897DF8          mov [rbp-0x8],rdi
00001F9A  488B45F8          mov rax,[rbp-0x8]
00001F9E  0FB600            movzx eax,byte [rax]
00001FA1  83E003            and eax,byte +0x3
00001FA4  5D                pop rbp
00001FA5  C3                ret
00001FA6  55                push rbp
00001FA7  4889E5            mov rbp,rsp
00001FAA  4883EC10          sub rsp,byte +0x10
00001FAE  48897DF8          mov [rbp-0x8],rdi
00001FB2  488975F0          mov [rbp-0x10],rsi
00001FB6  488B45F0          mov rax,[rbp-0x10]
00001FBA  4889C7            mov rdi,rax
00001FBD  E8C1F3FFFF        call 0x1383
00001FC2  488B45F8          mov rax,[rbp-0x8]
00001FC6  C9                leave
00001FC7  C3                ret
00001FC8  55                push rbp
00001FC9  4889E5            mov rbp,rsp
00001FCC  4883EC10          sub rsp,byte +0x10
00001FD0  48897DF8          mov [rbp-0x8],rdi
00001FD4  89F0              mov eax,esi
00001FD6  8845F4            mov [rbp-0xc],al
00001FD9  0FBE45F4          movsx eax,byte [rbp-0xc]
00001FDD  89C7              mov edi,eax
00001FDF  E86A030000        call 0x234e
00001FE4  488B45F8          mov rax,[rbp-0x8]
00001FE8  C9                leave
00001FE9  C3                ret
00001FEA  55                push rbp
00001FEB  4889E5            mov rbp,rsp
00001FEE  4883EC10          sub rsp,byte +0x10
00001FF2  48897DF8          mov [rbp-0x8],rdi
00001FF6  8975F4            mov [rbp-0xc],esi
00001FF9  8B45F4            mov eax,[rbp-0xc]
00001FFC  4863D0            movsxd rdx,eax
00001FFF  488B45F8          mov rax,[rbp-0x8]
00002003  4889D6            mov rsi,rdx
00002006  4889C7            mov rdi,rax
00002009  E802000000        call 0x2010
0000200E  C9                leave
0000200F  C3                ret
00002010  55                push rbp
00002011  4889E5            mov rbp,rsp
00002014  4881ECA0000000    sub rsp,0xa0
0000201B  4889BD68FFFFFF    mov [rbp-0x98],rdi
00002022  4889B560FFFFFF    mov [rbp-0xa0],rsi
00002029  488B8568FFFFFF    mov rax,[rbp-0x98]
00002030  4889C7            mov rdi,rax
00002033  E85AFFFFFF        call 0x1f92
00002038  3C01              cmp al,0x1
0000203A  743A              jz 0x2076
0000203C  3C01              cmp al,0x1
0000203E  7267              jc 0x20a7
00002040  3C03              cmp al,0x3
00002042  0F8590000000      jnz near 0x20d8
00002048  488B9560FFFFFF    mov rdx,[rbp-0xa0]
0000204F  488D45C0          lea rax,[rbp-0x40]
00002053  4889D6            mov rsi,rdx
00002056  4889C7            mov rdi,rax
00002059  E8DBFDFFFF        call 0x1e39
0000205E  488D45C0          lea rax,[rbp-0x40]
00002062  4889C7            mov rdi,rax
00002065  E819F3FFFF        call 0x1383
0000206A  488B8568FFFFFF    mov rax,[rbp-0x98]
00002071  E98B000000        jmp 0x2101
00002076  488B9560FFFFFF    mov rdx,[rbp-0xa0]
0000207D  488D8570FFFFFF    lea rax,[rbp-0x90]
00002084  4889D6            mov rsi,rdx
00002087  4889C7            mov rdi,rax
0000208A  E8EDFCFFFF        call 0x1d7c
0000208F  488D8570FFFFFF    lea rax,[rbp-0x90]
00002096  4889C7            mov rdi,rax
00002099  E8E5F2FFFF        call 0x1383
0000209E  488B8568FFFFFF    mov rax,[rbp-0x98]
000020A5  EB5A              jmp short 0x2101
000020A7  488B9560FFFFFF    mov rdx,[rbp-0xa0]
000020AE  488D8570FFFFFF    lea rax,[rbp-0x90]
000020B5  4889D6            mov rsi,rdx
000020B8  4889C7            mov rdi,rax
000020BB  E8FFFBFFFF        call 0x1cbf
000020C0  488D8570FFFFFF    lea rax,[rbp-0x90]
000020C7  4889C7            mov rdi,rax
000020CA  E8B4F2FFFF        call 0x1383
000020CF  488B8568FFFFFF    mov rax,[rbp-0x98]
000020D6  EB29              jmp short 0x2101
000020D8  488B9560FFFFFF    mov rdx,[rbp-0xa0]
000020DF  488D45E0          lea rax,[rbp-0x20]
000020E3  4889D6            mov rsi,rdx
000020E6  4889C7            mov rdi,rax
000020E9  E866FEFFFF        call 0x1f54
000020EE  488D45E0          lea rax,[rbp-0x20]
000020F2  4889C7            mov rdi,rax
000020F5  E889F2FFFF        call 0x1383
000020FA  488B8568FFFFFF    mov rax,[rbp-0x98]
00002101  C9                leave
00002102  C3                ret
00002103  90                nop
00002104  55                push rbp
00002105  4889E5            mov rbp,rsp
00002108  4881ECA0000000    sub rsp,0xa0
0000210F  4889BD68FFFFFF    mov [rbp-0x98],rdi
00002116  4889B560FFFFFF    mov [rbp-0xa0],rsi
0000211D  488B8568FFFFFF    mov rax,[rbp-0x98]
00002124  4889C7            mov rdi,rax
00002127  E866FEFFFF        call 0x1f92
0000212C  3C01              cmp al,0x1
0000212E  743A              jz 0x216a
00002130  3C01              cmp al,0x1
00002132  7267              jc 0x219b
00002134  3C03              cmp al,0x3
00002136  0F8590000000      jnz near 0x21cc
0000213C  488B9560FFFFFF    mov rdx,[rbp-0xa0]
00002143  488D45C0          lea rax,[rbp-0x40]
00002147  4889D6            mov rsi,rdx
0000214A  4889C7            mov rdi,rax
0000214D  E862FCFFFF        call 0x1db4
00002152  488D45C0          lea rax,[rbp-0x40]
00002156  4889C7            mov rdi,rax
00002159  E825F2FFFF        call 0x1383
0000215E  488B8568FFFFFF    mov rax,[rbp-0x98]
00002165  E98B000000        jmp 0x21f5
0000216A  488B9560FFFFFF    mov rdx,[rbp-0xa0]
00002171  488D8570FFFFFF    lea rax,[rbp-0x90]
00002178  4889D6            mov rsi,rdx
0000217B  4889C7            mov rdi,rax
0000217E  E874FBFFFF        call 0x1cf7
00002183  488D8570FFFFFF    lea rax,[rbp-0x90]
0000218A  4889C7            mov rdi,rax
0000218D  E8F1F1FFFF        call 0x1383
00002192  488B8568FFFFFF    mov rax,[rbp-0x98]
00002199  EB5A              jmp short 0x21f5
0000219B  488B9560FFFFFF    mov rdx,[rbp-0xa0]
000021A2  488D8570FFFFFF    lea rax,[rbp-0x90]
000021A9  4889D6            mov rsi,rdx
000021AC  4889C7            mov rdi,rax
000021AF  E886FAFFFF        call 0x1c3a
000021B4  488D8570FFFFFF    lea rax,[rbp-0x90]
000021BB  4889C7            mov rdi,rax
000021BE  E8C0F1FFFF        call 0x1383
000021C3  488B8568FFFFFF    mov rax,[rbp-0x98]
000021CA  EB29              jmp short 0x21f5
000021CC  488B9560FFFFFF    mov rdx,[rbp-0xa0]
000021D3  488D45E0          lea rax,[rbp-0x20]
000021D7  4889D6            mov rsi,rdx
000021DA  4889C7            mov rdi,rax
000021DD  E847FDFFFF        call 0x1f29
000021E2  488D45E0          lea rax,[rbp-0x20]
000021E6  4889C7            mov rdi,rax
000021E9  E895F1FFFF        call 0x1383
000021EE  488B8568FFFFFF    mov rax,[rbp-0x98]
000021F5  C9                leave
000021F6  C3                ret
000021F7  90                nop
000021F8  55                push rbp
000021F9  4889E5            mov rbp,rsp
000021FC  48897DF8          mov [rbp-0x8],rdi
00002200  89F0              mov eax,esi
00002202  8845F4            mov [rbp-0xc],al
00002205  488B45F8          mov rax,[rbp-0x8]
00002209  0FB600            movzx eax,byte [rax]
0000220C  83E0FC            and eax,byte -0x4
0000220F  89C2              mov edx,eax
00002211  488B45F8          mov rax,[rbp-0x8]
00002215  8810              mov [rax],dl
00002217  488B45F8          mov rax,[rbp-0x8]
0000221B  0FB610            movzx edx,byte [rax]
0000221E  0FB645F4          movzx eax,byte [rbp-0xc]
00002222  83E003            and eax,byte +0x3
00002225  09C2              or edx,eax
00002227  488B45F8          mov rax,[rbp-0x8]
0000222B  8810              mov [rax],dl
0000222D  488B45F8          mov rax,[rbp-0x8]
00002231  5D                pop rbp
00002232  C3                ret
00002233  55                push rbp
00002234  4889E5            mov rbp,rsp
00002237  4883EC10          sub rsp,byte +0x10
0000223B  48897DF8          mov [rbp-0x8],rdi
0000223F  488B45F8          mov rax,[rbp-0x8]
00002243  0FB600            movzx eax,byte [rax]
00002246  3C0F              cmp al,0xf
00002248  770F              ja 0x2259
0000224A  BE30000000        mov esi,0x30
0000224F  BF01101000        mov edi,0x101001
00002254  E86FFDFFFF        call 0x1fc8
00002259  488B45F8          mov rax,[rbp-0x8]
0000225D  0FB600            movzx eax,byte [rax]
00002260  0FB6C0            movzx eax,al
00002263  89C6              mov esi,eax
00002265  BF01101000        mov edi,0x101001
0000226A  E87BFDFFFF        call 0x1fea
0000226F  90                nop
00002270  C9                leave
00002271  C3                ret
00002272  55                push rbp
00002273  4889E5            mov rbp,rsp
00002276  4883EC20          sub rsp,byte +0x20
0000227A  48897DE8          mov [rbp-0x18],rdi
0000227E  488B45E8          mov rax,[rbp-0x18]
00002282  4889C6            mov rsi,rax
00002285  BF01101000        mov edi,0x101001
0000228A  E84BF2FFFF        call 0x14da
0000228F  BE3A000000        mov esi,0x3a
00002294  4889C7            mov rdi,rax
00002297  E82CFDFFFF        call 0x1fc8
0000229C  BE03000000        mov esi,0x3
000022A1  4889C7            mov rdi,rax
000022A4  E84FFFFFFF        call 0x21f8
000022A9  C745FC00000000    mov dword [rbp-0x4],0x0
000022B0  837DFC0F          cmp dword [rbp-0x4],byte +0xf
000022B4  7F3A              jg 0x22f0
000022B6  837DFC08          cmp dword [rbp-0x4],byte +0x8
000022BA  7507              jnz 0x22c3
000022BC  B88B121000        mov eax,0x10128b
000022C1  EB05              jmp short 0x22c8
000022C3  B88E121000        mov eax,0x10128e
000022C8  4889C6            mov rsi,rax
000022CB  BF01101000        mov edi,0x101001
000022D0  E8D1FCFFFF        call 0x1fa6
000022D5  8B45FC            mov eax,[rbp-0x4]
000022D8  4863D0            movsxd rdx,eax
000022DB  488B45E8          mov rax,[rbp-0x18]
000022DF  4801D0            add rax,rdx
000022E2  4889C7            mov rdi,rax
000022E5  E849FFFFFF        call 0x2233
000022EA  8345FC01          add dword [rbp-0x4],byte +0x1
000022EE  EBC0              jmp short 0x22b0
000022F0  BE0A000000        mov esi,0xa
000022F5  BF01101000        mov edi,0x101001
000022FA  E8C9FCFFFF        call 0x1fc8
000022FF  BE02000000        mov esi,0x2
00002304  4889C7            mov rdi,rax
00002307  E8ECFEFFFF        call 0x21f8
0000230C  90                nop
0000230D  C9                leave
0000230E  C3                ret
0000230F  55                push rbp
00002310  4889E5            mov rbp,rsp
00002313  4883EC20          sub rsp,byte +0x20
00002317  48897DE8          mov [rbp-0x18],rdi
0000231B  488975E0          mov [rbp-0x20],rsi
0000231F  48C745F800000000  mov qword [rbp-0x8],0x0
00002327  488B45F8          mov rax,[rbp-0x8]
0000232B  483B45E0          cmp rax,[rbp-0x20]
0000232F  731A              jnc 0x234b
00002331  488B55E8          mov rdx,[rbp-0x18]
00002335  488B45F8          mov rax,[rbp-0x8]
00002339  4801D0            add rax,rdx
0000233C  4889C7            mov rdi,rax
0000233F  E82EFFFFFF        call 0x2272
00002344  488345F810        add qword [rbp-0x8],byte +0x10
00002349  EBDC              jmp short 0x2327
0000234B  90                nop
0000234C  C9                leave
0000234D  C3                ret
0000234E  55                push rbp
0000234F  4889E5            mov rbp,rsp
00002352  53                push rbx
00002353  89F8              mov eax,edi
00002355  8845F4            mov [rbp-0xc],al
00002358  B801000000        mov eax,0x1
0000235D  B902000000        mov ecx,0x2
00002362  0FB655F4          movzx edx,byte [rbp-0xc]
00002366  89CB              mov ebx,ecx
00002368  89D7              mov edi,edx
0000236A  CD30              int 0x30
0000236C  90                nop
0000236D  5B                pop rbx
0000236E  5D                pop rbp
0000236F  C3                ret
00002370  1400              adc al,0x0
00002372  0000              add [rax],al
00002374  0000              add [rax],al
00002376  0000              add [rax],al
00002378  017A52            add [rdx+0x52],edi
0000237B  0001              add [rcx],al
0000237D  7810              js 0x238f
0000237F  011B              add [rbx],ebx
00002381  0C07              or al,0x7
00002383  08900100001C      or [rax+0x1c000001],dl
00002389  0000              add [rax],al
0000238B  001C00            add [rax+rax],bl
0000238E  0000              add [rax],al
00002390  20F5              and ch,dh
00002392  FF                db 0xff
00002393  FF                db 0xff
00002394  3E0000            add [ds:rax],al
00002397  0000              add [rax],al
00002399  41                rex.b
0000239A  0E                db 0x0e
0000239B  108602430D06      adc [rsi+0x60d4302],al
000023A1  790C              jns 0x23af
000023A3  07                db 0x07
000023A4  0800              or [rax],al
000023A6  0000              add [rax],al
000023A8  1C00              sbb al,0x0
000023AA  0000              add [rax],al
000023AC  3C00              cmp al,0x0
000023AE  0000              add [rax],al
000023B0  3EF5              ds cmc
000023B2  FF                db 0xff
000023B3  FF1D00000000      call dword far [rel 0x23b9]
000023B9  41                rex.b
000023BA  0E                db 0x0e
000023BB  108602430D06      adc [rsi+0x60d4302],al
000023C1  58                pop rax
000023C2  0C07              or al,0x7
000023C4  0800              or [rax],al
000023C6  0000              add [rax],al
000023C8  1C00              sbb al,0x0
000023CA  0000              add [rax],al
000023CC  5C                pop rsp
000023CD  0000              add [rax],al
000023CF  003B              add [rbx],bh
000023D1  F5                cmc
000023D2  FF                db 0xff
000023D3  FF6600            jmp [rsi+0x0]
000023D6  0000              add [rax],al
000023D8  00410E            add [rcx+0xe],al
000023DB  108602430D06      adc [rsi+0x60d4302],al
000023E1  02610C            add ah,[rcx+0xc]
000023E4  07                db 0x07
000023E5  0800              or [rax],al
000023E7  001C00            add [rax+rax],bl
000023EA  0000              add [rax],al
000023EC  7C00              jl 0x23ee
000023EE  0000              add [rax],al
000023F0  82                db 0x82
000023F1  F5                cmc
000023F2  FF                db 0xff
000023F3  FF1D00000000      call dword far [rel 0x23f9]
000023F9  41                rex.b
000023FA  0E                db 0x0e
000023FB  108602430D06      adc [rsi+0x60d4302],al
00002401  58                pop rax
00002402  0C07              or al,0x7
00002404  0800              or [rax],al
00002406  0000              add [rax],al
00002408  1C00              sbb al,0x0
0000240A  0000              add [rax],al
0000240C  9C                pushf
0000240D  0000              add [rax],al
0000240F  0080F5FFFF12      add [rax+0x12fffff5],al
00002415  0000              add [rax],al
00002417  0000              add [rax],al
00002419  41                rex.b
0000241A  0E                db 0x0e
0000241B  108602430D06      adc [rsi+0x60d4302],al
00002421  4D0C07            o64 or al,0x7
00002424  0800              or [rax],al
00002426  0000              add [rax],al
00002428  1C00              sbb al,0x0
0000242A  0000              add [rax],al
0000242C  BC00000072        mov esp,0x72000000
00002431  F5                cmc
00002432  FF                db 0xff
00002433  FF                db 0xff
00002434  3D00000000        cmp eax,0x0
00002439  41                rex.b
0000243A  0E                db 0x0e
0000243B  108602430D06      adc [rsi+0x60d4302],al
00002441  780C              js 0x244f
00002443  07                db 0x07
00002444  0800              or [rax],al
00002446  0000              add [rax],al
00002448  1C00              sbb al,0x0
0000244A  0000              add [rax],al
0000244C  DC00              fadd qword [rax]
0000244E  0000              add [rax],al
00002450  90                nop
00002451  F5                cmc
00002452  FF                db 0xff
00002453  FF9100000000      call [rcx+0x0]
00002459  41                rex.b
0000245A  0E                db 0x0e
0000245B  108602430D06      adc [rsi+0x60d4302],al
00002461  028C0C07080000    add cl,[rsp+rcx+0x807]
00002468  1C00              sbb al,0x0
0000246A  0000              add [rax],al
0000246C  FC                cld
0000246D  0000              add [rax],al
0000246F  0002              add [rdx],al
00002471  F6FF              idiv bh
00002473  FF6F00            jmp dword far [rdi+0x0]
00002476  0000              add [rax],al
00002478  00410E            add [rcx+0xe],al
0000247B  108602430D06      adc [rsi+0x60d4302],al
00002481  026A0C            add ch,[rdx+0xc]
00002484  07                db 0x07
00002485  0800              or [rax],al
00002487  001C00            add [rax+rax],bl
0000248A  0000              add [rax],al
0000248C  1C01              sbb al,0x1
0000248E  0000              add [rax],al
00002490  52                push rdx
00002491  F6FF              idiv bh
00002493  FF12              call [rdx]
00002495  0000              add [rax],al
00002497  0000              add [rax],al
00002499  41                rex.b
0000249A  0E                db 0x0e
0000249B  108602430D06      adc [rsi+0x60d4302],al
000024A1  4D0C07            o64 or al,0x7
000024A4  0800              or [rax],al
000024A6  0000              add [rax],al
000024A8  1C00              sbb al,0x0
000024AA  0000              add [rax],al
000024AC  3C01              cmp al,0x1
000024AE  0000              add [rax],al
000024B0  44F6FF            idiv dil
000024B3  FF6600            jmp [rsi+0x0]
000024B6  0000              add [rax],al
000024B8  00410E            add [rcx+0xe],al
000024BB  108602430D06      adc [rsi+0x60d4302],al
000024C1  02610C            add ah,[rcx+0xc]
000024C4  07                db 0x07
000024C5  0800              or [rax],al
000024C7  001C00            add [rax+rax],bl
000024CA  0000              add [rax],al
000024CC  5C                pop rsp
000024CD  0100              add [rax],eax
000024CF  008AF6FFFF3F      add [rdx+0x3ffffff6],cl
000024D5  0000              add [rax],al
000024D7  0000              add [rax],al
000024D9  41                rex.b
000024DA  0E                db 0x0e
000024DB  108602430D06      adc [rsi+0x60d4302],al
000024E1  7A0C              jpe 0x24ef
000024E3  07                db 0x07
000024E4  0800              or [rax],al
000024E6  0000              add [rax],al
000024E8  1C00              sbb al,0x0
000024EA  0000              add [rax],al
000024EC  7C01              jl 0x24ef
000024EE  0000              add [rax],al
000024F0  A9F6FFFF25        test eax,0x25fffff6
000024F5  0000              add [rax],al
000024F7  0000              add [rax],al
000024F9  41                rex.b
000024FA  0E                db 0x0e
000024FB  108602430D06      adc [rsi+0x60d4302],al
00002501  60                db 0x60
00002502  0C07              or al,0x7
00002504  0800              or [rax],al
00002506  0000              add [rax],al
00002508  1C00              sbb al,0x0
0000250A  0000              add [rax],al
0000250C  9C                pushf
0000250D  0100              add [rax],eax
0000250F  00AEF6FFFF29      add [rsi+0x29fffff6],ch
00002515  0000              add [rax],al
00002517  0000              add [rax],al
00002519  41                rex.b
0000251A  0E                db 0x0e
0000251B  108602430D06      adc [rsi+0x60d4302],al
00002521  640C07            fs or al,0x7
00002524  0800              or [rax],al
00002526  0000              add [rax],al
00002528  1C00              sbb al,0x0
0000252A  0000              add [rax],al
0000252C  BC01000000        mov esp,0x1
00002531  0000              add [rax],al
00002533  001D00000000      add [rel 0x2539],bl
00002539  41                rex.b
0000253A  0E                db 0x0e
0000253B  108602430D06      adc [rsi+0x60d4302],al
00002541  58                pop rax
00002542  0C07              or al,0x7
00002544  0800              or [rax],al
00002546  0000              add [rax],al
00002548  1C00              sbb al,0x0
0000254A  0000              add [rax],al
0000254C  DC01              fadd qword [rcx]
0000254E  0000              add [rax],al
00002550  97                xchg eax,edi
00002551  F6FF              idiv bh
00002553  FF5300            call [rbx+0x0]
00002556  0000              add [rax],al
00002558  00410E            add [rcx+0xe],al
0000255B  108602430D06      adc [rsi+0x60d4302],al
00002561  024E0C            add cl,[rsi+0xc]
00002564  07                db 0x07
00002565  0800              or [rax],al
00002567  001C00            add [rax+rax],bl
0000256A  0000              add [rax],al
0000256C  FC                cld
0000256D  0100              add [rax],eax
0000256F  00CA              add dl,cl
00002571  F6FF              idiv bh
00002573  FF8500000000      inc dword [rbp+0x0]
00002579  41                rex.b
0000257A  0E                db 0x0e
0000257B  108602430D06      adc [rsi+0x60d4302],al
00002581  02800C070800      add al,[rax+0x8070c]
00002587  001C00            add [rax+rax],bl
0000258A  0000              add [rax],al
0000258C  1C02              sbb al,0x2
0000258E  0000              add [rax],al
00002590  2F                db 0x2f
00002591  F7FF              idiv edi
00002593  FF                db 0xff
00002594  3800              cmp [rax],al
00002596  0000              add [rax],al
00002598  00410E            add [rcx+0xe],al
0000259B  108602430D06      adc [rsi+0x60d4302],al
000025A1  730C              jnc 0x25af
000025A3  07                db 0x07
000025A4  0800              or [rax],al
000025A6  0000              add [rax],al
000025A8  1C00              sbb al,0x0
000025AA  0000              add [rax],al
000025AC  3C02              cmp al,0x2
000025AE  0000              add [rax],al
000025B0  47F7FF            idiv r15d
000025B3  FF8500000000      inc dword [rbp+0x0]
000025B9  41                rex.b
000025BA  0E                db 0x0e
000025BB  108602430D06      adc [rsi+0x60d4302],al
000025C1  02800C070800      add al,[rax+0x8070c]
000025C7  001C00            add [rax+rax],bl
000025CA  0000              add [rax],al
000025CC  5C                pop rsp
000025CD  0200              add al,[rax]
000025CF  00ACF7FFFF3800    add [rdi+rsi*8+0x38ffff],ch
000025D6  0000              add [rax],al
000025D8  00410E            add [rcx+0xe],al
000025DB  108602430D06      adc [rsi+0x60d4302],al
000025E1  730C              jnc 0x25ef
000025E3  07                db 0x07
000025E4  0800              or [rax],al
000025E6  0000              add [rax],al
000025E8  1C00              sbb al,0x0
000025EA  0000              add [rax],al
000025EC  7C02              jl 0x25f0
000025EE  0000              add [rax],al
000025F0  C4                db 0xc4
000025F1  F7FF              idiv edi
000025F3  FF8500000000      inc dword [rbp+0x0]
000025F9  41                rex.b
000025FA  0E                db 0x0e
000025FB  108602430D06      adc [rsi+0x60d4302],al
00002601  02800C070800      add al,[rax+0x8070c]
00002607  001C00            add [rax+rax],bl
0000260A  0000              add [rax],al
0000260C  9C                pushf
0000260D  0200              add al,[rax]
0000260F  0029              add [rcx],ch
00002611  F8                clc
00002612  FF                db 0xff
00002613  FF                db 0xff
00002614  3800              cmp [rax],al
00002616  0000              add [rax],al
00002618  00410E            add [rcx+0xe],al
0000261B  108602430D06      adc [rsi+0x60d4302],al
00002621  730C              jnc 0x262f
00002623  07                db 0x07
00002624  0800              or [rax],al
00002626  0000              add [rax],al
00002628  1C00              sbb al,0x0
0000262A  0000              add [rax],al
0000262C  BC02000041        mov esp,0x41000002
00002631  F8                clc
00002632  FF                db 0xff
00002633  FF                db 0xff
00002634  B800000000        mov eax,0x0
00002639  41                rex.b
0000263A  0E                db 0x0e
0000263B  108602430D06      adc [rsi+0x60d4302],al
00002641  02B30C070800      add dh,[rbx+0x8070c]
00002647  001C00            add [rax+rax],bl
0000264A  0000              add [rax],al
0000264C  DC02              fadd qword [rdx]
0000264E  0000              add [rax],al
00002650  D9F8              fprem
00002652  FF                db 0xff
00002653  FF2B              jmp dword far [rbx]
00002655  0000              add [rax],al
00002657  0000              add [rax],al
00002659  41                rex.b
0000265A  0E                db 0x0e
0000265B  108602430D06      adc [rsi+0x60d4302],al
00002661  660C07            o16 or al,0x7
00002664  0800              or [rax],al
00002666  0000              add [rax],al
00002668  1C00              sbb al,0x0
0000266A  0000              add [rax],al
0000266C  FC                cld
0000266D  0200              add al,[rax]
0000266F  00E4              add ah,ah
00002671  F8                clc
00002672  FF                db 0xff
00002673  FF                db 0xff
00002674  3D00000000        cmp eax,0x0
00002679  41                rex.b
0000267A  0E                db 0x0e
0000267B  108602430D06      adc [rsi+0x60d4302],al
00002681  780C              js 0x268f
00002683  07                db 0x07
00002684  0800              or [rax],al
00002686  0000              add [rax],al
00002688  2000              and [rax],al
0000268A  0000              add [rax],al
0000268C  1C03              sbb al,0x3
0000268E  0000              add [rax],al
00002690  0000              add [rax],al
00002692  0000              add [rax],al
00002694  5A                pop rdx
00002695  0000              add [rax],al
00002697  0000              add [rax],al
00002699  41                rex.b
0000269A  0E                db 0x0e
0000269B  108602430D06      adc [rsi+0x60d4302],al
000026A1  45830302          add dword [r11],byte +0x2
000026A5  50                push rax
000026A6  0C07              or al,0x7
000026A8  0800              or [rax],al
000026AA  0000              add [rax],al
000026AC  1C00              sbb al,0x0
000026AE  0000              add [rax],al
000026B0  400300            add eax,[rax]
000026B3  00DE              add dh,bl
000026B5  F8                clc
000026B6  FF                db 0xff
000026B7  FF1400            call [rax+rax]
000026BA  0000              add [rax],al
000026BC  00410E            add [rcx+0xe],al
000026BF  108602430D06      adc [rsi+0x60d4302],al
000026C5  4F0C07            o64 or al,0x7
000026C8  0800              or [rax],al
000026CA  0000              add [rax],al
000026CC  1C00              sbb al,0x0
000026CE  0000              add [rax],al
000026D0  60                db 0x60
000026D1  0300              add eax,[rax]
000026D3  00D2              add dl,dl
000026D5  F8                clc
000026D6  FF                db 0xff
000026D7  FF22              jmp [rdx]
000026D9  0000              add [rax],al
000026DB  0000              add [rax],al
000026DD  41                rex.b
000026DE  0E                db 0x0e
000026DF  108602430D06      adc [rsi+0x60d4302],al
000026E5  5D                pop rbp
000026E6  0C07              or al,0x7
000026E8  0800              or [rax],al
000026EA  0000              add [rax],al
000026EC  1C00              sbb al,0x0
000026EE  0000              add [rax],al
000026F0  800300            add byte [rbx],0x0
000026F3  0000              add [rax],al
000026F5  0000              add [rax],al
000026F7  003500000000      add [rel 0x26fd],dh
000026FD  41                rex.b
000026FE  0E                db 0x0e
000026FF  108602430D06      adc [rsi+0x60d4302],al
00002705  700C              jo 0x2713
00002707  07                db 0x07
00002708  0800              or [rax],al
0000270A  0000              add [rax],al
0000270C  1C00              sbb al,0x0
0000270E  0000              add [rax],al
00002710  A0030000B4F8FFFF  mov al,[qword 0x22fffff8b4000003]
         -22
00002719  0000              add [rax],al
0000271B  0000              add [rax],al
0000271D  41                rex.b
0000271E  0E                db 0x0e
0000271F  108602430D06      adc [rsi+0x60d4302],al
00002725  5D                pop rbp
00002726  0C07              or al,0x7
00002728  0800              or [rax],al
0000272A  0000              add [rax],al
0000272C  1C00              sbb al,0x0
0000272E  0000              add [rax],al
00002730  C00300            rol byte [rbx],byte 0x0
00002733  00B6F8FFFF26      add [rsi+0x26fffff8],dh
00002739  0000              add [rax],al
0000273B  0000              add [rax],al
0000273D  41                rex.b
0000273E  0E                db 0x0e
0000273F  108602430D06      adc [rsi+0x60d4302],al
00002745  61                db 0x61
00002746  0C07              or al,0x7
00002748  0800              or [rax],al
0000274A  0000              add [rax],al
0000274C  1C00              sbb al,0x0
0000274E  0000              add [rax],al
00002750  E003              loopne 0x2755
00002752  0000              add [rax],al
00002754  BCF8FFFFF3        mov esp,0xf3fffff8
00002759  0000              add [rax],al
0000275B  0000              add [rax],al
0000275D  41                rex.b
0000275E  0E                db 0x0e
0000275F  108602430D06      adc [rsi+0x60d4302],al
00002765  02EE              add ch,dh
00002767  0C07              or al,0x7
00002769  0800              or [rax],al
0000276B  001C00            add [rax+rax],bl
0000276E  0000              add [rax],al
00002770  000400            add [rax+rax],al
00002773  0090F9FFFFF3      add [rax-0xc000007],dl
00002779  0000              add [rax],al
0000277B  0000              add [rax],al
0000277D  41                rex.b
0000277E  0E                db 0x0e
0000277F  108602430D06      adc [rsi+0x60d4302],al
00002785  02EE              add ch,dh
00002787  0C07              or al,0x7
00002789  0800              or [rax],al
0000278B  001C00            add [rax+rax],bl
0000278E  0000              add [rax],al
00002790  200400            and [rax+rax],al
00002793  0064FAFF          add [rdx+rdi*8-0x1],ah
00002797  FF                db 0xff
00002798  3B00              cmp eax,[rax]
0000279A  0000              add [rax],al
0000279C  00410E            add [rcx+0xe],al
0000279F  108602430D06      adc [rsi+0x60d4302],al
000027A5  760C              jna 0x27b3
000027A7  07                db 0x07
000027A8  0800              or [rax],al
000027AA  0000              add [rax],al
000027AC  1C00              sbb al,0x0
000027AE  0000              add [rax],al
000027B0  400400            add al,0x0
000027B3  007FFA            add [rdi-0x6],bh
000027B6  FF                db 0xff
000027B7  FF                db 0xff
000027B8  3F                db 0x3f
000027B9  0000              add [rax],al
000027BB  0000              add [rax],al
000027BD  41                rex.b
000027BE  0E                db 0x0e
000027BF  108602430D06      adc [rsi+0x60d4302],al
000027C5  7A0C              jpe 0x27d3
000027C7  07                db 0x07
000027C8  0800              or [rax],al
000027CA  0000              add [rax],al
000027CC  1C00              sbb al,0x0
000027CE  0000              add [rax],al
000027D0  60                db 0x60
000027D1  0400              add al,0x0
000027D3  009EFAFFFF9D      add [rsi-0x62000006],bl
000027D9  0000              add [rax],al
000027DB  0000              add [rax],al
000027DD  41                rex.b
000027DE  0E                db 0x0e
000027DF  108602430D06      adc [rsi+0x60d4302],al
000027E5  02980C070800      add bl,[rax+0x8070c]
000027EB  001C00            add [rax+rax],bl
000027EE  0000              add [rax],al
000027F0  80040000          add byte [rax+rax],0x0
000027F4  1BFB              sbb edi,ebx
000027F6  FF                db 0xff
000027F7  FF                db 0xff
000027F8  3F                db 0x3f
000027F9  0000              add [rax],al
000027FB  0000              add [rax],al
000027FD  41                rex.b
000027FE  0E                db 0x0e
000027FF  108602430D06      adc [rsi+0x60d4302],al
00002805  7A0C              jpe 0x2813
00002807  07                db 0x07
00002808  0800              or [rax],al
0000280A  0000              add [rax],al
0000280C  1C00              sbb al,0x0
0000280E  0000              add [rax],al
00002810  A00400003AFBFFFF  mov al,[qword 0x22fffffb3a000004]
         -22
00002819  0000              add [rax],al
0000281B  0000              add [rax],al
0000281D  41                rex.b
0000281E  0E                db 0x0e
0000281F  108602430D06      adc [rsi+0x60d4302],al
00002825  4183035C          add dword [r11],byte +0x5c
00002829  0C07              or al,0x7
0000282B  081C00            or [rax+rax],bl
0000282E  0000              add [rax],al
00002830  C0040000          rol byte [rax+rax],byte 0x0
00002834  0000              add [rax],al
00002836  0000              add [rax],al
00002838  2200              and al,[rax]
0000283A  0000              add [rax],al
0000283C  00410E            add [rcx+0xe],al
0000283F  108602430D06      adc [rsi+0x60d4302],al
00002845  4183035C          add dword [r11],byte +0x5c
00002849  0C07              or al,0x7
0000284B  0820              or [rax],ah
0000284D  0000              add [rax],al
0000284F  00E0              add al,ah
00002851  0400              add al,0x0
00002853  002A              add [rdx],ch
00002855  DF                db 0xdf
00002856  FF                db 0xff
00002857  FF4C0100          dec dword [rcx+rax+0x0]
0000285B  0000              add [rax],al
0000285D  41                rex.b
0000285E  0E                db 0x0e
0000285F  108602430D06      adc [rsi+0x60d4302],al
00002865  45830303          add dword [r11],byte +0x3
00002869  42010C07          add [rdi+r8],ecx
0000286D  0800              or [rax],al
0000286F  001C00            add [rax+rax],bl
00002872  0000              add [rax],al
00002874  0405              add al,0x5
00002876  0000              add [rax],al
00002878  52                push rdx
00002879  E0FF              loopne 0x287a
0000287B  FFA900000000      jmp dword far [rcx+0x0]
00002881  41                rex.b
00002882  0E                db 0x0e
00002883  108602430D06      adc [rsi+0x60d4302],al
00002889  02A40C07080000    add ah,[rsp+rcx+0x807]
00002890  1C00              sbb al,0x0
00002892  0000              add [rax],al
00002894  2405              and al,0x5
00002896  0000              add [rax],al
00002898  DCE0              fsubr to st0
0000289A  FF                db 0xff
0000289B  FFC1              inc ecx
0000289D  0200              add al,[rax]
0000289F  0000              add [rax],al
000028A1  41                rex.b
000028A2  0E                db 0x0e
000028A3  108602430D06      adc [rsi+0x60d4302],al
000028A9  03BC020C070800    add edi,[rdx+rax+0x8070c]
000028B0  1C00              sbb al,0x0
000028B2  0000              add [rax],al
000028B4  440500007DE3      add eax,0xe37d0000
000028BA  FF                db 0xff
000028BB  FF1F              call dword far [rdi]
000028BD  0000              add [rax],al
000028BF  0000              add [rax],al
000028C1  41                rex.b
000028C2  0E                db 0x0e
000028C3  108602430D06      adc [rsi+0x60d4302],al
000028C9  5A                pop rdx
000028CA  0C07              or al,0x7
000028CC  0800              or [rax],al
000028CE  0000              add [rax],al
000028D0  1C00              sbb al,0x0
000028D2  0000              add [rax],al
000028D4  640500007CE3      fs add eax,0xe37c0000
000028DA  FF                db 0xff
000028DB  FF5E00            call dword far [rsi+0x0]
000028DE  0000              add [rax],al
000028E0  00410E            add [rcx+0xe],al
000028E3  108602430D06      adc [rsi+0x60d4302],al
000028E9  02590C            add bl,[rcx+0xc]
000028EC  07                db 0x07
000028ED  0800              or [rax],al
000028EF  001C00            add [rax+rax],bl
000028F2  0000              add [rax],al
000028F4  84050000BAE3      test [rel 0xffffffffe3ba28fa],al
000028FA  FF                db 0xff
000028FB  FF1B              call dword far [rbx]
000028FD  0000              add [rax],al
000028FF  0000              add [rax],al
00002901  41                rex.b
00002902  0E                db 0x0e
00002903  108602430D06      adc [rsi+0x60d4302],al
00002909  56                push rsi
0000290A  0C07              or al,0x7
0000290C  0800              or [rax],al
0000290E  0000              add [rax],al
00002910  1C00              sbb al,0x0
00002912  0000              add [rax],al
00002914  A4                movsb
00002915  050000B5E3        add eax,0xe3b50000
0000291A  FF                db 0xff
0000291B  FF1F              call dword far [rdi]
0000291D  0000              add [rax],al
0000291F  0000              add [rax],al
00002921  41                rex.b
00002922  0E                db 0x0e
00002923  108602430D06      adc [rsi+0x60d4302],al
00002929  5A                pop rdx
0000292A  0C07              or al,0x7
0000292C  0800              or [rax],al
0000292E  0000              add [rax],al
00002930  1C00              sbb al,0x0
00002932  0000              add [rax],al
00002934  C4                db 0xc4
00002935  050000B4E3        add eax,0xe3b40000
0000293A  FF                db 0xff
0000293B  FF1F              call dword far [rdi]
0000293D  0000              add [rax],al
0000293F  0000              add [rax],al
00002941  41                rex.b
00002942  0E                db 0x0e
00002943  108602430D06      adc [rsi+0x60d4302],al
00002949  5A                pop rdx
0000294A  0C07              or al,0x7
0000294C  0800              or [rax],al
0000294E  0000              add [rax],al
00002950  1C00              sbb al,0x0
00002952  0000              add [rax],al
00002954  E405              in al,0x5
00002956  0000              add [rax],al
00002958  B3E3              mov bl,0xe3
0000295A  FF                db 0xff
0000295B  FF1B              call dword far [rbx]
0000295D  0000              add [rax],al
0000295F  0000              add [rax],al
00002961  41                rex.b
00002962  0E                db 0x0e
00002963  108602430D06      adc [rsi+0x60d4302],al
00002969  56                push rsi
0000296A  0C07              or al,0x7
0000296C  0800              or [rax],al
0000296E  0000              add [rax],al
00002970  1C00              sbb al,0x0
00002972  0000              add [rax],al
00002974  0406              add al,0x6
00002976  0000              add [rax],al
00002978  AE                scasb
00002979  E3FF              jrcxz 0x297a
0000297B  FF26              jmp [rsi]
0000297D  0000              add [rax],al
0000297F  0000              add [rax],al
00002981  41                rex.b
00002982  0E                db 0x0e
00002983  108602430D06      adc [rsi+0x60d4302],al
00002989  61                db 0x61
0000298A  0C07              or al,0x7
0000298C  0800              or [rax],al
0000298E  0000              add [rax],al
00002990  1C00              sbb al,0x0
00002992  0000              add [rax],al
00002994  2406              and al,0x6
00002996  0000              add [rax],al
00002998  B4E3              mov ah,0xe3
0000299A  FF                db 0xff
0000299B  FF1B              call dword far [rbx]
0000299D  0000              add [rax],al
0000299F  0000              add [rax],al
000029A1  41                rex.b
000029A2  0E                db 0x0e
000029A3  108602430D06      adc [rsi+0x60d4302],al
000029A9  56                push rsi
000029AA  0C07              or al,0x7
000029AC  0800              or [rax],al
000029AE  0000              add [rax],al
000029B0  1C00              sbb al,0x0
000029B2  0000              add [rax],al
000029B4  44                rex.r
000029B5  06                db 0x06
000029B6  0000              add [rax],al
000029B8  AF                scasd
000029B9  E3FF              jrcxz 0x29ba
000029BB  FF1F              call dword far [rdi]
000029BD  0000              add [rax],al
000029BF  0000              add [rax],al
000029C1  41                rex.b
000029C2  0E                db 0x0e
000029C3  108602430D06      adc [rsi+0x60d4302],al
000029C9  5A                pop rdx
000029CA  0C07              or al,0x7
000029CC  0800              or [rax],al
000029CE  0000              add [rax],al
000029D0  0000              add [rax],al
000029D2  0000              add [rax],al
000029D4  0F1F4000          nop dword [rax+0x0]
000029D8  1400              adc al,0x0
000029DA  0000              add [rax],al
000029DC  0000              add [rax],al
000029DE  0000              add [rax],al
000029E0  017A52            add [rdx+0x52],edi
000029E3  0001              add [rcx],al
000029E5  7810              js 0x29f7
000029E7  011B              add [rbx],ebx
000029E9  0C07              or al,0x7
000029EB  08900100001C      or [rax+0x1c000001],dl
000029F1  0000              add [rax],al
000029F3  001C00            add [rax+rax],bl
000029F6  0000              add [rax],al
000029F8  8EE3              mov fs,ebx
000029FA  FF                db 0xff
000029FB  FF13              call [rbx]
000029FD  0000              add [rax],al
000029FF  0000              add [rax],al
00002A01  41                rex.b
00002A02  0E                db 0x0e
00002A03  108602430D06      adc [rsi+0x60d4302],al
00002A09  4E0C07            o64 or al,0x7
00002A0C  0800              or [rax],al
00002A0E  0000              add [rax],al
00002A10  1C00              sbb al,0x0
00002A12  0000              add [rax],al
00002A14  3C00              cmp al,0x0
00002A16  0000              add [rax],al
00002A18  81E3FFFF2300      and ebx,0x23ffff
00002A1E  0000              add [rax],al
00002A20  00410E            add [rcx+0xe],al
00002A23  108602430D06      adc [rsi+0x60d4302],al
00002A29  5E                pop rsi
00002A2A  0C07              or al,0x7
00002A2C  0800              or [rax],al
00002A2E  0000              add [rax],al

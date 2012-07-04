# What is it
--------------------------------------------------------------------
A modifiable DCPU-16 assembler, you can add instructions sets under instr_sets/

I started doing this because I wanted an assembler that could support
any Microchip MCU. Using the syntax in the description
of the instruction set from the Microchip MCU Datasheets.

Currently I have added the DCPU-16 instruction set from the game 0x10c.

## Tests
--------------------------------------------------------------------
This is the output from the assembler as it looks now with DCPU specification loaded.


*Debug/Work in progress/Output looks like this, this is assembled
*using forced long pointers, to compare with the sample code from DCPU-16 1.1 spec.
--------------------------------------------------------------------
	SET A, 0x30, ..OK, 0x30(16) 0x7C01(16) current size: 32
	SET P[0x1000], P[n] 0x1000, 0x20, ..OK, 0x1000(16) 0x20(16) 0x7DE1(16) current size: 80
	SUB A, P[0x1000], P[n] 0x1000, ..OK, 0x1000(16) 0x7803(16) current size: 112
	IFN A, 0x10, ..OK, 0xC00D(16) current size: 128
	SET PC, L[crash], L[n] crash, ..OK, 0x1A(16) 0x7DC1(16) current size: 160
	SET I, S[10], S[v] 10, ..OK, 0xA861(16) current size: 176
	SET A, 0x2000, ..OK, 0x2000(16) 0x7C01(16) current size: 208
	found tag: loop
	SET [0x2000+I], [n+a] 0x2000, I, R[A], R[a] A, ..OK, 0x2000(16) 0x2161(16) current size: 240
	SUB I, 1, ..OK, 0x8463(16) current size: 256
	IFN I, 0, ..OK, 0x806D(16) current size: 272
	SET PC, L[loop], L[n] loop, ..OK, 0xD(16) 0x7DC1(16) current size: 304
	SET X, 0x4, ..OK, 0x9031(16) current size: 320
	JSR L[testsub], L[n] testsub, ..OK, 0x18(16) 0x7C10(16) current size: 352
	SET PC, L[crash], L[n] crash, ..OK, 0x1A(16) 0x7DC1(16) current size: 384
	found tag: testsub
	SHL X, 4, ..OK, 0x9037(16) current size: 400
	SET PC, POP, ..OK, 0x61C1(16) current size: 416
	found tag: crash
	SET PC, L[crash], L[n] crash, ..OK, 0x1A(16) 0x7DC1(16) current size: 448
	Assembled OK.

*Data output
      0x0000: 0x7C01 0x0030 0x7DE1 0x1000 0x0020 0x7803 0x1000 0xC00D 
      0x0008: 0x7DC1 0x001A 0xA861 0x7C01 0x2000 0x2161 0x2000 0x8463 
      0x0010: 0x806D 0x7DC1 0x000D 0x9031 0x7C10 0x0018 0x7DC1 0x001A 
      0x0018: 0x9037 0x61C1 0x7DC1 0x001A 

*This is assembled using automatic choice of long/short pointers.
-----------------------------------------------------------------------------------------
	SET A, 0x30, ..OK, 0x30(16) 0x7C01(16) current size: 32
	SET [0x1000], [n] 0x1000, 0x20, ..OK, 0x1000(16) 0x20(16) 0x7DE1(16) current size: 80
	SUB A, [0x1000], [n] 0x1000, ..OK, 0x1000(16) 0x7803(16) current size: 112
	IFN A, 0x10, ..OK, 0xC00D(16) current size: 128
	SET PC, crash, ..OK, 0xD9C1(16) current size: 144
	SET I, 10, ..OK, 0xA861(16) current size: 160
	SET A, 0x2000, ..OK, 0x2000(16) 0x7C01(16) current size: 192
	found tag: loop
	SET [0x2000+I], [n+a] 0x2000, I, [A], [a] A, ..OK, 0x2000(16) 0x2161(16) current size: 224
	SUB I, 1, ..OK, 0x8463(16) current size: 240
	IFN I, 0, ..OK, 0x806D(16) current size: 256
	SET PC, loop, ..OK, 0xB1C1(16) current size: 272
	SET X, 0x4, ..OK, 0x9031(16) current size: 288
	JSR testsub, ..OK, 0xD010(16) current size: 304
	SET PC, crash, ..OK, 0xD9C1(16) current size: 320
	found tag: testsub
	SHL X, 4, ..OK, 0x9037(16) current size: 336
	SET PC, POP, ..OK, 0x61C1(16) current size: 352
	found tag: crash
	SET PC, crash, ..OK, 0xD9C1(16) current size: 368
	Assembled OK.

	0x0000: 0x7C01 0x0030 0x7DE1 0x1000 0x0020 0x7803 0x1000 0xC00D 
	0x0008: 0xD9C1 0xA861 0x7C01 0x2000 0x2161 0x2000 0x8463 0x806D 
	0x0010: 0xB1C1 0x9031 0xD010 0xD9C1 0x9037 0x61C1 0xD9C1 

** Manually testing if SET PC, crash was assembled OK **
    Assembled: 0xD9C1 -> 1101100111000001 ; Reference
    SET Instruction   -> bbbbbbaaaaaa0001
    PC Hardsymbol     ->       011100
    Short liteal form -> 1vvvvv
    Crash value:352/16->  10110
    Result            -> 1101100111000001


*And with 1.7 Spec
-----------------------------------------------------------------------------------------
	0x0000: 0x7C01 0x0030 0x7FC1 0x1000 0x0020 0x7803 0x1000 0xC013 
	0x0008: 0xDB81 0xA8C1 0x7C01 0x2000 0x22C1 0x2000 0x84C3 0x80D3 
	0x0010: 0xB381 0x9061 0xD020 0xDB81 0x906F 0x6381 0xDB81 

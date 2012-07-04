# What is it
---------------------------------------------------
A modifiable DCPU-16 assembler, you can add instructions sets under instr_sets/

I started doing this because I wanted an assembler that could support
any Microchip MCU. Using the syntax in the description
of the instruction set from the Microchip MCU Datasheets.

Currently I have added the DCPU-16 instruction set from the game 0x10c.

# Howto Assemble a file
---------------------------------------------------
To assemble using the 1.1 specification

 ./as -set instr_sets/DCPU-16.set asmfile.asm

To assemble using the 1.7 specification

 ./as -set instr_sets/DCPU-16_1-7.set asmfile.asm


# Howto make a set file
--------------------------------------------------
Take the DCPU-16.set file as an example

     ; Line comments can be made like this
     ; Instruction name : Parameters : Bit pattern :
     -INSTRUCTIONS
     SET:a,b:bbbbbbaaaaaa0001:
     ADD:a,b:bbbbbbaaaaaa0010:
     SUB:a,b:bbbbbbaaaaaa0011:
     JSR:a  :aaaaaa0000010000:
     ;... More instructions follow

There are three options for an instruction separeted with ':'. First we specify the name of the instruction, then a list of parameters that the instruction take. JSR only takes one parameter. The last part describes the value of the opcode in base 2/binary. You can clearly see where the parameters are parsed into the opcode. Take the parameter 'a' as an example, you can see it have to fit into 6 bits, so it cannot have a value larger than 2^6, or the assembler will fail with an error.

with this information we can not write code like this

     SET 0x00, 0x01
     ADD 0x00, 0x05

But we want to be able to use pointers, and raw literals for the DCPU-16 set are coded in a certain way. So we have this part of the set file to describe subarguments.
 -ARGUMENTS
 [a]  :a  :001aaa:                ::[ABCXYZIJ]\{1\}                     :All:
 [n+a]:n,a:010aaa:nnnnnnnnnnnnnnnn::[a-zA-Z0-9]\{1,100\},[ABCXYZIJ]\{1\}:All:
 [n]  :n  :011110:nnnnnnnnnnnnnnnn::                                    :All:
 n    :n  :011111:nnnnnnnnnnnnnnnn::                                    :All:
 v    :v  :1vvvvv:                ::                                    :All:

This 7 parts separated with colons. First a simple regular expression for the argument ([a]), then a list of subarguments (a), then the value that is used for that argument in base 2/binary (001aaa). Then comes an overflow value, this value will be added after the opcode, not parsed into it. We have some extra colons here, this is not used for anything yet. Then we have en extra regular expression for each subargument ([ABCXYZIJ]), and the value 'All' says that the subargument can be used for all instructions, otherwise we specify a list of instructions here.

So now, we can write code like this

 SET 0x00, [X]

This will match for the first subargument. This will chose the value '001aaa' for the second argument

 SET 0x00, [0x00]

This will NOT work, because of the extra regular expression. In the first subargument, it only allows A,B,C,X,Y,Z,I,J to be specified inside the brackets. 

 SET 0x00, 10

This will match for the last subargument '1vvvvv' only allows values between 0 - 2^5.

 SET 0x00, 0x1000

This will match for the argument that makes the value '011111'. It will add 0x1000 to an extra 16 bits after the instruction.

 SET [0x1000+X], 0x0

This will match for the [n+a] argument.

If no extra regular expression is defined, [a-zA-Z0-9]{1,100} is used. But we still miss something. X doesn't have a value. The assembler still needs to know what X is, otherwise we will have lines in our code marked as UNDEFINED, and the assembling will not complete.

Under Hardsymbols we specify the value that X has. As you can see we have 'All' here also, because in the future we might want to lock a certain symbol to a specific instruction or argument. The same symbol might have a different value for a different instruction or argument.

   -HARDSYMBOLS
   A          :000000:All:
   B          :000001:All:
   C          :000010:All:
   X          :000011:All:
   Y          :000100:All:
   Z          :000101:All:
   I          :000110:All:
   J          :000111:All:
   
Next we also have a list of normal Symbols, under the -SYMBOLS tag. These symbols are equal to normal numbers, they can be specified with #define in the code. Or in the set file under -SYMBOLS with the same syntax as hardsymbols. The difference between symbols and hardsymbols is that a hardsymbol will not be encoded with a argument rule, it will be put straight into the opcode. Example

     SET X, B ; X has the value 000011 and B the value 000001

this is different from writing

     SET X, 1 ; Here 1 will have the value 100001 because of the argument
     	      ; which matched for the literal 1 -> '1vvvvv'.


## Tests
---------------------------------------------------
This is the output from the assembler as it looks now with DCPU specification loaded.


### Debug/Work in progress/Output looks like this, this is assembled using forced long pointers, to compare with the sample code from DCPU-16 1.1 spec.
---------------------------------------------------
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

#### Data output
      0x0000: 0x7C01 0x0030 0x7DE1 0x1000 0x0020 0x7803 0x1000 0xC00D 
      0x0008: 0x7DC1 0x001A 0xA861 0x7C01 0x2000 0x2161 0x2000 0x8463 
      0x0010: 0x806D 0x7DC1 0x000D 0x9031 0x7C10 0x0018 0x7DC1 0x001A 
      0x0018: 0x9037 0x61C1 0x7DC1 0x001A 

### This is assembled using automatic choice of long/short pointers.
---------------------------------------------------
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

#### Data output
	0x0000: 0x7C01 0x0030 0x7DE1 0x1000 0x0020 0x7803 0x1000 0xC00D 
	0x0008: 0xD9C1 0xA861 0x7C01 0x2000 0x2161 0x2000 0x8463 0x806D 
	0x0010: 0xB1C1 0x9031 0xD010 0xD9C1 0x9037 0x61C1 0xD9C1 

#### Manually testing if SET PC, crash was assembled OK
    Assembled: 0xD9C1 -> 1101100111000001 ; Reference
    SET Instruction   -> bbbbbbaaaaaa0001
    PC Hardsymbol     ->       011100
    Short liteal form -> 1vvvvv
    Crash value:352/16->  10110
    Result            -> 1101100111000001


### And with 1.7 Spec
---------------------------------------------------
	0x0000: 0x7C01 0x0030 0x7FC1 0x1000 0x0020 0x7803 0x1000 0xC013 
	0x0008: 0xDB81 0xA8C1 0x7C01 0x2000 0x22C1 0x2000 0x84C3 0x80D3 
	0x0010: 0xB381 0x9061 0xD020 0xDB81 0x906F 0x6381 0xDB81 

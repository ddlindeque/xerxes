# Project XERXES
The idea of this project is to <i>build</i> a computer from scratch, albeit a virtual one. The current plan is as follows:
* Phase 1
  * Create the virtual hardware
    * [x] CPU
    * [x] System Bus
    * [ ] Machine with a system clock
    * [ ] Punch card reader
    * [ ] Monitor
    * [ ] Keyboard
  * Create an Operating System
    * [ ] Encode a text editor using punch cards
    * [ ] Encode an assembler using punch cards
    * [ ] Encode a compiler using assembly
    * [ ] Encode an operating system using the compiler

## Virtual CPU
Base the CPU on the 6502.

## Virtual System Bus
CPU's and Devices connect to the system bus only. CPU's and devices all read and write to addresses on the system bus.

## Virtual machine
The virtual machine will host a system bus, a set of CPU's and collection of other devices. It will also run a clock to clock the system.

## Virtual punch card reader
A punch card will be represented by a text file, and the reader will read this file as if it is a punch card. The punch card will have random access capability, thus each byte will be addressable. Each byte represented by the punch card will be one line in the file. Each line will have 1's and 0's representing the holes.
````
0 1 0 0  1 0 1 1
0 0 1 0  0 0 0 0
````
All whitespace (except line ends) are ignored. We have line comments starting with #, representing written notes on the cards.

## Text editor
The text editor is the simplest editor. All that is needed is to be able to add, change and delete characters in a file. Almost like the simplest version of vi. The text editor edits text in a specific location in RAM.

## Assembler
The first version of the assembler is the simplest assembler, to read a file of assembly instructions and encode them to binary code. The source text will be read from a specific hard coded address (where the text editor operate), and written to a specific hard coded address.
````asm
LDA #$01
STA $0200
LDA #$05
STA $0201
LDA #$08
STA $0202
````

## Compiler
The first compiler will be a non-optimizing compiler. We will have a lexer, parser and code generator. The language will be C-like and will include the <i>asm</i> syntax for raw assembly code.

## Operating system
The operating system is basically just a terminal window. It includes an 'nix kind of file system. It also includes the <i>pipes and filters</i> system including stdout, stderr and stdin. It also includes the '>', '<', '>>' and '|' operators.

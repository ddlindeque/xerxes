# The <i>intern</i>
## Addressing modes
* Immediate  
The next byte is the value
```
LDA #$45
```
* Absolute  
The next two bytes is the address of the value
```
LDA $1234
```
* Absolute offset X  
The next two bytes is the address with offset in X
```
LDA $1234,X
```
* Absolute offset Y  
The nexst two bytes is the address with offset in Y
```
LDA $1234,Y
```
* Zero Page  
The next byte is the address in page zero
```
LDA $12
```
* Zero Page offset X  
The next byte is the address in page zero with offset in X
```
LDA $12,X
```
* Zero Page offset Y  
The next byte is the address in page zero with offset in Y
```
LDA $12,Y
```
* Indirect  
The next byte is an address in page zero. The two bytes at that address is the address of the value.
```
LDA ($12)
```
* Indirect offset X  
The next byte is an address in page zero offset by value in X. The two bytes at that address is the address of the value. Offset is of the first address (before indirection).
```
LDA ($12,X)
```
* Indirect offset Y
The next byte is an address in page zero. The two bytes at that address is the address of the value offset by the value in Y. Offset is of the second address (after indirection).
```
LDA ($12),Y
```
* Register A
The operation will be performed directly on register A.
```
INC A
```
* Register X
The operation will be performed directly on register X.
```
INC X
```
* Register Y
The operation will be performed directly on register Y.
```
INC Y
```
## Assembly representation of addressing modes
* Immediate  
To specify an immediate addressing mode, the <i>address</i> is specified using a literal value. The literal value can be specified as a decimal number, i.e. ```1000```, or a hex number, i.e. ```$3E8```.
```asm
BASE $200
CSTR "Hello world" ; This takes 11 bytes of memory starting at 0x0200
@myaddr1: CLI ; Instruction at address labelled 'myaddr1'

BASE $300
LDA $200 ; Load the byte at address 0x0200
```
* Absolute  
Absolute addressing mode is specified using a label. If the label is in page zero, then page zero addressing is used.
```asm
BASE $200
CSTR "Hello world" ; This takes 11 bytes of memory starting at 0x0200
@myaddr1: CLI ; Instruction at address labelled 'myaddr1'

BASE $300
LDA @myaddr1 ; Load the byte at label '@myaddr1'
```
* Absolute offset X  
Specified using a label and <i>+X</i>
```asm
LDA @myaddr1+X ; Load the byte at @myaddress1 + (value of X)
```
* Absolute offset Y  
Specified using a label and <i>+Y</i>
```asm
LDA @myaddr1+Y ; Load the byte at @myaddress1 + (value of Y)
```
* Zero Page, Zero Page X and Zero Page Y  
Same as Absolute ?, but label is address in page zero.
* Indirect  
Specify using a <i>derefence</i> of a label. The label **must** be an address in page zero.
```asm
LDA (@myaddr)
```
* Indirect offset X  
Specify using a <i>dereference</i> of a label with the X value
```asm
LDA (@myaddr+X)
```
* Indirect offset Y  
Specify using a <i>dereference</i> of a label with the Y value
```asm
LDA (@myaddr)+Y
```
* Register A, X, Y  
Specify the register directly
```asm
INC A
INC X
INC Y
```
## Syntax
The syntax has the following features
* Whitespace is ignored
* Comments start with ';' to the end of the line.
* Statements can be metadata, data or instructions.
  * Metadata  
    The only metadata statement is the <i>BASE</i> statement which indicates where in memory the next statement should be located.
    ```
    BASE $200 ; The next statement will be located at address $200
    ```
  * Data  
    Data statements are statements to write data to memory locations.
    * Single bytes  
      ```
      DATA $12 ; Writes $12 to this address
      ```
    * Two bytes  
      ```
      DATA $1234 ; Writes $34 to this address and $12 to the next (note the lo byte goes first)
      ```
    * Character  
      ```
      DATA 'a' ; Write the ASCII value of 'a' to this address
      ```
    * String  
      ```
      DATA "This is my string" ; 'T' at this address, 'h' the next, etc.
      ```
* A statement has an optional label.
```
DATA $12 ; No label
@mylabel1: DATA $12 ; Label '@mylabel1' defined.
```
* Instructions can be any of the following:
  * BRK
  * ADC [addr]
  * SBC [addr]
  * AND [addr]
  * ASL [addr]
  * BCC [addr]
  * BCS [addr]
  * BEQ [addr]
  * BMI [addr]
  * BNE [addr]
  * BPL [addr]
  * BRA [addr]
  * BVC [addr]
  * BVS [addr]
  * BIT [addr]
  * CLC
  * CLD
  * CLI
  * CLV
  * SEC
  * SED
  * SEI
  * CMP [addr]
  * CPX [addr]
  * CPY [addr]
  * DEC [addr]
  * INC [addr]
  * EOR [addr]
  * JMP [addr]
  * JSR [addr]
  * LDA [addr]
  * LDX [addr]
  * LDY [addr]
  * LSR [addr]
  * NOP
  * OR [addr]
  * PHA
  * PHP
  * PHX
  * PLA
  * PLP
  * PLX
  * PLY
  * ROL [addr]
  * RTI
  * RTS
  * ROR [addr]
  * STA [addr]
  * STY [addr]
  * STX [addr]
  * TAX
  * TXA
  * TAY
  * TYA
  * TSX
  * TXS
### Document
```bison
DOCUMENT: INSTRUCTIONS
INSTRUCTIONS:
    INSTRUCTION
  | INSTRUCTIONS '\n' INSTRUCTION
INSTRUCTION:
    opcode
  | opcode PARAMETER
  | label ':' opcode
  | label ':' opcode PARAMETER
PARAMETER:
    number
  | char
  | string
  | label
  | label '+' 'X'
  | label '+' 'Y'
  | '(' label ')'
  | '(' label '+' 'X' ');
  | '(' label ')' + 'Y'
  | 'A'
  | 'X'
  | 'Y'
```
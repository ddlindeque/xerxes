# The <i>intern</i>
## Addressing modes
* Immediate  
The next byte is the value
```
LDA $45
```
* Absolute  
The next two bytes is the address of the value
```
LDA #$1234
```
* Absolute offset X  
The next two bytes is the address with offset in X
```
LDA #$1234 + X
```
* Absolute offset Y  
The nexst two bytes is the address with offset in Y
```
LDA #$1234 + Y
```
* Zero Page  
The next byte is the address in page zero
```
LDA #$12
```
* Zero Page offset X  
The next byte is the address in page zero with offset in X
```
LDA #$12 + X
```
* Zero Page offset Y  
The next byte is the address in page zero with offset in Y
```
LDA #$12 + Y
```
* Indirect  
The next byte is an address in page zero. The two bytes at that address is the address of the value.
```
LDA [#$12]
```
* Indirect offset X  
The next byte is an address in page zero offset by value in X. The two bytes at that address is the address of the value. Offset is of the first address (before indirection).
```
LDA [#$12 + X]
```
* Indirect offset Y
The next byte is an address in page zero. The two bytes at that address is the address of the value offset by the value in Y. Offset is of the second address (after indirection).
```
LDA [#$12] + Y
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
To specify an immediate addressing mode, the <i>value</i> is specified using a number expression. The literal number must be a single byte only.
```asm
BASE $0300
LDA $20 ; Load the value $20 into A
```
* Absolute  
Absolute addressing mode is specified using an address expresison. If the address is in page zero, then page zero addressing is used.
```asm
BASE $0200
@myaddr1: CLI ; Instruction at address labelled 'myaddr1'

BASE $0300
LDA @myaddr1 ; Load the byte at label '@myaddr1'
```
* Absolute offset X  
Specified using an address expression and <i>+X</i>
```asm
LDA @myaddr1+X ; Load the byte at @myaddress1 + (value of X)
```
* Absolute offset Y  
Specified using an address expression and <i>+Y</i>
```asm
LDA @myaddr1+Y ; Load the byte at @myaddress1 + (value of Y)
```
* Zero Page, Zero Page X and Zero Page Y  
Same as Absolute, but the address is in page zero.
* Indirect  
We must read the address from the address specified.
```asm
LDA [@myaddr]
```
* Indirect offset X  
Specify using an address to an address with offset X
```asm
LDA [@myaddr+X]
```
* Indirect offset Y  
Specify using an address with offset Y of an address
```asm
LDA [@myaddr]+Y
```
* Register A, X, Y  
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
    * The <i>BASE</i> statement indicates where in memory the next statement should be located.  
    ```
    BASE #$0200 ; The next statement will be located at address #$0200
    ; To make a literal an address, use the #
    ```
    * The <i>START</i> statement indicates where in memory the executable program starts.
    ```
    START #$0200 ; The executable program starts at address 200 (hex)
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
    * We can also write the address of a label, which would write two byte (lo byte first)
      ```
      DATA @someLabel
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
* Constants can be defined using the %name syntax.
```
%baseAddress = $0200 + %ofs
BASE %baseAddress
```
* Expressions can be used anywhere a literal value is required. Precedence can be specified using the '(' ')' syntax, i.e.: ```%var = (%x + 3) * %y```. The following operators will be supported:
  * ```+```
  * ```-```
  * ```*```
### Document
```yacc
INSTRUCTION:
    instr
  | instr EXPRESSION
  | label ':' INSTRUCTION
  | constant '=' EXPRESSION

EXPRESSION:
    byte
  | word
  | constant
  | label
  | reg
  | '#' EXPRESSION
  | '(' EXPRESSION ')'
  | EXPRESSION '+' EXPRESSION
  | EXPRESSION '-' EXPRESSION
  | EXPRESSION '*' EXPRESSION
  | '[' EXPRESSION ']'
  | 'lo' '(' EXPRESSION ')'
  | 'hi' '(' EXPRESSION ')'
```
### Type system
#### Supported types
An expression can have any of the following types
* byte
* word
* addr
* reference (address of an address or a register)
#### Expression Types
The expression productions will have the following types
| Expression | Type
|--- |---
| byte | **byte**
| word | **word**
| label | **addr**
| reg | **reference**
| '#' EXPRESSION | **address**
| '(' EXPRESSION ')' | <i>type of inner expression</i>
| EXPRESSION '+' EXPRESSION | <i>See below</i>
| EXPRESSION '-' EXPRESSION | <i>See below</i>
| EXPRESSION '*' EXPRESSION | <i>See below</i>
| '[' EXPRESSION ']' | **addr**; <i>inner must be addr</i>
| 'lo' '(' EXPRESSION ')' | **byte**
| 'hi' '(' EXPRESSION ')' | **byte**
#### Operators
We have the following resulting types for operators
* '+'  

|        | byte | word  | addr
|---      |---   |---   |---
| byte    | byte | word | addr
| word    | word | word | addr
| addr    | addr | addr | 

* '-'

|        | byte | word  | addr
|---      |---   |---   |---
| byte    | byte |      |
| word    | word | word |
| addr    | addr | addr | byte

* '*'

|        | byte | word | addr
|---      |---   |---   |---
| byte    | word | word |
| word    | word | word |
| addr    |      |      | 
#### Functions
##### lo
Returns the lo byte of a value or address
##### hi
Returns the hi byte of a value or address
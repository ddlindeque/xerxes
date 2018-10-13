BASE $200

; Clear the screen
LDX 1000 ; 40 * 25
LDA ' '
@label1: STA $400+X
DEC X
BNE @label1

@label2: JMP @label2
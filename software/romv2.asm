; Page Zero usage
; 03: Index for writing buffer to memory
; 04-0B: Punch card reader driver jump table 
;   04-05: lo-hi of 'no instruction' (status = 0)   0xE820
;   06-07: lo-hi of 'data ready' (status = 1)       0xE821
;   08-09: lo-hi of 'addr lo' (status = 2)          0xE830
;   0A-0B: lo-hi of 'addr hi' (status = 3)          0xE840
;   0C-0D: lo-hi of 'run' (status = 4)              0xE850
; 0E-0F: Start address to write to or run program
; Page 1 = stack
; Page 2 = executable entry point
; Page E0 = Boot entry point
; Page E1 = Punchcard API
; Page E8 = General & Driver ISR
; Page FF = Vectors

%bootEntry = #$E000
%isr = #$E800

%pcrMemIndex = #$03
%pcrJumpTable = #$04
%pcrMemAddr = #$0E
%pcrControl = #$D02F
%pcrStatus = #$D030
%pcrRegister = #$D031
%pcrInitControl = $01
%pcrRunControl = $02
%pcrPageZeroAddr = %pcrMemIndex

; Boot entry point
BASE %bootEntry

; Set stack pointer to $FF
LDX $FF
TXS

; Clear the decimal flag
CLD

; Call punch card reader initialization
JSR @pcrInit
CLI
; Loop forever
@again: NOP
JMP @again

@pcrInit:
; Copy the page zero data to page zero
LDX @pageZeroDataEnd - @pageZeroDataBegin
@loop: LDA @pageZeroDataBegin+X
STA %pcrPageZeroAddr+X
DEC X
BNE @loop
LDA %pcrInitControl
STA %pcrControl
LDA %pcrRunControl
STA %pcrControl
RTS

@pcrNoInstr:
RTS

@pcrDataReady:
LDA %pcrRegister
LDY %pcrMemIndex
STA [%pcrMemAddr] + Y
INC %pcrMemIndex
LDA %pcrRunControl
STA %pcrControl
RTS

@pcrAddrLo:
LDA %pcrRegister
STA %pcrMemAddr
LDA %pcrRunControl
STA %pcrControl
RTS

@pcrAddrHi:
LDA %pcrRegister
STA %pcrMemAddr + $01
LDA $00
STA %pcrMemIndex
LDA %pcrRunControl
STA %pcrControl
RTS

@pcrRun:
CLI
JMP [%pcrMemAddr]

@pageZeroDataBegin:
DATA $00 ; Memory Index
; Jump vector
DATA @pcrNoInstr
DATA @pcrDataReady
DATA @pcrAddrLo
DATA @pcrAddrHi
DATA @pcrRun
@pageZeroDataEnd: DATA $00

BASE %isr
JSR @pcrISR
CLI
RTI

@pcrISR:
LDA %pcrStatus
ASL A
TAX
JMP [%pcrJumpTable + X]

BASE $FFFC
DATA %bootEntry
DATA %isr
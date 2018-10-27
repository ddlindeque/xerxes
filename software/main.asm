; Memory usage:
; Kernel is finished, so we can reuse all memory. We can use the following pages
; 0x00: page zero
;     0x03-0x04: monitor cursor
;     0xF0-0xFF: temporary var area
; 0x01: hardware stack
; 0x02-0x9F: program ram
; 0xC0: data
; 0xCF: software stack

; Calling Convention
; Using a 'software stack' using the X register as the stack pointer
; The stack will be at address 0xCF00 to 0xCFFF and is bottom-up
; Before we call a function, we push the parameters, (i.e. p1 and p2)
; PHX ; Preserve the value of X (current frame)
; LDA <p1>
; STA $CF00,X
; LDA <p2>
; STA $CF01,X
; JSR <sub>
; PLX ; (Restore current frame - this means we do not have to 'pop' all pushed)

; The called function
; Reserve space for the local variables (5 bytes)
; Access parameter p1
; LDA $CF00,X
; Access first local var
; LDA $CF02,X
; Call a function
; PHX
; LDA <p1>
; STA $CF07,X
; STX $07
; JSR <sub>
; PLX

; Main program
%base = #$0200
%stackbase = #$CF00

START %base ; This is where the program should start

BASE %base
; Initialize the system - we dump the whole kernel stack
LDX $FF
TXS
; Reset the software stack
LDX $00
; Clear the screen: monitor_clear():void
PHX 
JSR @monitor_clear
PLX
; Goto 2, 1: monitor_goto(x:byte,y:byte):void
PHX
LDA $02
STA (%stackbase)+X
LDA $01
STA (%stackbase+$01)+X
JSR @monitor_goto
PLX
; Print "Hello Xerxes": monitor_print(addr:word,len:byte):void
PHX
LDA lo(@data_hello)
STA (%stackbase)+X
LDA hi(@data_hello)
STA (%stackbase+$01)+X
LDA $0C ; length = 12 bytes
STA (%stackbase+$02)+X
JSR @monitor_print
PLX
; Loop forever
@label5: JMP @label5
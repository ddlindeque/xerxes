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

; Boot entry point
; Set stack pointer to $FF
0xE000 A2 ; LDX $A2
0xE001 FF
; TXS
0xE002 9A ; TXS
; Call punch card reader initialization
0xE003 20 ; JSR 0xE100
0xE004 00
0xE005 E1
; Loop until the program starts
0xE006 58 ; CLI
0xE007 EA ; NOP
0xE008 4C ; JMP 0xE007
0xE009 07
0xE00A E0

; void InitializePunchCardRead()
; {
    ; Initialize the driver vector
        ; 0x0004 20 ; E820
        ; 0x0005 E8
        ; 0x0006 21 ; E821
        ; 0x0007 E8
        ; 0x0008 30 ; E830
        ; 0x0009 E8
        ; 0x000A 40 ; E840
        ; 0x000B E8
        ; 0x000C 50 ; E850
        ; 0x000D E8
        0xE100 A9 ; LDA $20
        0xE101 20 ;
        0xE102 85 ; STA $04
        0xE103 04
        0xE104 A9 ; LDA $E8
        0xE105 E8 ;
        0xE106 85 ; STA $05
        0xE107 05
        0xE108 A9 ; LDA $21
        0xE109 21 ;
        0xE10A 85 ; STA $06
        0xE10B 06
        0xE10C A9 ; LDA $E8
        0xE10D E8 ;
        0xE10E 85 ; STA $07
        0xE10F 07
        0xE110 A9 ; LDA $30
        0xE111 30 ;
        0xE112 85 ; STA $08
        0xE113 08
        0xE114 A9 ; LDA $E8
        0xE115 E8 ;
        0xE116 85 ; STA $09
        0xE117 09
        0xE118 A9 ; LDA $40
        0xE119 40 ;
        0xE11A 85 ; STA $0A
        0xE11B 0A
        0xE11C A9 ; LDA $E8
        0xE11D E8 ;
        0xE11E 85 ; STA $0B
        0xE11F 0B
        0xE120 A9 ; LDA $50
        0xE121 50 ;
        0xE122 85 ; STA $0C
        0xE123 0C
        0xE124 A9 ; LDA $E8
        0xE125 E8 ;
        0xE126 85 ; STA $0D
        0xE127 0D
    ; Initialise the reader
        0xE128 A9 ; LDA $01
        0xE129 01
        ; Write A to 0xD02F
        0xE12A 8D ; STA 0xD02F
        0xE12B 2F
        0xE12C D0
    ; Start the reader
        0xE12D 20 ; JSR 0xE140
        0xE12E 40
        0xE12F E1
    ; Return
    0xE130 60 ; RTS
; }

; void StartReader()
; {
    ; Write 02 to A
    0xE140 A9 ; LDA $02
    0xE141 02
    ; Write A to 0xD02F
    0xE142 8D ; STA 0xD02F
    0xE143 2F
    0xE144 D0
    ; Return
    0xE145 60 ; RTS
; }

; General IRQ ISR - Poll the drivers
    ; Poll the punch card reader
    0xE800 20 ; JSR 0xE810
    0xE801 10
    0xE802 E8
    0xE803 58 ; CLI
    0xE804 40 ; RTI

; Punch card reader ISR
    ; Read status register, multiply by 2 and jump to driver function
    ; into X. This is the index into a jump table
    ; Load Reader Status into A
        0xE810 AD ; LDA 0xD030
        0xE811 30
        0xE812 D0
    ; Shift A left once (x2)
        ; A = A * 2 (ASL 1)
        0xE813 0A ; ASL A
    ; Lookup and jump
        ; X <- A
        0xE814 AA ; TAX
        ; Jump to (0x0004 + X)
        0xE815 7C ; JMP ($04,X)
        0xE816 04

    ; Do nothing
        0xE820 60 ; RTS

    ; Read next byte
        ; Read reader register into A
        0xE821 AD ; LDA $D031 
        0xE822 31
        0xE823 D0
        ; Y <- index (at $03)
        0xE824 A4 ; LDY $03
        0xE825 03
        ; Write to memory (pointed to by $0E + index (Y))
        0xE826 91 ; STA ($0E),Y
        0xE827 0E
        ; Inc $0003
        0xE828 E6 ; INC $03
        0xE829 03
        ; Start the reader
        0xE82A 20 ; JSR 0xE140
        0xE82B 40
        0xE82C E1
        0xE82D 60 ; RTS

    ; Load 'lo' of new address
        0xE830 AD ; LDA $D031
        0xE831 31
        0xE832 D0
        0xE833 85 ; STA $0E
        0xE834 0E
        ; Start the reader
        0xE835 20 ; JSR 0xE140
        0xE836 40
        0xE837 E1
        0xE838 60 ; RTS

    ; Load 'hi' of new address and reset index to 0
        0xE840 AD ; LDA $D031
        0xE841 31
        0xE842 D0
        0xE843 85 ; STA $0F
        0xE844 0F
        0xE845 A9 ; LDA #00
        0xE846 00
        0xE847 85 ; STA $03
        0xE848 03
        ; Start the reader
        0xE849 20 ; JSR 0xE140
        0xE84A 40
        0xE84B E1
        0xE84C 60 ; RTS

    ; "Run program" routine
        0xE850 58 ; CLI (Manually clear the interupt mask, since we're not returning)
        0xE851 6C ; JMP ($0E)
        0xE852 0E

; Vectors
    ; NMI -> ????
    0xFFFA ?? ; NMI LO
    0xFFFB ?? ; NMI HI
    ; RESET -> E000
    0xFFFC 00 ; RESET LO
    0xFFFD E0 ; RESET HI
    ; IRQ -> E800
    0xFFFE 00 ; IRQ LO
    0xFFFF E8 ; IRQ HI
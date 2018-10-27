%stackbase = #$CF00

%clear = $20
%screenAddress1 = #$0400
%screenAddress2 = #$04FA
%screenAddress3 = #$05F4
%screenAddress4 = #$06EE

%cursorAddress = #$03

@monitor_clear: ; void monitor_clear()
    ; Clear the screen
    ; 0-249
    LDY $FA
    LDA %clear
    STA %screenAddress1
    @monitor_clear_label1: STA %screenAddress1+Y
    DEC Y
    BNE @monitor_clear_label1
    ; 250-499
    LDX $FA
    @monitor_clear_label2: STA %screenAddress2+Y
    DEC Y
    BNE @monitor_clear_label2
    ; 500-749
    LDX $FA
    @monitor_clear_label3: STA %screenAddress3+Y
    DEC Y
    BNE @monitor_clear_label3
    ; 750-999
    LDX $F9
    @monitor_clear_label4: STA %screenAddress4+Y
    DEC Y
    BNE @monitor_clear_label4
    RTS

@monitor_goto: ; void monitor_goto(byte x, byte y)
    ; Set the cursor location to the start
    LDA $00
    STA %cursorAddress
    LDA $04
    STA %cursorAddress + $01

    ; For each y, increment the location by 40
    LDY (%stackbase+$01)+X ; Y = y
    BEQ @monitor_goto_x ; if Y = 0, goto x
    @monitor_goto_y_1:
    LDA %cursorAddress
    CLC
    ADC $28 ; $28 = 40 (columns)
    STA %cursorAddress
    BCC @monitor_goto_y_2 ; No carry - do not increment hi
    INC %cursorAddress+$01
    @monitor_goto_y_2: 
    DEC Y
    BNE @monitor_goto_y_1 ; Y != 0, inc again

    ; Now add x to the address
    @monitor_goto_x:
    LDA (%stackbase)+X ; A = x
    CLC
    ADC %cursorAddress ; A = x + %cursorAddress
    STA %cursorAddress
    BCC @monitor_goto_x_2 ; No carry - do not increment hi
    INC %cursorAddress+$01
    @monitor_goto_x_2:
    RTS

@monitor_print: ; void monitor_print(addr, len)
    ; Write the target address to the temporary page zero var area
    LDA (%stackbase)+X
    STA #$F0
    LDA (%stackbase+$01)+X
    STA #$F1
    ; Copy the data at addr to the current cursor
    LDY (%stackbase+$02)+X ; Y = len
    BEQ @monitor_print_done ; if Y = 0, goto end
    @monitor_print_again:
    DEC Y
    LDA [#$F0] + Y
    STA [%cursorAddress]+Y
    INC Y ; Test the value of Y
    DEC Y
    BNE @monitor_print_again
    @monitor_print_done:
    RTS
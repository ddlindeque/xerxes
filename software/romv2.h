template<typename TRom> void initialize_kernel_rom(TRom *rom)
{
    // ./software/romv2.asm
    // 1: ; Page Zero usage
    // 2: ; 03: Index for writing buffer to memory
    // 3: ; 04-0B: Punch card reader driver jump table 
    // 4: ;   04-05: lo-hi of 'no instruction' (status = 0)   0xE820
    // 5: ;   06-07: lo-hi of 'data ready' (status = 1)       0xE821
    // 6: ;   08-09: lo-hi of 'addr lo' (status = 2)          0xE830
    // 7: ;   0A-0B: lo-hi of 'addr hi' (status = 3)          0xE840
    // 8: ;   0C-0D: lo-hi of 'run' (status = 4)              0xE850
    // 9: ; 0E-0F: Start address to write to or run program
    // 10: ; Page 1 = stack
    // 11: ; Page 2 = executable entry point
    // 12: ; Page E0 = Boot entry point
    // 13: ; Page E1 = Punchcard API
    // 14: ; Page E8 = General & Driver ISR
    // 15: ; Page FF = Vectors
    // 16: 
    // 17: %bootEntry = #$E000
    // 18: %isr = #$E800
    // 19: 
    // 20: %pcrMemIndex = #$03
    // 21: %pcrJumpTable = #$04
    // 22: %pcrMemAddr = #$0E
    // 23: %pcrControl = #$D02F
    // 24: %pcrStatus = #$D030
    // 25: %pcrRegister = #$D031
    // 26: %pcrInitControl = $01
    // 27: %pcrRunControl = $02
    // 28: %pcrPageZeroAddr = %pcrMemIndex
    // 29: 
    // 30: ; Boot entry point
    // 31: BASE %bootEntry
    // 32: 
    // 33: ; Set stack pointer to $FF
    // 34: LDX $FF
    rom->program(0xE000,0xA2);
    rom->program(0xE001,0xFF);
    // 35: TXS
    rom->program(0xE002,0x9A);
    // 36: 
    // 37: ; Clear the decimal flag
    // 38: CLD
    rom->program(0xE003,0xD8);
    // 39: 
    // 40: ; Call punch card reader initialization
    // 41: JSR @pcrInit
    rom->program(0xE004,0x20);
    rom->program(0xE005,0x0C);
    rom->program(0xE006,0xE0);
    // 42: CLI
    rom->program(0xE007,0x58);
    // 43: ; Loop forever
    // 44: @again: NOP
    rom->program(0xE008,0xEA);
    // 45: JMP @again
    rom->program(0xE009,0x4C);
    rom->program(0xE00A,0x08);
    rom->program(0xE00B,0xE0);
    // 46: 
    // 47: @pcrInit:
    // 48: ; Copy the page zero data to page zero
    // 49: LDX @pageZeroDataEnd - @pageZeroDataBegin
    rom->program(0xE00C,0xA2);
    rom->program(0xE00D,0x0B);
    // 50: @loop: LDA @pageZeroDataBegin+X
    rom->program(0xE00E,0xBD);
    rom->program(0xE00F,0x4E);
    rom->program(0xE010,0xE0);
    // 51: STA %pcrPageZeroAddr+X
    rom->program(0xE011,0x95);
    rom->program(0xE012,0x03);
    // 52: DEC X
    rom->program(0xE013,0xCA);
    // 53: BNE @loop
    rom->program(0xE014,0xD0);
    rom->program(0xE015,0xF8);
    // 54: LDA %pcrInitControl
    rom->program(0xE016,0xA9);
    rom->program(0xE017,0x01);
    // 55: STA %pcrControl
    rom->program(0xE018,0x8D);
    rom->program(0xE019,0x2F);
    rom->program(0xE01A,0xD0);
    // 56: LDA %pcrRunControl
    rom->program(0xE01B,0xA9);
    rom->program(0xE01C,0x02);
    // 57: STA %pcrControl
    rom->program(0xE01D,0x8D);
    rom->program(0xE01E,0x2F);
    rom->program(0xE01F,0xD0);
    // 58: RTS
    rom->program(0xE020,0x60);
    // 59: 
    // 60: @pcrNoInstr:
    // 61: RTS
    rom->program(0xE021,0x60);
    // 62: 
    // 63: @pcrDataReady:
    // 64: LDA %pcrRegister
    rom->program(0xE022,0xAD);
    rom->program(0xE023,0x31);
    rom->program(0xE024,0xD0);
    // 65: LDY %pcrMemIndex
    rom->program(0xE025,0xA4);
    rom->program(0xE026,0x03);
    // 66: STA [%pcrMemAddr] + Y
    rom->program(0xE027,0x91);
    rom->program(0xE028,0x0E);
    // 67: INC %pcrMemIndex
    rom->program(0xE029,0xE6);
    rom->program(0xE02A,0x03);
    // 68: LDA %pcrRunControl
    rom->program(0xE02B,0xA9);
    rom->program(0xE02C,0x02);
    // 69: STA %pcrControl
    rom->program(0xE02D,0x8D);
    rom->program(0xE02E,0x2F);
    rom->program(0xE02F,0xD0);
    // 70: RTS
    rom->program(0xE030,0x60);
    // 71: 
    // 72: @pcrAddrLo:
    // 73: LDA %pcrRegister
    rom->program(0xE031,0xAD);
    rom->program(0xE032,0x31);
    rom->program(0xE033,0xD0);
    // 74: STA %pcrMemAddr
    rom->program(0xE034,0x85);
    rom->program(0xE035,0x0E);
    // 75: LDA %pcrRunControl
    rom->program(0xE036,0xA9);
    rom->program(0xE037,0x02);
    // 76: STA %pcrControl
    rom->program(0xE038,0x8D);
    rom->program(0xE039,0x2F);
    rom->program(0xE03A,0xD0);
    // 77: RTS
    rom->program(0xE03B,0x60);
    // 78: 
    // 79: @pcrAddrHi:
    // 80: LDA %pcrRegister
    rom->program(0xE03C,0xAD);
    rom->program(0xE03D,0x31);
    rom->program(0xE03E,0xD0);
    // 81: STA %pcrMemAddr + $01
    rom->program(0xE03F,0x85);
    rom->program(0xE040,0x0F);
    // 82: LDA $00
    rom->program(0xE041,0xA9);
    rom->program(0xE042,0x00);
    // 83: STA %pcrMemIndex
    rom->program(0xE043,0x85);
    rom->program(0xE044,0x03);
    // 84: LDA %pcrRunControl
    rom->program(0xE045,0xA9);
    rom->program(0xE046,0x02);
    // 85: STA %pcrControl
    rom->program(0xE047,0x8D);
    rom->program(0xE048,0x2F);
    rom->program(0xE049,0xD0);
    // 86: RTS
    rom->program(0xE04A,0x60);
    // 87: 
    // 88: @pcrRun:
    // 89: CLI
    rom->program(0xE04B,0x58);
    // 90: JMP [%pcrMemAddr]
    rom->program(0xE04C,0x6C);
    rom->program(0xE04D,0x0E);
    // 91: 
    // 92: @pageZeroDataBegin:
    // 93: DATA $00 ; Memory Index
    rom->program(0xE04E,0x00);
    // 94: ; Jump vector
    // 95: DATA @pcrNoInstr
    rom->program(0xE04F,0x21);
    rom->program(0xE050,0xE0);
    // 96: DATA @pcrDataReady
    rom->program(0xE051,0x22);
    rom->program(0xE052,0xE0);
    // 97: DATA @pcrAddrLo
    rom->program(0xE053,0x31);
    rom->program(0xE054,0xE0);
    // 98: DATA @pcrAddrHi
    rom->program(0xE055,0x3C);
    rom->program(0xE056,0xE0);
    // 99: DATA @pcrRun
    rom->program(0xE057,0x4B);
    rom->program(0xE058,0xE0);
    // 100: @pageZeroDataEnd: DATA $00
    rom->program(0xE059,0x00);
    // 101: 
    // 102: BASE %isr
    // 103: JSR @pcrISR
    rom->program(0xE800,0x20);
    rom->program(0xE801,0x05);
    rom->program(0xE802,0xE8);
    // 104: CLI
    rom->program(0xE803,0x58);
    // 105: RTI
    rom->program(0xE804,0x40);
    // 106: 
    // 107: @pcrISR:
    // 108: LDA %pcrStatus
    rom->program(0xE805,0xAD);
    rom->program(0xE806,0x30);
    rom->program(0xE807,0xD0);
    // 109: ASL A
    rom->program(0xE808,0x0A);
    // 110: TAX
    rom->program(0xE809,0xAA);
    // 111: JMP [%pcrJumpTable + X]
    rom->program(0xE80A,0x7C);
    rom->program(0xE80B,0x04);
    // 112: 
    // 113: BASE $FFFC
    // 114: DATA %bootEntry
    rom->program(0xFFFC,0x00);
    rom->program(0xFFFD,0xE0);
    // 115: DATA %isr
    rom->program(0xFFFE,0x00);
    rom->program(0xFFFF,0xE8);
}

#include "console.h"

#include <curses.h>
#include <ostream>
#include <sstream>
#include <iomanip>
#include <stdlib.h>

/*

The terminal window will be layed out as follows
  0         1         2         3         4         5         6         7         8
  0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
0 +--------------- MONITOR ----------------+ +--------- CPU ---------+ +------- PC -------+
1 |                                        | | PC = 0x5687 -> 0x5688 | | 3424:69 ADC REGX | 
2 |                                        | | Y  = 0x35   -> 0x35   | | 3435:40 'A'      |
3 |                                        | | X  = 0x78             | | 3436:40 'A'      |
4 |                                        | | S  = 0x75             | | 3437:60 'a'      |
5 |                                        | | A  = 0x34             | +------------------+
6 |                                        | | C  = 1                | +------ STACK -----+
7 |                                        | | Z  = 0                | | 3324:43 'a'      |
8 |                                        | | I  = 0                | |                  |
9 |                                        | | D  = 1                | |                  |
0 |                                        | | B  = 0                | |                  |
1 |                                        | | V  = 1                | |                  |
2 |                                        | | N  = 0                | |                  |
3 |                                        | + TICKS = 55456345      | |                  |
4 |                                        | +-----------------------+ +------------------+
5 |                                        | +--------- BUS ---------+ +------ WATCH -----+
6 |                                        | | 0x4543: 0x33 '3'      | | 3334: 12 'a'     |
7 |                                        | |                       | |                  |
8 |                                        | |                       | |                  |
9 |                                        | |                       | |                  |
0 |                                        | |                       | |                  |
1 |                                        | |                       | |                  |
2 |                                        | |                       | |                  |
3 |                                        | |                       | |                  |
4 |                                        | |                       | |                  |
5 |                                        | |                       | |                  |
6 +----------------------------------------+ +-----------------------+ +------------------+
7 > 
8------alerts-------------
9 | nmi | | irq | | reset |
0 input label: input value
*/

namespace dave
{

void console::initialize()
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    scrollok(stdscr, FALSE);
    start_color();

    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_WHITE, COLOR_RED);
    init_pair(3, COLOR_WHITE, COLOR_YELLOW);
    init_pair(4, COLOR_BLACK, COLOR_BLUE);
}

int console::getkey()
{
    return getch();
}

void console::teardown()
{
    endwin();
}

void console::reset_cursor()
{
    move(27, 2);
    refresh();
}

void console::draw_screen()
{
    // Draw the emulator
    move(0, 0);
    addch(ACS_ULCORNER);
    for(int i = 0; i < 40; i++) {
        addch(ACS_HLINE);
    }
    addch(ACS_URCORNER);
    for(int i = 0; i < 25; i++) {
        move(i + 1, 0);
        addch(ACS_VLINE);
        move(i + 1, 41);
        addch(ACS_VLINE);
    }
    move(26, 0);
    addch(ACS_LLCORNER);
    for(int i = 0; i < 40; i++) {
        addch(ACS_HLINE);
    }
    addch(ACS_LRCORNER);
    move(0, 16);
    addch(' ');
    addch('M');
    addch('O');
    addch('N');
    addch('I');
    addch('T');
    addch('O');
    addch('R');
    addch(' ');

    // Draw the CPU window
    move(0, 43);
    addch(ACS_ULCORNER);
    for(int i = 0; i < 23; i++) {
        addch(ACS_HLINE);
    }
    addch(ACS_URCORNER);
    for(int i = 0; i < 13; i++) {
        move(i + 1, 43);
        addch(ACS_VLINE);
        move(i + 1, 67);
        addch(ACS_VLINE);
    }
    move(14, 43);
    addch(ACS_LLCORNER);
    for(int i = 0; i < 23; i++) {
        addch(ACS_HLINE);
    }
    addch(ACS_LRCORNER);
    move(0, 53);
    addch(' ');
    addch('C');
    addch('P');
    addch('U');
    addch(' ');
    move(1,45); addch('P'); addch('C'); addch(' '); addch('=');
    move(2,45); addch('Y'); addch(' '); addch(' '); addch('=');
    move(3,45); addch('X'); addch(' '); addch(' '); addch('=');
    move(4,45); addch('S'); addch(' '); addch(' '); addch('=');
    move(5,45); addch('A'); addch(' '); addch(' '); addch('=');
    move(6,45); addch('C'); addch(' '); addch(' '); addch('=');
    move(7,45); addch('Z'); addch(' '); addch(' '); addch('=');
    move(8,45); addch('I'); addch(' '); addch(' '); addch('=');
    move(9,45); addch('D'); addch(' '); addch(' '); addch('=');
    move(10,45); addch('B'); addch(' '); addch(' '); addch('=');
    move(11,45); addch('V'); addch(' '); addch(' '); addch('=');
    move(12,45); addch('N'); addch(' '); addch(' '); addch('=');
    move(13,45); addch('T'); addch('I'); addch('C'); addch('K'); addch('S'); addch(' '); addch('=');

    // Draw the BUS window
    move(15, 43);
    addch(ACS_ULCORNER);
    for(int i = 0; i < 23; i++) {
        addch(ACS_HLINE);
    }
    addch(ACS_URCORNER);
    for(int i = 0; i < 11; i++) {
        move(i + 16, 43);
        addch(ACS_VLINE);
        move(i + 16, 67);
        addch(ACS_VLINE);
    }
    move(26, 43);
    addch(ACS_LLCORNER);
    for(int i = 0; i < 23; i++) {
        addch(ACS_HLINE);
    }
    addch(ACS_LRCORNER);
    move(15, 53);
    addch(' ');
    addch('B');
    addch('U');
    addch('S');
    addch(' ');

    // Draw the PC window
    move(0, 69);
    addch(ACS_ULCORNER);
    for(int i = 0; i < 18; i++) {
        addch(ACS_HLINE);
    }
    addch(ACS_URCORNER);
    for(int i = 0; i < 4; i++) {
        move(i + 1, 69);
        addch(ACS_VLINE);
        move(i + 1, 88);
        addch(ACS_VLINE);
    }
    move(5, 69);
    addch(ACS_LLCORNER);
    for(int i = 0; i < 18; i++) {
        addch(ACS_HLINE);
    }
    addch(ACS_LRCORNER);
    move(0, 77);
    addch(' ');
    addch('P');
    addch('C');
    addch(' ');

    // Draw the STACK window
    move(6, 69);
    addch(ACS_ULCORNER);
    for(int i = 0; i < 18; i++) {
        addch(ACS_HLINE);
    }
    addch(ACS_URCORNER);
    for(int i = 0; i < 7; i++) {
        move(i + 7, 69);
        addch(ACS_VLINE);
        move(i + 7, 88);
        addch(ACS_VLINE);
    }
    move(14, 69);
    addch(ACS_LLCORNER);
    for(int i = 0; i < 18; i++) {
        addch(ACS_HLINE);
    }
    addch(ACS_LRCORNER);
    move(6, 76);
    addch(' ');
    addch('S');
    addch('T');
    addch('A');
    addch('C');
    addch('K');
    addch(' ');

    // Draw the WATCH window
    move(15, 69);
    addch(ACS_ULCORNER);
    for(int i = 0; i < 18; i++) {
        addch(ACS_HLINE);
    }
    addch(ACS_URCORNER);
    for(int i = 0; i < 10; i++) {
        move(i + 16, 69);
        addch(ACS_VLINE);
        move(i + 16, 88);
        addch(ACS_VLINE);
    }
    move(26, 69);
    addch(ACS_LLCORNER);
    for(int i = 0; i < 18; i++) {
        addch(ACS_HLINE);
    }
    addch(ACS_LRCORNER);
    move(15, 76);
    addch(' ');
    addch('W');
    addch('A');
    addch('T');
    addch('C');
    addch('H');
    addch(' ');

    move(27, 0);
    addch('>');
    reset_cursor();
}

template<typename T> struct register_writer {};
template<> struct register_writer<bool> {
    inline void operator()(std::ostream &stm, const bool &value) const {
        stm << (value ? "T" : "F") << "   ";
    }
};
template<> struct register_writer<uint8_t> {
    inline void operator()(std::ostream &stm, const uint8_t &value) const {
        stm << std::setfill('0') << std::hex << std::setw(2) << (uint)value << "  ";
    }
};
template<> struct register_writer<uint16_t> {
    inline void operator()(std::ostream &stm, const uint16_t &value) const {
        stm << std::setfill('0') << std::hex << std::setw(4) << (uint)value;
    }
};

int get_register_line(const std::string &name)
{
    if (name == "PC") return 1;
    else if (name == "Y") return 2;
    else if (name == "X") return 3;
    else if (name == "S") return 4;
    else if (name == "A") return 5;
    else if (name == "C") return 6;
    else if (name == "Z") return 7;
    else if (name == "I") return 8;
    else if (name == "D") return 9;
    else if (name == "B") return 10;
    else if (name == "V") return 11;
    else if (name == "N") return 12;
    else return 0;
}

template<typename T> void g_update_cpu_register(const std::string &name, const T &value)
{
    int line = get_register_line(name);
    if (line == 0) return;
    static uint16_t prev_values[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    move(line, 50);
    std::stringstream stm;
    register_writer<T>()(stm, (T)prev_values[line]);
    if (prev_values[line] != (uint16_t)value) {
        stm << " -> ";
        register_writer<T>()(stm, value);
        prev_values[line] = (uint16_t)value;
        for(auto &ch : stm.str()) {
            addch(ch | COLOR_PAIR(1));
        }
    }
    else {
        stm << "         ";
        for(auto &ch : stm.str()) {
            addch(ch);
        }
    }
    
    console::reset_cursor();
}

void console::update_cpu_register(const std::string &name, const bool &value)
{
    g_update_cpu_register<bool>(name, value);
}

void console::update_cpu_register(const std::string &name, const uint8_t &value)
{
    g_update_cpu_register<uint8_t>(name, value);
}

void console::update_cpu_register(const std::string &name, const uint16_t &value)
{
    g_update_cpu_register<uint16_t>(name, value);
}

void console::update_char_on_virtual_monitor(const REG16 &addr, const REG8 &data)
{
    // Calculate the coordinate
    // addr = 0: 0,0 (transpose to 1,1)
    // addr = 39: 0,39 (transpose to 1,40)
    // addr = 40: 1,0 (transpose to 2,1) 
    REG16 y = (addr / 40) + 1;
    REG16 x = (addr % 40) + 1;
    move(y, x);

    static char random_chars[] = { '!', '"', '%', '^', '&', '*', '(', ')', '+', '{', '}', '[', ']', '@', ':', ';', '~', '#', '<', '>', ',', '.' };

    // Output the character at this memory address
    auto ch = data & 0x7F;
    int c = 7;
    if (!isprint(ch)) {
        ch = random_chars[rand() % sizeof(random_chars)];
    }
    addch(ch);
}

void console::show_operation(const std::string &op)
{
    move(27, 2);
    for(auto &ch : op) {
        addch(ch);
    }
    for(int i = 0; i < 50; i++) {
        addch(' ');
    }
    clear_alert();
}

void console::update_ticks(const size_t ticks)
{
    std::stringstream stm;
    stm << ticks;
    move(13, 53);
    for(auto &ch : stm.str()) {
        addch(ch);
    }
    reset_cursor();
}

void console::alert(const std::string &msg)
{
    move(28, 0);
    addch(' ' | COLOR_PAIR(2));
    addch('-' | COLOR_PAIR(2));
    addch('=' | COLOR_PAIR(2));
    addch(' ' | COLOR_PAIR(2));
    for(auto &ch : msg) {
        addch(ch | COLOR_PAIR(2));
    }
    addch(' ' | COLOR_PAIR(2));
    addch('=' | COLOR_PAIR(2));
    addch('-' | COLOR_PAIR(2));
    addch(' ' | COLOR_PAIR(2));
    reset_cursor();
}

void console::clear_alert()
{
    move(28,0);
    for(int i = 0; i < 80; i++) {
        addch(' ');
    }
    reset_cursor();
}

int bus_line = 0;

void console::clear_bus()
{
    bus_line = 0;
    for(int line = 16; line < 26; line++) {
        move(line, 44);
        for(int col = 44; col < 67; col++) {
            addch(' ');
        }
    }
}

void write_data_addr(const REG16 &addr, const REG8 &data, const int &color = 0)
{
    std::stringstream stm;
    stm << std::setfill('0') << std::hex << std::setw(4) << (uint)addr;
    stm << ':';
    stm << std::setfill('0') << std::hex << std::setw(2) << (uint)data;
    if (isprint((int)data)) {
        stm << " '" << (char)data << '\'';
    }
    for(auto &ch : stm.str()) {
        addch(ch | color);
    }
}

void console::add_bus(const REG16 &addr, const REG8 *data)
{
    if (bus_line == 10) return;
    move(bus_line + 16, 45);
    write_data_addr(addr, *data);
    bus_line++;
}

void console::report_pc(system_bus *bus, const REG16 &addr)
{
    REG8 op = 0, s1 = 0, s2 = 0, s3 = 0;
    bus->read(addr, &op);
    bus->read(addr+1, &s1);
    bus->read(addr+2, &s2);
    bus->read(addr+3, &s3);

    std::stringstream stm;
    stm << std::setfill('0') << std::hex << std::setw(4) << (uint)addr;
    stm << ':';
    stm << std::setfill('0') << std::hex << std::setw(2) << (uint)op;
    stm << ' ';
    switch(op) {
        case 0x00: stm << "BRK"; break;
        case 0x69: stm << "ADC IMM"; break;
        case 0x6D: stm << "ADC ABS"; break;
        case 0x65: stm << "ADC ZPG"; break;
        case 0x61: stm << "ADC INDX"; break;
        case 0x71: stm << "ADC INDY"; break;
        case 0x75: stm << "ADC ZPGX"; break;
        case 0x7D: stm << "ADC ABSX"; break;
        case 0x79: stm << "ADC ABSY"; break;
        case 0x72: stm << "ADC IND"; break;
        case 0x29: stm << "AND IMM"; break;
        case 0x2D: stm << "AND ABS"; break;
        case 0x25: stm << "AND ZPG"; break;
        case 0x21: stm << "AND INDX"; break;
        case 0x31: stm << "AND INDY"; break;
        case 0x35: stm << "AND ZPGX"; break;
        case 0x3D: stm << "AND ABSX"; break;
        case 0x39: stm << "AND ABSY"; break;
        case 0x32: stm << "AND IND"; break;
        case 0x0E: stm << "ASL ABS"; break;
        case 0x06: stm << "ASL ZPG"; break;
        case 0x0A: stm << "ASL ACC"; break;
        case 0x16: stm << "ASL ZPGX"; break;
        case 0x1E: stm << "ASL ABSX"; break;
        case 0x90: stm << "BCC"; break;
        case 0xB0: stm << "BCS"; break;
        case 0xF0: stm << "BEQ"; break;
        case 0x30: stm << "BMI"; break;
        case 0xD0: stm << "BNE"; break;
        case 0x10: stm << "BPL"; break;
        case 0x80: stm << "BRA"; break;
        case 0x50: stm << "BVC"; break;
        case 0x70: stm << "BVS"; break;
        case 0x89: stm << "BIT IMM"; break;
        case 0x2C: stm << "BIT ABS"; break;
        case 0x24: stm << "BIT ZPG"; break;
        case 0x34: stm << "BIT ZPGX"; break;
        case 0x3C: stm << "BIT ABSX"; break;
        case 0x18: stm << "CLC"; break;
        case 0xD8: stm << "CLD"; break;
        case 0x58: stm << "CLI"; break;
        case 0xB8: stm << "CLV"; break;
        case 0x38: stm << "SEC"; break;
        case 0xF8: stm << "SED"; break;
        case 0x78: stm << "SEI"; break;
        case 0xC9: stm << "CMP IMM"; break;
        case 0xCD: stm << "CMP ABS"; break;
        case 0xC5: stm << "CMP ZPG"; break;
        case 0xC1: stm << "CMP INDX"; break;
        case 0xD1: stm << "CMP INDY"; break;
        case 0xD5: stm << "CMP ZPGX"; break;
        case 0xDD: stm << "CMP ABSX"; break;
        case 0xD9: stm << "CMP ABSY"; break;
        case 0xD2: stm << "CMP IND"; break;
        case 0xE0: stm << "CPX IMM"; break;
        case 0xEC: stm << "CPX ABS"; break;
        case 0xE4: stm << "CPX ZPG"; break;
        case 0xC0: stm << "CPY IMM"; break;
        case 0xCC: stm << "CPY ABS"; break;
        case 0xC4: stm << "CPY ZPG"; break;
        case 0xCE: stm << "DEC ABS"; break;
        case 0xC6: stm << "DEC ZPG"; break;
        case 0x3A: stm << "DEC ACC"; break;
        case 0xD6: stm << "DEC ZPGX"; break;
        case 0xDE: stm << "DEC ABSX"; break;
        case 0xCA: stm << "DEC REGX"; break;
        case 0x88: stm << "DEC REGY"; break;
        case 0xEE: stm << "INC ABS"; break;
        case 0xE6: stm << "INC ZPG"; break;
        case 0x1A: stm << "INC ACC"; break;
        case 0xF6: stm << "INC ZPGX"; break;
        case 0xFE: stm << "INC ABSX"; break;
        case 0xE8: stm << "INC REGX"; break;
        case 0xC8: stm << "INC REGY"; break;
        case 0x49: stm << "XOR IMM"; break;
        case 0x4D: stm << "XOR ABS"; break;
        case 0x45: stm << "XOR ZPG"; break;
        case 0x41: stm << "XOR INDX"; break;
        case 0x51: stm << "XOR INXY"; break;
        case 0x55: stm << "XOR ZPGX"; break;
        case 0x5D: stm << "XOR ABSX"; break;
        case 0x59: stm << "XOR ABSY"; break;
        case 0x52: stm << "XOR IND"; break;
        case 0x4C: stm << "JMP ABS"; break;
        case 0x7C: stm << "JMP INDX"; break;
        case 0x6C: stm << "JMP IND"; break;
        case 0x20: stm << "JSR ABS"; break;
        case 0xA9: stm << "LDA IMM"; break;
        case 0xAD: stm << "LDA ABS"; break;
        case 0xA5: stm << "LDA ZPG"; break;
        case 0xA1: stm << "LDA INDX"; break;
        case 0xB1: stm << "LDA INDY"; break;
        case 0xB5: stm << "LDA ZPGX"; break;
        case 0xBD: stm << "LDA ABSX"; break;
        case 0xB9: stm << "LDA ABSY"; break;
        case 0xB2: stm << "LDA IND"; break;
        case 0xA2: stm << "LDX IMM"; break;
        case 0xAE: stm << "LDX ABS"; break;
        case 0xA6: stm << "LDX ZPG"; break;
        case 0xBE: stm << "LDX ABSY"; break;
        case 0xB6: stm << "LDX ZPGY"; break;
        case 0xA0: stm << "LDY IMM"; break;
        case 0xAC: stm << "LDY ABS"; break;
        case 0xA4: stm << "LDY ZPG"; break;
        case 0xB4: stm << "LDY ZPGX"; break;
        case 0xBC: stm << "LDY ABSX"; break;
        case 0x4E: stm << "LSR ABS"; break;
        case 0x46: stm << "LSR ZPG"; break;
        case 0x4A: stm << "LSR ACC"; break;
        case 0x56: stm << "LSR ZPGX"; break;
        case 0x5E: stm << "LSR ABSX"; break;
        case 0xEA: stm << "NOP"; break;
        case 0x09: stm << "OR IMM"; break;
        case 0x0D: stm << "OR ABS"; break;
        case 0x05: stm << "OR ZPG"; break;
        case 0x01: stm << "OR INDX"; break;
        case 0x11: stm << "OR INDY"; break;
        case 0x15: stm << "OR ZPGX"; break;
        case 0x1D: stm << "OR ABSX"; break;
        case 0x19: stm << "OR ABSY"; break;
        case 0x12: stm << "OR IND"; break;
        case 0x48: stm << "PUSH A"; break;
        case 0x08: stm << "PUSH P"; break;
        case 0xDA: stm << "PUSH X"; break;
        case 0x68: stm << "PULL A"; break;
        case 0x28: stm << "PULL P"; break;
        case 0xFA: stm << "PULL X"; break;
        case 0x7A: stm << "PULL Y"; break;
        default:
            stm << "???";
            break;
    }

    move(1, 71);
    for(auto &ch : stm.str()) {
        addch(ch);
    }

    move(2, 71);
    write_data_addr(addr+1, s1);
    move(3, 71);
    write_data_addr(addr+2, s2);
    move(4, 71);
    write_data_addr(addr+3, s3);
}

void console::report_s(system_bus *bus, const REG8 &value)
{
    // addr is the top of the stack. The stack goes increasing values down
    REG16 addr = 0x0100 | value;
    int line = 7;
    for(int i = 0; i < 7; i++) {
        REG8 data = 0;
        bus->read(addr, &data);
        move(line, 71);
        write_data_addr(addr, data);
        addr++;
        line++;
    }
}

void write_button(const std::string &text, bool value)
{
    auto c = value ? COLOR_PAIR(3) : COLOR_PAIR(4);
    addch(' ' | c);
    for(auto &ch : text) {
        addch(ch | c);
    }
    addch(' '| c);
    console::reset_cursor();
}

void console::update_nmi_line(bool value)
{
    move(29, 0);
    write_button("nmi", value);
}

void console::update_irq_line(bool value)
{
    move(29, 8);
    write_button("irq", value);
}

void console::update_reset_line(bool value)
{
    move(29, 16);
    write_button("reset", value);
}

REG16 console::get_addr_from_input(const std::string &label)
{
    // Require user input
    move(30, 0);
    int col = 0;
    for(auto &ch : label) {
        addch(ch);
        col++;
    }
    addch(':');
    col++;
    addch(' ');
    col++;
    char buf[100];
    buf[0] = 0;
    size_t index = 0;
    while(true) {
        int pos = col;
        char *p = buf;
        move(30, pos);
        while(*p != '\x0') {
            addch(*p);
            p++;
            pos++;
        }
        addch(' ');
        move(30,pos);
        
        auto key = console::getkey();
        switch(key) {
        case 127: // backspace
            if (index != 0) {
                index--;
                buf[index] = 0;
            }
            break;
        case 10: // enter
            move(30,0);
            for(int i = 0; i < 120; i++) {
                addch(' ');
            }
            return strtol(buf, NULL, 16);
        default:
            buf[index] = (char)key;
            index++;
            buf[index] = '\x0';
            break;
        }
    }
}

int next_watch_line = 16;

void console::clear_watches()
{
    for(int line = 16; line < 26; line++) {
        move(line, 70);
        for (int col = 70; col < 88; col++) {
            addch(' ');
        }
    }
    next_watch_line = 16;
}

void console::add_watch(const REG16 &addr, const REG8 &cur_value, bool value_changed)
{
    if (next_watch_line == 25) return;
    move(next_watch_line, 71);
    next_watch_line++;
    write_data_addr(addr, cur_value, value_changed ? COLOR_PAIR(1) : 0);
}

}
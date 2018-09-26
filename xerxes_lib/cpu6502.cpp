#include "stdafx.h"
#include "cpu6502.h"

#include "common.h"
#include "rom.h"

/*
Important addresses

Page 0: Used largely for VMT's
Page 1: Used solely as the stack (FF is the first pushed entry)
Page FF: FE(low) & FF (high) is IRQ address, FC (low) FD (high) is the Reset address, FA (low) and FB (high) is Non-maskable address


Interupt
  - push pc(high)
  - push pc(low)
  - push p (status)

Looks like memory is mapped as follows:

0x0000:  (0 = input, 1 = output)
    0: Direction of I/O for bit 0
    1: Direction of I/O for bit 1
    2: Direction of I/O for bit 2
    3: Direction of I/O for bit 3
    4: Direction of I/O for bit 4
    5: Direction of I/O for bit 5
    6: Direction of I/O for bit 6 (Not used)
    7: Direction of I/O for bit 7 (Not used)
0x0001: I/O port DR
    0: Select ROM/RAM at 0xA000 (0 = RAM, 1 = ROM)
    1: Select ROM/RAM at 0xE000 (0 = RAM, 1 = ROM)
    2: Select I/O or CHAR ROM at 0xD000 (0 = CHAR ROM, 1 = I/O)
    3: Casette data output line
    4: Casette switch sense (0 = button pressed, 1 = not pressed)
    5: Casette motor control (0 = motor off, 1 = motor on)
    6: Not used
    7: Not used
0x0002: Unused
0x0003-0x00FF: Used by BASIC (addresses of different functions)
0x0100-0x01FF: Stack
0x0200-0x9FFF: RAM
    0x0200-0x03FF: Basic & Kernal working area
    0x0400-0x07E7: Screen buffer (25 lines, 40 cols)
    0x07E8-0x07F7: ???
    0x07F8-0x07FF: Sprite shape data pointers
    0x0800-0x9FFF: Basic Program Text
0xA000-0xBFFF: Basic ROM
0xC000-0xCFFF: RAM
0xD000-0xDFFF: I/O or CHAR ROM
    for I/O
        0xD000-0xD02E: VIC Chip Registers
            0xD014: (0: ??, 1: ??, 2: ??, 3: ??, 4: mode, ...)
            0xD018: Video Buffer Address (Base Address Nybble) & dot data
            0xD020: Border colour
            0xD021: Background colour
            0xD022: Alternative background colour 1
            0xD023: Alternative background colour 2
        0xD02F-0xD03F: Unused
        0xD040-0xD3FF: Register Images
        0xD400-0xD41C: Sound Interface Device
        0xD41D-0xD41F: Unused
        0xD420-0xD7FF: SID Register Images
        0xD800-0xDBFF: Colour RAM (0: Black, 1: White, Red, Cyan, Purple, Green, Blue, Yellow, Orange, Brown, Light red, Dark gray, Medium gray, Light green, Light blue, Light gray)
        0xDC00-0xDC0F: CIA
            0xDC00-0xDC01: ports A & B
    for CHAR ROM
        Bitmaps of all characters
0xE000-0xFFFF: Kernal ROM
    0xE000: Boot entry point
    0xFF81-0xFFF5: Jump Table
    0xFFFA-0xFFFB: Non-maskabale Interupt Vector
    0xFFFC-0xFFFD: Reset vector (0xFCE2)
    0xFFFE-0xFFFF: Maskable Interupt Vector

Page 0x00: (Page 0)
    0x00: I/O port DDR
    0x01: I/O port DR
    0x02: Unused
    0x03-0xFF: Used by BASIC
Page 0x01 (Page 1): Stack
Page 0x02-0x9F (Page 2-159): RAM
    Page 0x02-0x03 (Page 2 & 3): Basic & Kernel working area
    Page 0x04-0x07 (Pages 4-7): Screen buffer
    Page 0x08-0x9F (Pages 8-159): Basic Program Text
Page 0xA0-0xBF (Page 160-191): Basic ROM
Page 0xC0-0xCF (Page 192-207): RAM
Page 0xD0-0xDF (Page 208-223): I/O or CHAR ROM
Page 0xE0-0xFF (Page 224-255): Kernal ROM

*/

namespace dave
{
    struct UNPACK {
        REG8 hi;
        REG8 lo;
    };

    // The next byte is the value
    struct imm {
        static inline auto get_addr(system_bus *bus, cpu6502::registers &regs, int &cycles) -> REG16 {
            return regs.PC++;
        }
    };
    // The next two bytes is the address of the value
    struct abs {
        static inline auto get_addr(system_bus *bus, cpu6502::registers &regs, int &cycles) -> REG16 {
            REG8 lo, hi;
            bus->read(regs.PC, &lo);
            regs.PC++;
            bus->read(regs.PC, &hi);
            regs.PC++;
            return ((REG16)hi << 8) | (REG16)lo;
        }
    };
    // The next two bytes is the address with offset x of the value
    struct abs_x {
        static inline auto get_addr(system_bus *bus, cpu6502::registers &regs, int &cycles) -> REG16 {
            REG8 lo, hi;
            bus->read(regs.PC, &lo);
            regs.PC++;
            bus->read(regs.PC, &hi);
            regs.PC++;
            REG16 addr = ((REG16)hi << 8) | (REG16)lo;
            auto a = addr & 0xFF00;
            addr += regs.X;
            if (a != (addr & 0xFF00)) {
                cycles++;
            }
            return addr;
        }
    };
    // The next two bytes is the address with offset y of the value
    struct abs_y {
        static inline auto get_addr(system_bus *bus, cpu6502::registers &regs, int &cycles) -> REG16 {
            REG8 lo, hi;
            bus->read(regs.PC, &lo);
            regs.PC++;
            bus->read(regs.PC, &hi);
            regs.PC++;
            REG16 addr = ((REG16)hi << 8) | (REG16)lo;
            auto a = addr & 0xFF00;
            addr += regs.Y;
            if (a != (addr & 0xFF00)) {
                cycles++;
            }
            return addr;
        }
    };
    // The next byte is the address in page zero of the value
    struct zpg {
        static inline auto get_addr(system_bus *bus, cpu6502::registers &regs, int &cycles) -> REG16 {
            REG8 lo;
            bus->read(regs.PC, &lo);
            regs.PC++;
            return (REG16)lo;
        }
    };
    // The next byte is the address in page zero with offset x of the value
    struct zpg_x {
        static inline auto get_addr(system_bus *bus, cpu6502::registers &regs, int &cycles) -> REG16 {
            REG8 lo;
            bus->read(regs.PC, &lo);
            regs.PC++;
            lo += regs.X;
            return (REG16)lo;
        }
    };
    // The next byte is the address in page zero with offset y of the value
    struct zpg_y {
        static inline auto get_addr(system_bus *bus, cpu6502::registers &regs, int &cycles) -> REG16 {
            REG8 lo;
            bus->read(regs.PC, &lo);
            regs.PC++;
            lo += regs.Y;
            return (REG16)lo;
        }
    };
    // The next byte is the address in page zero of the address of the value
    struct ind {
        static inline auto get_addr(system_bus *bus, cpu6502::registers &regs, int &cycles) -> REG16 {
            REG8 v, lo, hi;
            bus->read(regs.PC, &v);
            regs.PC++;
            bus->read((REG16)v, &lo);
            v++;
            bus->read((REG16)v, &hi);
            return ((REG16)hi << 8) | (REG16)lo;
        }
    };
    // The next byte is the address in page zero of the address with offset X (no carry) of the value
    struct ind_x {
        static inline auto get_addr(system_bus *bus, cpu6502::registers &regs, int &cycles) -> REG16 {
            REG8 v, lo, hi;
            bus->read(regs.PC, &v);
            regs.PC++;
            v += regs.X;
            bus->read((REG16)v, &lo);
            v++;
            bus->read((REG16)v, &hi);
            return ((REG16)hi << 8) | (REG16)lo;
        }
    };
    // The next byte is the address in page zero of the address with offset Y (carry) of the value 
    struct ind_y {
        static inline auto get_addr(system_bus *bus, cpu6502::registers &regs, int &cycles) -> REG16 {
            REG8 v, lo, hi;
            bus->read(regs.PC, &v);
            regs.PC++;
            bus->read((REG16)v, &lo);
            v++;
            bus->read((REG16)v, &hi);
            REG16 addr = ((REG16)hi << 8) | (REG16)lo;
            auto a = addr & 0xFF00;
            addr += regs.Y;
            if (a != (addr & 0xFF00)) {
                cycles++;
            }
            return addr;
        }
    };
    // The accumulator is used as the memory address
    struct acc {
    };
    // The X register
    struct reg_x {};
    // The Y register
    struct reg_y {};

    struct branch {
        template<typename _Pred> inline auto operator()(system_bus *bus, cpu6502::registers &regs, const _Pred &pred, int &cycles) const -> void {
            REG8 ofs;
            bus->read(regs.PC, &ofs);
            regs.PC++;
            if (pred(regs)) {
                // We consume one cycle for the branch
                cycles++;
                // If we're branching to another page, then we consume another cycle
                auto pc = regs.PC & 0xFF00;
                regs.PC += (SIGNED_REG8)ofs;
                if (pc != (regs.PC & 0xFF00)) {
                    cycles++;
                }
            }
        }
    };
    template<typename _AM> struct cmp_v {
        inline auto operator()(system_bus *bus, cpu6502::registers &regs, REG8 &v, int &cycles) const -> void {
            REG8 m;
            bus->read(_AM::get_addr(bus, regs, cycles), &m);
            regs.P.Z = (m == v ? 1 : 0);
            regs.P.C = (v < m ? 0 : 1);
            // v - m
            auto az = (v & 0x80) != 0;
            if (az ^ ((m & 0x80) != 0)) {
                regs.P.N = az;
            }
            else {
                regs.P.N = ((SIGNED_REG8)v) < ((SIGNED_REG8)m) ? 1 : 0;
            }
        }
    };

    inline auto is_neg(const REG8 &value) -> REG8 {
        return (value & 0x80) == 0 ? 0 : 1;
    }

    struct _incdec {
        template<typename _Action> inline auto operator()(cpu6502::registers &regs, REG8 &value, const _Action &action) -> void {
            action(value);
            regs.P.Z = value == 0 ? 1 : 0;
            regs.P.N = is_neg(value);
        }
    };
    struct inc {
        inline auto operator()(cpu6502::registers &regs, REG8 &value) -> void {
            _incdec()(regs, value, [](REG8 &value) { ++value; });
        }
    };
    struct dec {
        inline auto operator()(cpu6502::registers &regs, REG8 &value) -> void {
            _incdec()(regs, value, [](REG8 &value) { --value; });
        }
    };
    template<typename _Op, typename _AM> struct incdec {
        inline auto operator()(system_bus *bus, cpu6502::registers &regs, int &cycles) -> void {
            auto addr = _AM::get_addr(bus, regs, cycles);
            REG8 m;
            bus->read(addr, &m);
            _Op()(regs, m);
            bus->write(addr, &m);
        }
    };
    template<typename _Op> struct incdec<_Op, acc> {
        inline auto operator()(system_bus *bus, cpu6502::registers &regs) -> void {
            _Op()(regs, regs.A);
        }
    };
    template<typename _Op> struct incdec<_Op, reg_x> {
        inline auto operator()(system_bus *bus, cpu6502::registers &regs) -> void {
            _Op()(regs, regs.X);
        }
    };
    template<typename _Op> struct incdec<_Op, reg_y> {
        inline auto operator()(system_bus *bus, cpu6502::registers &regs) -> void {
            _Op()(regs, regs.Y);
        }
    };
    template<typename _Op, typename _AM> struct logic {
        inline auto operator()(system_bus *bus, cpu6502::registers &regs, int &cycles) const -> void {
            REG8 m;
            bus->read(_AM::get_addr(bus, regs, cycles), &m);
            regs.A = _Op()(regs.A, m);
            regs.P.N = is_neg(regs.A);
            regs.P.Z = regs.A == 0 ? 1 : 0;
        }
    };
    struct and {
        inline auto operator()(const REG8 &v1, const REG8 &v2) -> REG8 {
            return v1 & v2;
        }
    };
    struct or {
        inline auto operator()(const REG8 &v1, const REG8 &v2) -> REG8 {
            return v1 | v2;
        }
    };
    struct eor {
        inline auto operator()(const REG8 &v1, const REG8 &v2) -> REG8 {
            return v1 ^ v2;
        }
    };

    inline auto stack_push(system_bus *bus, cpu6502::registers &regs, const REG8 &value) -> void {
        // write the byte and decrement the stack register
        bus->write(0x0100 | regs.S, &value);
        regs.S--;
    }

    inline auto stack_pull(system_bus *bus, cpu6502::registers &regs) -> REG8 {
        // increment the stack register and read the byte
        regs.S++;
        REG8 value;
        bus->read(0x0100 | regs.S, &value);
        regs.P.N = is_neg(value);
        regs.P.Z = value == 0;
        return value;
    }

    struct brk {
        inline auto operator()(system_bus *bus, cpu6502::registers &regs) const -> void {
            regs.PC++;
            UNPACK *p = (UNPACK*)&regs.PC;
            stack_push(bus, regs, p->hi);
            stack_push(bus, regs, p->lo);
            stack_push(bus, regs, *((REG8*)&regs.P));
            UNPACK *upc = (UNPACK*)&regs.PC;
            bus->read(0xFFFE, &upc->lo);
            bus->read(0xFFFF, &upc->hi);
            regs.P.I = 1;
            regs.P.B = 1;
            regs.P.D = 0;
        }
    };
    template<typename _AM> struct adc {
        inline auto operator()(system_bus *bus, cpu6502::registers &regs, int &cycles) const -> void {
            if (regs.P.D != 0) {
                cycles++;
            }
            REG8 m;
            bus->read(_AM::get_addr(bus, regs, cycles), &m);
            REG16 res;
            if (regs.P.D == 0) {
                res = regs.A + 0x06;
                auto t2 = res ^ m ^ regs.P.C;                   // sum without carry propagation
                res += m + regs.P.C;                   // provisional sum
                t2 = res ^ t2;                  // all the binary carry bits
                t2 = ~t2 & 0x10;         // just the BCD carry bits
                if (t2 != 0) {
                    res -= 0x06; // corrected BCD sum
                }
            }
            else {
                res = regs.A + m + regs.P.C;
            }

            regs.P.V = ((regs.A ^ res) & (m ^ res) & 0x80) == 0 ? 0 : 1; // overflow when the sign bit was the same for A & M, and it's changed value.
            regs.P.C = (res & 0x0100) == 0 ? 0 : 1;
            res &= 0xFF;
            regs.P.Z = res == 0 ? 1 : 0;
            regs.P.N = is_neg((REG8)res);
            regs.A = (REG8)res;
        }
    };
    template<typename _AM> struct asl {
        inline auto operator()(system_bus *bus, cpu6502::registers &regs, int &cycles) const -> void {
            auto addr = _AM::get_addr(bus, regs, cycles);
            REG8 m;
            bus->read(addr, &m);
            REG16 res = ((REG16)m) << 1;
            regs.P.C = (res & 0x0100) != 0;
            m = res & 0xFF;
            regs.P.N = is_neg(m);
            regs.P.Z = m == 0 ? 1 : 0;
            bus->write(addr, &m);
        }
    };
    template<> struct asl<acc> {
        inline auto operator()(system_bus *bus, cpu6502::registers &regs) const -> void {
            REG16 res = (REG16)regs.A << 1;
            regs.P.C = (res & 0x0100) != 0;
            res &= 0xFF;
            regs.P.N = is_neg((REG8)res);
            regs.P.Z = (res) == 0 ? 1 : 0;
            regs.A = (REG8)res;
        }
    };
    template<typename _AM> struct lsr {
        inline auto operator()(system_bus *bus, cpu6502::registers &regs, int &cycles) const -> void {
            auto addr = _AM::get_addr(bus, regs, cycles);
            REG8 m;
            bus->read(addr, &m);
            regs.P.C = m & 0x01;
            REG16 res = ((REG16)m) >> 1;
            m = res & 0xFF;
            regs.P.N = 0;
            regs.P.Z = m == 0 ? 1 : 0;
            bus->write(addr, &m);
        }
    };
    template<> struct lsr<acc> {
        inline auto operator()(system_bus *bus, cpu6502::registers &regs) const -> void {
            regs.P.C = regs.A & 0x01;
            REG16 res = (REG16)regs.A << 1;
            res &= 0xFF;
            regs.P.N = 0;
            regs.P.Z = (res) == 0 ? 1 : 0;
            regs.A = (REG8)res;
        }
    };
    template<typename _AM> struct bit {
        inline auto operator()(system_bus *bus, cpu6502::registers &regs, int &cycles) const -> void {
            REG8 m;
            bus->read(_AM::get_addr(bus, regs, cycles), &m);
            auto r = m & regs.A;
            regs.P.Z = r == 0;
            regs.P.N = is_neg(m & 0x80) == 0 ? 0 : 1;
            regs.P.V = (m & 0x40) == 0 ? 0 : 1;
        }
    };
    template<> struct bit<imm> {
        inline auto operator()(system_bus *bus, cpu6502::registers &regs, int &cycles) const -> void {
            REG8 m;
            bus->read(imm::get_addr(bus, regs, cycles), &m);
            auto r = m & regs.A;
            regs.P.Z = r == 0;
        }
    };
    template<typename _AM> struct cmp {
        inline auto operator()(system_bus *bus, cpu6502::registers &regs, int &cycles) const -> void {
            cmp_v<_AM>()(bus, regs, regs.A, cycles);
        }
    };
    template<typename _AM> struct cpx {
        inline auto operator()(system_bus *bus, cpu6502::registers &regs, int &cycles) const -> void {
            cmp_v<_AM>()(bus, regs, regs.X, cycles);
        }
    };
    template<typename _AM> struct cpy {
        inline auto operator()(system_bus *bus, cpu6502::registers &regs, int &cycles) const -> void {
            cmp_v<_AM>()(bus, regs, regs.Y, cycles);
        }
    };
    template<typename _AM> struct jmp {
        inline auto operator()(system_bus *bus, cpu6502::registers &regs, int &cycles) const -> void {
            regs.PC = _AM::get_addr(bus, regs, cycles);
        }
    };
    template<typename _AM> struct jsr {
        inline auto operator()(system_bus *bus, cpu6502::registers &regs, int &cycles) const -> void {
            // push PC-1
            auto addr = _AM::get_addr(bus, regs, cycles);
            auto pc = regs.PC - 1;
            regs.PC = addr;
            UNPACK *p = (UNPACK*)&pc;
            stack_push(bus, regs, p->hi);
            stack_push(bus, regs, p->lo);
        }
    };
    template<typename _AM> struct ld {
        inline auto operator()(system_bus *bus, cpu6502::registers &regs, REG8 &target, int &cycles) const -> void {
            // M -> A
            auto addr = _AM::get_addr(bus, regs, cycles);
            REG8 m;
            bus->read(addr, &m);
            regs.P.Z = m == 0;
            regs.P.N = is_neg(m);
            target = m;
        }
    };
    template<typename _AM> struct lda {
        inline auto operator()(system_bus *bus, cpu6502::registers &regs, int &cycles) const -> void {
            ld<_AM>()(bus, regs, regs.A, cycles);
        }
    };
    template<typename _AM> struct ldx {
        inline auto operator()(system_bus *bus, cpu6502::registers &regs, int &cycles) const -> void {
            ld<_AM>()(bus, regs, regs.X, cycles);
        }
    };
    template<typename _AM> struct ldy {
        inline auto operator()(system_bus *bus, cpu6502::registers &regs, int &cycles) const -> void {
            ld<_AM>()(bus, regs, regs.Y, cycles);
        }
    };

    void cpu6502::clock()
    {
        if (_cycles_left_for_current_operation != 0) {
            _cycles_left_for_current_operation--;
            return;
        }

        if (_bus->reset) {
            // the reset line is high - jump to the reset code
            _cycles_left_for_current_operation = 5;
            UNPACK *p = (UNPACK*)&_registers.PC;
            stack_push(_bus, _registers, p->hi);
            stack_push(_bus, _registers, p->lo);
            stack_push(_bus, _registers, *((REG8*)&_registers.P));
            _registers.P.I = 1;
            _registers.P.D = 0;
            UNPACK *upc = (UNPACK*)&_registers.PC;
            _bus->read(0xFFFC, &upc->lo);
            _bus->read(0xFFFD, &upc->hi);
            return;
        }
        else if (_bus->nmi && _prev_nmi == false)
        {
            // Non-maskable interupt
            _cycles_left_for_current_operation = 6;
            _prev_nmi = true;
            UNPACK *p = (UNPACK*)&_registers.PC;
            stack_push(_bus, _registers, p->hi);
            stack_push(_bus, _registers, p->lo);
            stack_push(_bus, _registers, *((REG8*)&_registers.P));
            UNPACK *upc = (UNPACK*)&_registers.PC;
            _bus->read(0xFFFA, &upc->lo);
            _bus->read(0xFFFB, &upc->hi);
            _registers.P.I = 1;
            return;
        }
        else {
            // Maskable interupt (unmasked)
            _prev_nmi = false;
            if (_bus->irq && _registers.P.I == 0) {
                UNPACK *p = (UNPACK*)&_registers.PC;
                stack_push(_bus, _registers, p->hi);
                stack_push(_bus, _registers, p->lo);
                stack_push(_bus, _registers, *((REG8*)&_registers.P));
                UNPACK *upc = (UNPACK*)&_registers.PC;
                _bus->read(0xFFFE, &upc->lo);
                _bus->read(0xFFFF, &upc->hi);
                _registers.P.I = 1;
                _cycles_left_for_current_operation = 6;
                return;
            }
        }

        REG8 oc;
        _bus->read(_registers.PC, &oc);
        _registers.PC++;
        switch (oc) {
        case 0x00:
            _cycles_left_for_current_operation = 6;
            brk()(_bus, _registers);
            break;
        case 0x69:
            _cycles_left_for_current_operation = 1;
            adc<imm>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x6D:
            _cycles_left_for_current_operation = 3;
            adc<abs>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x65:
            _cycles_left_for_current_operation = 2;
            adc<zpg>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x61:
            _cycles_left_for_current_operation = 5;
            adc<ind_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x71:
            _cycles_left_for_current_operation = 4;
            adc<ind_y>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x75:
            _cycles_left_for_current_operation = 3;
            adc<zpg_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x7D:
            _cycles_left_for_current_operation = 3;
            adc<abs_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x79:
            _cycles_left_for_current_operation = 3;
            adc<abs_y>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x72:
            _cycles_left_for_current_operation = 4;
            adc<ind>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x29:
            _cycles_left_for_current_operation = 1;
            logic<and, imm>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x2D:
            _cycles_left_for_current_operation = 3;
            logic<and, abs>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x25:
            _cycles_left_for_current_operation = 2;
            logic<and, zpg>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x21:
            _cycles_left_for_current_operation = 5;
            logic<and, ind_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x31:
            _cycles_left_for_current_operation = 4;
            logic<and, ind_y>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x35:
            _cycles_left_for_current_operation = 3;
            logic<and, zpg_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x3D:
            _cycles_left_for_current_operation = 3;
            logic<and, abs_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x39:
            _cycles_left_for_current_operation = 3;
            logic<and, abs_y>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x32:
            _cycles_left_for_current_operation = 4;
            logic<and, ind>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x0E:
            _cycles_left_for_current_operation = 5;
            asl<abs>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x06:
            _cycles_left_for_current_operation = 4;
            asl<zpg>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x0A:
            _cycles_left_for_current_operation = 1;
            asl<acc>()(_bus, _registers);
            break;
        case 0x16:
            _cycles_left_for_current_operation = 5;
            asl<zpg_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x1E:
            _cycles_left_for_current_operation = 5;
            asl<abs_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x90: // bcc
            _cycles_left_for_current_operation = 1;
            branch()(_bus, _registers, [](const auto& regs) { return regs.P.C == 0; }, _cycles_left_for_current_operation);
            break;
        case 0xB0: // bcs
            _cycles_left_for_current_operation = 1;
            branch()(_bus, _registers, [](const auto &regs) { return regs.P.C != 0; }, _cycles_left_for_current_operation);
            break;
        case 0xF0: // beq
            _cycles_left_for_current_operation = 1;
            branch()(_bus, _registers, [](const auto &regs) { return regs.P.Z != 0; }, _cycles_left_for_current_operation);
            break;
        case 0x30: // bmi
            _cycles_left_for_current_operation = 1;
            branch()(_bus, _registers, [](const auto &regs) { return regs.P.N != 0; }, _cycles_left_for_current_operation);
            break;
        case 0xD0: // bne
            _cycles_left_for_current_operation = 1;
            branch()(_bus, _registers, [](const auto &regs) { return regs.P.Z == 0; }, _cycles_left_for_current_operation);
            break;
        case 0x10: // bpl
            _cycles_left_for_current_operation = 1;
            branch()(_bus, _registers, [](const auto &regs) { return regs.P.N == 0; }, _cycles_left_for_current_operation);
            break;
        case 0x80: // bra
            _cycles_left_for_current_operation = 1;
            branch()(_bus, _registers, [](const auto &regs) { return true; }, _cycles_left_for_current_operation);
            break;
        case 0x50: // bvc
            _cycles_left_for_current_operation = 1;
            branch()(_bus, _registers, [](const auto &regs) { return regs.P.V == 0; }, _cycles_left_for_current_operation);
            break;
        case 0x70: // bvs
            _cycles_left_for_current_operation = 1;
            branch()(_bus, _registers, [](const auto &regs) { return regs.P.V != 0; }, _cycles_left_for_current_operation);
            break;
        case 0x89:
            _cycles_left_for_current_operation = 1;
            bit<imm>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x2C:
            _cycles_left_for_current_operation = 3;
            bit<abs>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x24: // bit
            _cycles_left_for_current_operation = 2;
            bit<zpg>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x34:
            _cycles_left_for_current_operation = 3;
            bit<zpg_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x3C:
            _cycles_left_for_current_operation = 3;
            bit<abs_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x18: // clc
            _cycles_left_for_current_operation = 1;
            _registers.P.C = 0;
            break;
        case 0xD8: // cld
            _cycles_left_for_current_operation = 1;
            _registers.P.D = 0;
            break;
        case 0x58: // cli
            _cycles_left_for_current_operation = 1;
            _registers.P.I = 0;
            break;
        case 0xB8: // clv
            _cycles_left_for_current_operation = 1;
            _registers.P.V = 0;
            break;
        case 0x38: // sec
            _cycles_left_for_current_operation = 1;
            _registers.P.C = 1;
            break;
        case 0xF8: // sed
            _cycles_left_for_current_operation = 1;
            _registers.P.D = 1;
            break;
        case 0x78: // sei
            _cycles_left_for_current_operation = 1;
            _registers.P.I = 1;
            break;
        case 0xC9:
            _cycles_left_for_current_operation = 1;
            cmp<imm>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xCD:
            _cycles_left_for_current_operation = 3;
            cmp<abs>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xC5:
            _cycles_left_for_current_operation = 2;
            cmp<zpg>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xC1:
            _cycles_left_for_current_operation = 5;
            cmp<ind_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xD1:
            _cycles_left_for_current_operation = 4;
            cmp<ind_y>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xD5:
            _cycles_left_for_current_operation = 3;
            cmp<zpg_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xDD:
            _cycles_left_for_current_operation = 3;
            cmp<abs_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xD9:
            _cycles_left_for_current_operation = 3;
            cmp<abs_y>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xD2:
            _cycles_left_for_current_operation = 4;
            cmp<ind>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xE0:
            _cycles_left_for_current_operation = 1;
            cpx<imm>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xEC:
            _cycles_left_for_current_operation = 3;
            cpx<abs>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xE4:
            _cycles_left_for_current_operation = 2;
            cpx<zpg>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xC0:
            _cycles_left_for_current_operation = 1;
            cpy<imm>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xCC:
            _cycles_left_for_current_operation = 3;
            cpy<abs>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xC4:
            _cycles_left_for_current_operation = 2;
            cpy<zpg>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xCE:
            _cycles_left_for_current_operation = 5;
            incdec<dec, abs>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xC6:
            _cycles_left_for_current_operation = 4;
            incdec<dec, zpg>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x3A:
            _cycles_left_for_current_operation = 1;
            incdec<dec, acc>()(_bus, _registers);
            break;
        case 0xD6:
            _cycles_left_for_current_operation = 5;
            incdec<dec, zpg_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xDE:
            _cycles_left_for_current_operation = 5;
            incdec<dec, abs_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xCA:
            _cycles_left_for_current_operation = 1;
            incdec<dec, reg_x>()(_bus, _registers);
            break;
        case 0x88:
            _cycles_left_for_current_operation = 1;
            incdec<dec, reg_y>()(_bus, _registers);
            break;
        case 0xEE:
            _cycles_left_for_current_operation = 5;
            incdec<inc, abs>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xE6:
            _cycles_left_for_current_operation = 4;
            incdec<inc, zpg>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x1A:
            _cycles_left_for_current_operation = 1;
            incdec<inc, acc>()(_bus, _registers);
            break;
        case 0xF6:
            _cycles_left_for_current_operation = 5;
            incdec<inc, zpg_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xFE:
            _cycles_left_for_current_operation = 5;
            incdec<inc, abs_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xE8:
            _cycles_left_for_current_operation = 1;
            incdec<inc, reg_x>()(_bus, _registers);
            break;
        case 0xC8:
            _cycles_left_for_current_operation = 1;
            incdec<inc, reg_y>()(_bus, _registers);
            break;
        case 0x49:
            _cycles_left_for_current_operation = 1;
            logic<eor, imm>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x4D:
            _cycles_left_for_current_operation = 3;
            logic<eor, abs>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x45:
            _cycles_left_for_current_operation = 2;
            logic<eor, zpg>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x41:
            _cycles_left_for_current_operation = 5;
            logic<eor, ind_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x51:
            _cycles_left_for_current_operation = 4;
            logic<eor, ind_y>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x55:
            _cycles_left_for_current_operation = 3;
            logic<eor, zpg_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x5D:
            _cycles_left_for_current_operation = 3;
            logic<eor, abs_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x59:
            _cycles_left_for_current_operation = 3;
            logic<eor, abs_y>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x52:
            _cycles_left_for_current_operation = 4;
            logic<eor, ind>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x4C:
            _cycles_left_for_current_operation = 2;
            jmp<abs>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x7C:
            _cycles_left_for_current_operation = 5;
            jmp<ind_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x6C:
            _cycles_left_for_current_operation = 5;
            jmp<ind>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x20:
            _cycles_left_for_current_operation = 5;
            jsr<abs>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xA9:
            _cycles_left_for_current_operation = 1;
            lda<imm>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xAD:
            _cycles_left_for_current_operation = 3;
            lda<abs>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xA5:
            _cycles_left_for_current_operation = 2;
            lda<zpg>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xA1:
            _cycles_left_for_current_operation = 5;
            lda<ind_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xB1:
            _cycles_left_for_current_operation = 4;
            lda<ind_y>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xB5:
            _cycles_left_for_current_operation = 3;
            lda<zpg_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xBD:
            _cycles_left_for_current_operation = 3;
            lda<abs_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xB9:
            _cycles_left_for_current_operation = 3;
            lda<abs_y>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xB2:
            _cycles_left_for_current_operation = 4;
            lda<ind>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xA2:
            _cycles_left_for_current_operation = 1;
            ldx<imm>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xAE:
            _cycles_left_for_current_operation = 3;
            ldx<abs>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xA6:
            _cycles_left_for_current_operation = 2;
            ldx<zpg>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xBE:
            _cycles_left_for_current_operation = 3;
            ldx<abs_y>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xB6:
            _cycles_left_for_current_operation = 3;
            ldx<zpg_y>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xA0:
            _cycles_left_for_current_operation = 1;
            ldy<imm>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xAC:
            _cycles_left_for_current_operation = 3;
            ldy<abs>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xA4:
            _cycles_left_for_current_operation = 2;
            ldy<zpg>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xB4:
            _cycles_left_for_current_operation = 3;
            ldy<zpg_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xBC:
            _cycles_left_for_current_operation = 3;
            ldy<abs_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x4E:
            _cycles_left_for_current_operation = 5;
            lsr<abs>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x46:
            _cycles_left_for_current_operation = 4;
            lsr<zpg>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x4A:
            _cycles_left_for_current_operation = 1;
            lsr<acc>()(_bus, _registers);
            break;
        case 0x56:
            _cycles_left_for_current_operation = 5;
            lsr<zpg_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x5E:
            _cycles_left_for_current_operation = 5;
            lsr<abs_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0xEA:
            _cycles_left_for_current_operation = 1;
            break;
        case 0x09:
            _cycles_left_for_current_operation = 1;
            logic<or, imm>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x0D:
            _cycles_left_for_current_operation = 3;
            logic<or, abs>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x05:
            _cycles_left_for_current_operation = 2;
            logic<or, zpg>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x01:
            _cycles_left_for_current_operation = 5;
            logic<or, ind_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x11:
            _cycles_left_for_current_operation = 4;
            logic<or, ind_y>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x15:
            _cycles_left_for_current_operation = 3;
            logic<or, zpg_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x1D:
            _cycles_left_for_current_operation = 3;
            logic<or, abs_x>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x19:
            _cycles_left_for_current_operation = 3;
            logic<or, abs_y>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x12:
            _cycles_left_for_current_operation = 4;
            logic<or, ind>()(_bus, _registers, _cycles_left_for_current_operation);
            break;
        case 0x48:
            _cycles_left_for_current_operation = 2;
            stack_push(_bus, _registers, _registers.A);
            break;
        case 0x08:
            _cycles_left_for_current_operation = 2;
            stack_push(_bus, _registers, *((REG8*)&_registers.P));
            break;
        case 0xDA:
            _cycles_left_for_current_operation = 2;
            stack_push(_bus, _registers, _registers.X);
            break;
        case 0x68:
            _cycles_left_for_current_operation = 3;
            _registers.A = stack_pull(_bus, _registers);
            break;
        case 0x28:
            _cycles_left_for_current_operation = 3;
            *((REG8*)&_registers.P) = stack_pull(_bus, _registers);
            break;
        case 0xFA:
            _cycles_left_for_current_operation = 3;
            _registers.X = stack_pull(_bus, _registers);
            break;
        case 0x7A:
            _cycles_left_for_current_operation = 3;
            _registers.Y = stack_pull(_bus, _registers);
            break;
        }
    }
}

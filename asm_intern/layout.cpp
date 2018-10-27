#include "layout.h"

#include "logger.h"
#include "addressing.h"

#include <unordered_map>
#include <set>
#include <string>
#include <iomanip>

namespace dave
{

struct evaluator : public const_expression_visitor {
private:
    const std::unordered_map<std::string, value_range> &_labels;
public:
    evaluator(const std::unordered_map<std::string, value_range> &labels)
    : _labels(labels), _ok(true)
    {}

    bool _ok;
    value_range _value;

    virtual void visit(const value_expression* exp) override {
        _value.first = _value.second = exp->value;
    }

    virtual void visit(const label_expression* exp) override {
        auto f = _labels.find(exp->name);
        if (f == _labels.end()) {
            logger::def->log() << "The label '" << exp->name << "' could not be found" << end();
            _ok = false;
        }
        else {
            _value = f->second;
        }
    }

    virtual void visit(const addr_expression* exp) override {
        exp->value->accept(this);
    }

    virtual void visit(const reg_expression* exp) override {
        logger::def->log("Register has a non-static value");
        _ok = false;
    }

    virtual void visit(const add_expression* exp) override {
        exp->rhs->accept(this);
        auto rhs = _value;
        if (_ok) {
            exp->lhs->accept(this);
            _value.first += rhs.first;
            _value.second += rhs.second;
        }
    }

    virtual void visit(const subtract_expression* exp) override {
        exp->rhs->accept(this);
        auto rhs = _value;
        if (_ok) {
            exp->lhs->accept(this);
            _value.first -= rhs.second;
            _value.second -= rhs.first;
        }
    }

    virtual void visit(const multiply_expression* exp) override {
        exp->rhs->accept(this);
        auto rhs = _value;
        if (_ok) {
            exp->lhs->accept(this);
            auto a = _value.first;
            auto b = _value.second;
            _value.first = _value.second = a*rhs.first;
            auto x = a*rhs.second;
            if (x < _value.first) _value.first = x;
            if (x > _value.second) _value.second = x;
            x = b*rhs.first;
            if (x < _value.first) _value.first = x;
            if (x > _value.second) _value.second = x;
            x = b*rhs.second;
            if (x < _value.first) _value.first = x;
            if (x > _value.second) _value.second = x;
        }
    }

    virtual void visit(const indirect_addr_expression* exp) override {
        logger::def->log("Indirect address has a non-static value");
        _ok = false;
    }

    virtual void visit(const func_expression* exp) override {
        exp->parameter->accept(this);
        if (!_ok) return;
        // result can be 0x0034->0x0057 (lo = 34-57, hi = 0-0)
        // result can be 0x0034->0x0137 (lo = 0-255, hi = 0-1)
        // result can be 0x0034->0x0101 (lo = 34-FF or 00-01 thus 00-FF, hi = 0-1)
        // result can be 0x0134->0x0136 (lo = 34-36, hi = 01-01)
        // if hi is single value, then lo is (lo(min)->lo(max)), else lo=00->FF
        if (exp->name == "lo") {
            if ((_value.first & 0xFF00) == (_value.second & 0xFF00)) {
                _value.first &= 0x00FF;
                _value.second &= 0x00FF;
            }
            else {
                _value.first = 0x00;
                _value.second = 0xFF;
            }
        }
        else if (exp->name == "hi") {
            _value.first >>= 8;
            _value.second >>= 8;
        }
    }
};

void getInstructionSizeRange(instr_kind kind, value_range &size)
{
    static int sizes[][2] = {
        { 0,   0 }, // BASE
        { 0,   0 }, // START
        { 1,   2 }, // DATA (1 or 2 bytes of data)
        { 1,   1 }, // BRK
        { 2,   3 }, // ADC
        { 2,   3 }, // SBC
        { 2,   3 }, // AND
        { 1,   3 }, // ASL
        { 2,   2 }, // BCC
        { 2,   2 }, // BCS
        { 2,   2 }, // BEQ
        { 2,   2 }, // BMI
        { 2,   2 }, // BNE
        { 2,   2 }, // BPL
        { 2,   2 }, // BRA
        { 2,   2 }, // BVC
        { 2,   2 }, // BVS
        { 2,   3 }, // BIT
        { 1,   1 }, // CLC
        { 1,   1 }, // CLD
        { 1,   1 }, // CLI
        { 1,   1 }, // CLV
        { 1,   1 }, // SEC
        { 1,   1 }, // SED
        { 1,   1 }, // SEI
        { 2,   3 }, // CMP
        { 2,   3 }, // CPX
        { 2,   3 }, // CPY
        { 1,   3 }, // DEC
        { 1,   3 }, // INC
        { 2,   3 }, // EOR
        { 2,   3 }, // JMP
        { 3,   3 }, // JSR
        { 2,   3 }, // LDA
        { 2,   3 }, // LDX
        { 2,   3 }, // LDY
        { 1,   3 }, // LSR
        { 1,   1 }, // NOP
        { 2,   3 }, // OR
        { 1,   1 }, // PHA
        { 1,   1 }, // PHP
        { 1,   1 }, // PHX
        { 1,   1 }, // PLA
        { 1,   1 }, // PLP
        { 1,   1 }, // PLX
        { 1,   1 }, // PLY
        { 1,   3 }, // ROL
        { 1,   1 }, // RTI
        { 1,   1 }, // RTS
        { 1,   3 }, // ROR
        { 2,   3 }, // STA
        { 2,   3 }, // STY
        { 2,   3 }, // STX
        { 1,   1 }, // TAX
        { 1,   1 }, // TXA
        { 1,   1 }, // TAY
        { 1,   1 }, // TYA
        { 1,   1 }, // TSX
        { 1,   1 }, // TXS
    };

    auto s = sizes[(int)kind];
    size.first = s[0];
    size.second = s[1];
}

bool tryResolveInstructionSize(instr_kind kind, addressing_mode mode, size_t &size)
{
    static int sizes[][13] = {
        //imm, abs, absx, absy, zpg, zpgx, zpgy, ind, indx, indy, acc, regx, regy
        {-1,  -1,  -1,   -1,   -1,  -1,   -1,   -1,  -1,   -1,   -1,  -1,   -1 }, // BASE
        {-1,  -1,  -1,   -1,   -1,  -1,   -1,   -1,  -1,   -1,   -1,  -1,   -1 }, // START
        {-1,  -1,  -1,   -1,   -1,  -1,   -1,   -1,  -1,   -1,   -1,  -1,   -1 }, // DATA
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // BRK
        { 2,   3,   3,    3,    2,   2,   -1,    2,   2,    2,   -1,  -1,   -1 }, // ADC
        { 2,   3,   3,    3,    2,   2,   -1,    2,   2,    2,   -1,  -1,   -1 }, // SBC
        { 2,   3,   3,    3,    2,   2,   -1,    2,   2,    2,   -1,  -1,   -1 }, // AND
        {-1,   3,   3,   -1,    2,   2,   -1,   -1,  -1,   -1,    1,  -1,   -1 }, // ASL
        { 2,   2,   2,    2,    2,   2,    2,    2,   2,    2,    2,   2,    2 }, // BCC
        { 2,   2,   2,    2,    2,   2,    2,    2,   2,    2,    2,   2,    2 }, // BCS
        { 2,   2,   2,    2,    2,   2,    2,    2,   2,    2,    2,   2,    2 }, // BEQ
        { 2,   2,   2,    2,    2,   2,    2,    2,   2,    2,    2,   2,    2 }, // BMI
        { 2,   2,   2,    2,    2,   2,    2,    2,   2,    2,    2,   2,    2 }, // BNE
        { 2,   2,   2,    2,    2,   2,    2,    2,   2,    2,    2,   2,    2 }, // BPL
        { 2,   2,   2,    2,    2,   2,    2,    2,   2,    2,    2,   2,    2 }, // BRA
        { 2,   2,   2,    2,    2,   2,    2,    2,   2,    2,    2,   2,    2 }, // BVC
        { 2,   2,   2,    2,    2,   2,    2,    2,   2,    2,    2,   2,    2 }, // BVS
        { 2,   3,   3,   -1,    2,   2,   -1,   -1,  -1,   -1,   -1,  -1,   -1 }, // BIT
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // CLC
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // CLD
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // CLI
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // CLV
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // SEC
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // SED
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // SEI
        { 2,   3,   3,    3,    2,   2,   -1,    2,   2,    2,   -1,  -1,   -1 }, // CMP
        { 2,   3,  -1,   -1,    2,  -1,   -1,   -1,  -1,   -1,   -1,  -1,   -1 }, // CPX
        { 2,   3,  -1,   -1,    2,  -1,   -1,   -1,  -1,   -1,   -1,  -1,   -1 }, // CPY
        {-1,   3,   3,   -1,    2,   2,   -1,   -1,  -1,   -1,    1,   1,    1 }, // DEC
        {-1,   3,   3,   -1,    2,   2,   -1,   -1,  -1,   -1,    1,   1,    1 }, // INC
        { 2,   3,   3,    3,    2,   2,   -1,    2,   2,    2,   -1,  -1,   -1 }, // EOR
        {-1,   3,  -1,   -1,   -1,  -1,   -1,    2,   2,   -1,   -1,  -1,   -1 }, // JMP
        {-1,   3,  -1,   -1,    3,  -1,   -1,   -1,  -1,   -1,   -1,  -1,   -1 }, // JSR
        { 2,   3,   3,    3,    2,   2,   -1,    2,   2,    2,   -1,  -1,   -1 }, // LDA
        { 2,   3,  -1,    3,    2,  -1,    2,   -1,  -1,   -1,   -1,  -1,   -1 }, // LDX
        { 2,   3,   3,   -1,    2,   2,   -1,   -1,  -1,   -1,   -1,  -1,   -1 }, // LDY
        {-1,   3,   3,   -1,    2,   2,   -1,   -1,  -1,   -1,    1,  -1,   -1 }, // LSR
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // NOP
        { 2,   3,   3,    3,    2,   2,   -1,    2,   2,    2,   -1,  -1,   -1 }, // OR
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // PHA
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // PHP
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // PHX
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // PLA
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // PLP
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // PLX
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // PLY
        {-1,   3,   3,   -1,    2,   2,   -1,   -1,  -1,   -1,    1,  -1,   -1 }, // ROL
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // RTI
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // RTS
        {-1,   3,   3,   -1,    2,   2,   -1,   -1,  -1,   -1,    1,  -1,   -1 }, // ROR
        {-1,   3,   3,    3,    2,   2,   -1,   -1,   2,    2,   -1,  -1,   -1 }, // STA
        {-1,   3,  -1,   -1,    2,   2,   -1,   -1,  -1,   -1,   -1,  -1,   -1 }, // STY
        {-1,   3,  -1,   -1,    2,   2,   -1,   -1,  -1,   -1,   -1,  -1,   -1 }, // STX
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // TAX
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // TXA
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // TAY
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // TYA
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // TSX
        { 1,   1,   1,    1,    1,   1,    1,    1,   1,    1,    1,   1,    1 }, // TXS
    };
    int s = sizes[(int)kind][(int)mode];
    if (s == -1) {
        logger::def->log() << "Unsupported addressing mode for the instruction. The '" << kind << "' instruction does not support " << mode << end();
        return false;
    }
    else {
        size = (size_t)s;
        return true;
    }
}

void resolveInstructionOpcode(instr_kind kind, addressing_mode mode, REG8 &op)
{
    static REG8 ops[][13] = {
        //imm,  abs,  absx, absy, zpg,  zpgx, zpgy, ind,  indx, indy, acc,  regx, regy
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // BASE
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // START
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // DATA
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // BRK
        { 0x69, 0x6D, 0x7D, 0x79, 0x65, 0x75, 0x00, 0x72, 0x61, 0x71, 0x00, 0x00, 0x00 }, // ADC
        { 0xE9, 0xED, 0xFD, 0xF9, 0xE5, 0xF5, 0x00, 0xF2, 0xE1, 0xF1, 0x00, 0x00, 0x00 }, // SBC
        { 0x29, 0x2D, 0x3D, 0x39, 0x25, 0x35, 0x00, 0x32, 0x21, 0x31, 0x00, 0x00, 0x00 }, // AND
        { 0x00, 0x0E, 0x1E, 0x00, 0x06, 0x16, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00 }, // ASL
        { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 }, // BCC
        { 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0, 0xB0 }, // BCS
        { 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0 }, // BEQ
        { 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30 }, // BMI
        { 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0 }, // BNE
        { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 }, // BPL
        { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 }, // BRA
        { 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50 }, // BVC
        { 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70 }, // BVS
        { 0x89, 0x2C, 0x3C, 0x00, 0x24, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // BIT
        { 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18 }, // CLC
        { 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8, 0xD8 }, // CLD
        { 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58 }, // CLI
        { 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8 }, // CLV
        { 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38 }, // SEC
        { 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8 }, // SED
        { 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78 }, // SEI
        { 0xC9, 0xCD, 0xDD, 0xD9, 0xC5, 0xD5, 0x00, 0xD2, 0xC1, 0xD1, 0x00, 0x00, 0x00 }, // CMP
        { 0xE0, 0xEC, 0x00, 0x00, 0xE4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // CPX
        { 0xC0, 0xCC, 0x00, 0x00, 0xC4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // CPY
        { 0x00, 0xCE, 0xDE, 0x00, 0xC6, 0xD6, 0x00, 0x00, 0x00, 0x00, 0x3A, 0xCA, 0x88 }, // DEC
        { 0x00, 0xEE, 0xFE, 0x00, 0xE6, 0xF6, 0x00, 0x00, 0x00, 0x00, 0x1A, 0xE8, 0xC8 }, // INC
        { 0x49, 0x4D, 0x5D, 0x59, 0x45, 0x55, 0x00, 0x52, 0x41, 0x51, 0x00, 0x00, 0x00 }, // EOR
        { 0x00, 0x4C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6C, 0x7C, 0x00, 0x00, 0x00, 0x00 }, // JMP
        { 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // JSR
        { 0xA9, 0xAD, 0xBD, 0xB9, 0xA5, 0xB5, 0x00, 0xB2, 0xA1, 0xB1, 0x00, 0x00, 0x00 }, // LDA
        { 0xA2, 0xAE, 0x00, 0xBE, 0xA6, 0x00, 0xB6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // LDX
        { 0xA0, 0xAC, 0xBC, 0x00, 0xA4, 0xB4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // LDY
        { 0x00, 0x4E, 0x5E, 0x00, 0x46, 0x56, 0x00, 0x00, 0x00, 0x00, 0x4A, 0x00, 0x00 }, // LSR
        { 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA }, // NOP
        { 0x09, 0x0D, 0x1D, 0x19, 0x05, 0x15, 0x00, 0x12, 0x01, 0x11, 0x00, 0x00, 0x00 }, // OR
        { 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48 }, // PHA
        { 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08 }, // PHP
        { 0xDA, 0xDA, 0xDA, 0xDA, 0xDA, 0xDA, 0xDA, 0xDA, 0xDA, 0xDA, 0xDA, 0xDA, 0xDA }, // PHX
        { 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68 }, // PLA
        { 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28 }, // PLP
        { 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA }, // PLX
        { 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A }, // PLY
        { 0x00, 0x2E, 0x3E, 0x00, 0x26, 0x36, 0x00, 0x00, 0x00, 0x00, 0x2A, 0x00, 0x00 }, // ROL
        { 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 }, // RTI
        { 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60 }, // RTS
        { 0x00, 0x6E, 0x7E, 0x00, 0x66, 0x76, 0x00, 0x00, 0x00, 0x00, 0x6A, 0x00, 0x00 }, // ROR
        { 0x00, 0x8D, 0x9D, 0x99, 0x85, 0x95, 0x00, 0x00, 0x81, 0x91, 0x00, 0x00, 0x00 }, // STA
        { 0x00, 0x8C, 0x00, 0x00, 0x84, 0x94, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // STY
        { 0x00, 0x8E, 0x00, 0x00, 0x86, 0x96, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // STX
        { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA }, // TAX
        { 0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A, 0x8A }, // TXA
        { 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8, 0xA8 }, // TAY
        { 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98, 0x98 }, // TYA
        { 0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA, 0xBA }, // TSX
        { 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A }, // TXS
    };
    op = ops[(int)kind][(int)mode];
}

inline bool is_branch_instr(instr_kind kind) {
    switch(kind) {
        case instr_kind::BCC:
        case instr_kind::BCS:
        case instr_kind::BEQ:
        case instr_kind::BMI:
        case instr_kind::BNE:
        case instr_kind::BPL:
        case instr_kind::BRA:
        case instr_kind::BVC:
        case instr_kind::BVS:
            return true;
        default:
            return false;     
    }
}

// void report(const std::vector<file> &files) {
//     for(auto &f : files) {
//         for(auto &l : f.lines) {
//             std::cout << l._text << std::endl;
//             if (l._instr != nullptr) {
//                 std::cout << "0x" << std::setfill('0') << std::hex << std::uppercase << std::setw(4) << l._instr->_address_range.first
//                           << '-'
//                           << "0x" << std::setfill('0') << std::hex << std::uppercase << std::setw(4) << l._instr->_address_range.second
//                           << ": " << l._instr->_kind << " [" << l._instr->_addressing_mode << ']'
//                           << " (" << std::dec << l._instr->_instr_size_range.first << '-' << l._instr->_instr_size_range.second << ")"
//                           << std::endl;
//             }
//         }
//     }
// }

bool tryLayout(std::vector<file> &files, REG16 &startAddress)
{
    std::shared_ptr<expression> start;
    std::unordered_map<std::string, value_range> labels;
    // Find all labels, initialise context and find instruction size ranges
    // Also note and remove START instructions
    for(auto &f : files) {
        for(auto &l : f.lines) {
            if (l._instr != nullptr) {
                if (l._instr->_kind == instr_kind::START) {
                    start = l._instr->_parameter;
                    l._instr.reset();
                }
                else {
                    l._instr->_address_range.first = 0x0000;
                    l._instr->_address_range.second = 0xFFFF;
                    l._instr->_address = 0x0000;
                    l._instr->_instr_size_range.first = 1;
                    l._instr->_instr_size_range.second = 3;
                    l._instr->_instr_size = 0;
                    getInstructionSizeRange(l._instr->_kind, l._instr->_instr_size_range);
                    if (!l._instr->_label.empty()) {
                        if (!labels.emplace(l._instr->_label, value_range(0x0000, 0xFFFF)).second) {
                            logger::def->log() << "The label '" << l._instr->_label << "' was defined more than once" << end();
                            return false;
                        }
                    }
                }
            }
        }
    }

    // Resolve the BASE instruction and address ranges
    bool changes = true;
    while (changes) {
        changes = false;
        value_range cur(0x0000, 0x0000);
        for(auto &f : files) {
            logger::def->_filename = f.filename;
            logger::def->_line_no = 0;
            for(auto &l : f.lines) {
                logger::def->_line_no = l._line_no;
                if (l._instr != nullptr) {
                    if (l._instr->_kind == instr_kind::BASE) {
                        if (l._instr->_parameter == nullptr) {
                            logger::def->log("The BASE instruction requires a parameter with a type of 'addr'");
                            return false;
                        }
                        evaluator eval(labels);
                        l._instr->_parameter->accept(&eval);
                        if (!eval._ok) {
                            return false;
                        }
                        changes |= l._instr->_address_range.first != eval._value.first;
                        changes |= l._instr->_address_range.second != eval._value.second;
                        l._instr->_address_range = eval._value;
                        cur = eval._value;
                    }
                    else {
                        changes |= l._instr->_address_range.first != cur.first;
                        changes |= l._instr->_address_range.second != cur.second;
                        l._instr->_address_range = cur;
                        if (!l._instr->_label.empty()) {
                            auto f = labels.find(l._instr->_label);
                            if (f != labels.end()) {
                                changes |= f->second.first != cur.first;
                                changes |= f->second.second != cur.second;
                                f->second = cur;
                            }
                        }
                        cur.first += l._instr->_instr_size_range.first;
                        cur.second += l._instr->_instr_size_range.second;
                    }
                }
            }
        }
    }

    // Now we should have an accurate picture of where the labels will be (approx) and should be able to find the addressing modes
    for(auto &f : files) {
        logger::def->_filename = f.filename;
        logger::def->_line_no = 0;
        for(auto &l : f.lines) {
            logger::def->_line_no = l._line_no;
            if (l._instr != nullptr && l._instr->_parameter != nullptr && l._instr->_kind != instr_kind::BASE && l._instr->_kind != instr_kind::DATA) {
                if (!tryDetermineAddressingMode(labels, l._instr->_parameter, l._instr->_addressing_mode, l._instr->_addressing_mode_parameter)) {
                    return false;
                }
            }
        }
    }

    // All addressing mode are now known, thus we know the size of all instructions, except DATA instructions.
    for(auto &f : files) {
        logger::def->_filename = f.filename;
        logger::def->_line_no = 0;
        for(auto &l : f.lines) {
            logger::def->_line_no = l._line_no;
            if (l._instr != nullptr) {
                if (l._instr->_kind == instr_kind::BASE) {
                    l._instr->_instr_size = 0;
                }
                else if (l._instr->_kind == instr_kind::DATA) {
                }
                else {
                    if (!tryResolveInstructionSize(l._instr->_kind, l._instr->_addressing_mode, l._instr->_instr_size)) {
                        return false;
                    }
                }
            }
        }
    }

    // Now we know the size of all instruction (except DATA), we should be able to find the address of all instructions
    changes = true;
    while (changes) {
        changes = false;
        value_range cur(0x0000, 0x0000);
        for(auto &f : files) {
            logger::def->_filename = f.filename;
            logger::def->_line_no = 0;
            for(auto &l : f.lines) {
                logger::def->_line_no = l._line_no;
                if (l._instr != nullptr) {
                    if (l._instr->_kind == instr_kind::BASE) {
                        evaluator eval(labels);
                        l._instr->_parameter->accept(&eval);
                        if (!eval._ok) {
                            return false;
                        }
                        changes |= l._instr->_address_range.first != eval._value.first;
                        changes |= l._instr->_address_range.second != eval._value.second;
                        cur = eval._value;
                    }
                    else if (l._instr->_kind == instr_kind::DATA) {
                        changes |= l._instr->_address_range.first != cur.first;
                        changes |= l._instr->_address_range.second != cur.second;
                        l._instr->_address_range = cur;
                        if (!l._instr->_label.empty()) {
                            auto f = labels.find(l._instr->_label);
                            if (f != labels.end()) {
                                changes |= f->second.first != cur.first;
                                changes |= f->second.second != cur.second;
                                f->second = cur;
                            }
                        }
                        // Update the info on this parameter
                        evaluator eval(labels);
                        l._instr->_parameter->accept(&eval);
                        if (!eval._ok) {
                            return false;
                        }
                        if (eval._value.first < 0 || eval._value.second > 0xFFFF) {
                            logger::def->log("Parameter of DATA instruction is out of range");
                            return false;
                        }
                        auto f = eval._value.first < 256 ? 1 : 2;
                        auto t = eval._value.second < 256 ? 1 : 2;
                        changes |= f != l._instr->_instr_size_range.first;
                        changes |= t != l._instr->_instr_size_range.second;
                        l._instr->_instr_size_range.first = f;
                        l._instr->_instr_size_range.second = t;
                        cur.first += l._instr->_instr_size_range.first;
                        cur.second += l._instr->_instr_size_range.second;
                        if (l._instr->_instr_size_range.first == l._instr->_instr_size_range.second) {
                            l._instr->_instr_size = ((REG16)l._instr->_instr_size_range.first) & 0xFFFF;
                        }
                    }
                    else {
                        changes |= l._instr->_address_range.first != cur.first;
                        changes |= l._instr->_address_range.second != cur.second;
                        l._instr->_address_range = cur;
                        if (!l._instr->_label.empty()) {
                            auto f = labels.find(l._instr->_label);
                            if (f != labels.end()) {
                                changes |= f->second.first != cur.first;
                                changes |= f->second.second != cur.second;
                                f->second = cur;
                            }
                        }
                        cur.first += l._instr->_instr_size;
                        cur.second += l._instr->_instr_size;
                    }
                }
            }
        }
    }

    // At this point, if the some address is still unsure, then we cannot determine it
    for(auto &f : files) {
        logger::def->_filename = f.filename;
        logger::def->_line_no = 0;
        for(auto &l : f.lines) {
            logger::def->_line_no = l._line_no;
            if (l._instr != nullptr) {
                if (l._instr->_kind == instr_kind::BASE) {
                    l._instr.reset();
                }
                else {
                    if (l._instr->_address_range.first == l._instr->_address_range.second) {
                        l._instr->_address = l._instr->_address_range.first;
                        if (!l._instr->_label.empty()) {
                            auto f = labels.find(l._instr->_label);
                            if (f != labels.end()) {
                                f->second = l._instr->_address_range;
                            }
                        }
                    }
                    else {
                        logger::def->log("Unable to determine an address");
                        return false;
                    }
                }
            }
        }
    }

    // Now all labels should be resolved - so we can update all expressions
    for(auto &f : files) {
        logger::def->_filename = f.filename;
        logger::def->_line_no = 0;
        for(auto &l : f.lines) {
            logger::def->_line_no = l._line_no;
            if (l._instr != nullptr) {
                evaluator eval(labels);
                if (l._instr->_kind == instr_kind::DATA) {
                    l._instr->_parameter->accept(&eval);
                }
                else {
                    l._instr->_binary_representation.emplace_back();
                    resolveInstructionOpcode(l._instr->_kind, l._instr->_addressing_mode, l._instr->_binary_representation.back());
                    if (l._instr->_addressing_mode_parameter == nullptr) {
                        continue; // We have no parameter
                    }
                    l._instr->_addressing_mode_parameter->accept(&eval);
                }
                if (eval._value.first != eval._value.second) {
                    logger::def->log("Could not determine a value for the expression");
                    return false;
                }
                if (is_branch_instr(l._instr->_kind)) {
                    // We have a signed parameter - must be 8 bit signed - find the relative value
                    __int64_t rel = eval._value.first - l._instr->_address - 2; // -2 for the pc moving 2 bytes
                    if (rel < -128 || rel > 127) {
                        logger::def->log("The expression value is out of range");
                        return false;
                    }
                    l._instr->_binary_representation.push_back(rel & 0xFF);
                    continue;
                }
                if (eval._value.first < 0 || eval._value.first > 0xFFFF) {
                    logger::def->log("The expression value is out of range");
                    return false;
                }
                if (l._instr->_kind == instr_kind::DATA) {
                    if (eval._value.first < 256) {
                        // single byte
                        l._instr->_binary_representation.push_back(eval._value.first & 0xFF);
                    }
                    else {
                        // two byte parameter
                        l._instr->_binary_representation.push_back(eval._value.first & 0xFF);
                        l._instr->_binary_representation.push_back(eval._value.first >> 8);
                    }
                }
                else if (l._instr->_instr_size == 2) {
                    // one byte parameter
                    if (eval._value.first < 256) {
                        // ok
                        l._instr->_binary_representation.push_back(eval._value.first & 0xFF);
                    }
                    else {
                        // not ok
                        logger::def->log("Instruction expects a single byte as parameter while the parameter is out of range");
                        return false;
                    }
                }
                else {
                    // two byte parameter
                    l._instr->_binary_representation.push_back(eval._value.first & 0xFF);
                    l._instr->_binary_representation.push_back(eval._value.first >> 8);
                }
            }
        }
    }

    if (start != nullptr) {
        evaluator seval(labels);
        start->accept(&seval);
        if (seval._value.first != seval._value.second) {
            logger::def->log("Could not find a value for the start address");
            return false;
        }
        startAddress = seval._value.first;
    }
    else {
        logger::def->log("No 'START' instruction found");
        return false;
    }

    return true;
}


}
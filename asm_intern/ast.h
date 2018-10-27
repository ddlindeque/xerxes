#ifndef __ASTH
#define __ASTH

#include <string>
#include <vector>
#include <memory>
#include <ostream>

#include "../xerxes_lib/common.h"

namespace dave
{
    struct value_expression;
    struct addr_expression;
    struct label_expression;
    struct reg_expression;
    struct add_expression;
    struct subtract_expression;
    struct multiply_expression;
    struct indirect_addr_expression;
    struct func_expression;

    struct expression_visitor {
        virtual ~expression_visitor() {}
        virtual void visit(value_expression* exp) = 0;
        virtual void visit(addr_expression* exp) = 0;
        virtual void visit(label_expression* exp) = 0;
        virtual void visit(reg_expression* exp) = 0;
        virtual void visit(add_expression* exp) = 0;
        virtual void visit(subtract_expression* exp) = 0;
        virtual void visit(multiply_expression* exp) = 0;
        virtual void visit(indirect_addr_expression* exp) = 0;
        virtual void visit(func_expression* exp) = 0;
    };

    struct const_expression_visitor {
        virtual ~const_expression_visitor() {}
        virtual void visit(const value_expression* exp) = 0;
        virtual void visit(const addr_expression* exp) = 0;
        virtual void visit(const label_expression* exp) = 0;
        virtual void visit(const reg_expression* exp) = 0;
        virtual void visit(const add_expression* exp) = 0;
        virtual void visit(const subtract_expression* exp) = 0;
        virtual void visit(const multiply_expression* exp) = 0;
        virtual void visit(const indirect_addr_expression* exp) = 0;
        virtual void visit(const func_expression* exp) = 0;
    };

    struct expression {
        expression() = default;
        expression(const expression&) = default;
        expression(expression&&) = default;
        virtual ~expression() { }

        virtual void accept(expression_visitor *) = 0;
        virtual void accept(const_expression_visitor *) const = 0;
    };

    struct value_expression : public expression {
        REG16 value;

        value_expression() = default;
        value_expression(const value_expression&) = default;
        value_expression(value_expression&&) = default;
        explicit value_expression(REG16 value) : value(value) { }

        virtual void accept(expression_visitor *visitor) override { visitor->visit(this); }
        virtual void accept(const_expression_visitor *visitor) const override { visitor->visit(this); }
    };

    struct addr_expression : public expression {
        std::shared_ptr<expression> value;

        addr_expression() = default;
        addr_expression(const addr_expression&) = default;
        addr_expression(addr_expression&&) = default;
        explicit addr_expression(const std::shared_ptr<expression> &value) : value(value) { }

        virtual void accept(expression_visitor *visitor) override { visitor->visit(this); }
        virtual void accept(const_expression_visitor *visitor) const override { visitor->visit(this); }
    };

    struct label_expression : public expression {
        std::string name;

        label_expression() = default;
        label_expression(const label_expression&) = default;
        label_expression(label_expression&&) = default;
        explicit label_expression(const std::string &name) : name(name) { }

        virtual void accept(expression_visitor *visitor) override { visitor->visit(this); }
        virtual void accept(const_expression_visitor *visitor) const override { visitor->visit(this); }
    };

    struct reg_expression : public expression {
        char reg;

        reg_expression() = default;
        reg_expression(const reg_expression&) = default;
        reg_expression(reg_expression&&) = default;
        explicit reg_expression(char reg) : reg(reg) { }

        virtual void accept(expression_visitor *visitor) override { visitor->visit(this); }
        virtual void accept(const_expression_visitor *visitor) const override { visitor->visit(this); }
    };

    struct bin_expression : public expression {
        std::shared_ptr<expression> lhs;
        std::shared_ptr<expression> rhs;

        bin_expression() = default;
        bin_expression(const bin_expression&) = default;
        bin_expression(bin_expression&&) = default;
        explicit bin_expression(const std::shared_ptr<expression> &lhs, const std::shared_ptr<expression> &rhs) : lhs(lhs), rhs(rhs) { }
    };

    struct add_expression : public bin_expression {
        add_expression() = default;
        add_expression(const add_expression&) = default;
        add_expression(add_expression&&) = default;
        explicit add_expression(const std::shared_ptr<expression> &lhs, const std::shared_ptr<expression> &rhs) : bin_expression(lhs, rhs) { }

        virtual void accept(expression_visitor *visitor) override { visitor->visit(this); }
        virtual void accept(const_expression_visitor *visitor) const override { visitor->visit(this); }
    };

    struct subtract_expression : public bin_expression {
        subtract_expression() = default;
        subtract_expression(const subtract_expression&) = default;
        subtract_expression(subtract_expression&&) = default;
        explicit subtract_expression(const std::shared_ptr<expression> &lhs, const std::shared_ptr<expression> &rhs) : bin_expression(lhs, rhs) { }

        virtual void accept(expression_visitor *visitor) override { visitor->visit(this); }
        virtual void accept(const_expression_visitor *visitor) const override { visitor->visit(this); }
    };

    struct multiply_expression : public bin_expression {
        multiply_expression() = default;
        multiply_expression(const multiply_expression&) = default;
        multiply_expression(multiply_expression&&) = default;
        explicit multiply_expression(const std::shared_ptr<expression> &lhs, const std::shared_ptr<expression> &rhs) : bin_expression(lhs, rhs) { }

        virtual void accept(expression_visitor *visitor) override { visitor->visit(this); }
        virtual void accept(const_expression_visitor *visitor) const override { visitor->visit(this); }
    };

    struct indirect_addr_expression : public expression {
        std::shared_ptr<expression> addr;

        indirect_addr_expression() = default;
        indirect_addr_expression(const indirect_addr_expression&) = default;
        indirect_addr_expression(indirect_addr_expression&&) = default;
        explicit indirect_addr_expression(const std::shared_ptr<expression> &addr) : addr(addr) { }

        virtual void accept(expression_visitor *visitor) override { visitor->visit(this); }
        virtual void accept(const_expression_visitor *visitor) const override { visitor->visit(this); }
    };

    struct func_expression : public expression {
        std::string name;
        std::shared_ptr<expression> parameter;

        func_expression() = default;
        func_expression(const func_expression&) = default;
        func_expression(func_expression&&) = default;
        explicit func_expression(const std::string &name, const std::shared_ptr<expression> &parameter) : name(name), parameter(parameter) { }

        virtual void accept(expression_visitor *visitor) override { visitor->visit(this); }
        virtual void accept(const_expression_visitor *visitor) const override { visitor->visit(this); }
    };

    enum class instr_kind {
        BASE,
        START,
        DATA,
        BRK,
        ADC,
        SBC,
        AND,
        ASL,
        BCC,
        BCS,
        BEQ,
        BMI,
        BNE,
        BPL,
        BRA,
        BVC,
        BVS,
        BIT,
        CLC,
        CLD,
        CLI,
        CLV,
        SEC,
        SED,
        SEI,
        CMP,
        CPX,
        CPY,
        DEC,
        INC,
        EOR,
        JMP,
        JSR,
        LDA,
        LDX,
        LDY,
        LSR,
        NOP,
        OR,
        PHA,
        PHP,
        PHX,
        PLA,
        PLP,
        PLX,
        PLY,
        ROL,
        RTI,
        RTS,
        ROR,
        STA,
        STY,
        STX,
        TAX,
        TXA,
        TAY,
        TYA,
        TSX,
        TXS
    };

    inline auto operator << (std::ostream &os, instr_kind ik) -> std::ostream& {
        switch(ik) {
            case instr_kind::BASE: return os << "BASE";
            case instr_kind::DATA: return os << "DATA";
            case instr_kind::BRK: return os << "BRK";
            case instr_kind::ADC: return os << "ADC";
            case instr_kind::SBC: return os << "SBC";
            case instr_kind::AND: return os << "AND";
            case instr_kind::ASL: return os << "ASL";
            case instr_kind::BCC: return os << "BCC";
            case instr_kind::BCS: return os << "BCS";
            case instr_kind::BEQ: return os << "BEQ";
            case instr_kind::BMI: return os << "BMI";
            case instr_kind::BNE: return os << "BNE";
            case instr_kind::BPL: return os << "BPL";
            case instr_kind::BRA: return os << "BRA";
            case instr_kind::BVC: return os << "BVC";
            case instr_kind::BVS: return os << "BVS";
            case instr_kind::BIT: return os << "BIT";
            case instr_kind::CLC: return os << "CLC";
            case instr_kind::CLD: return os << "CLD";
            case instr_kind::CLI: return os << "CLI";
            case instr_kind::CLV: return os << "CLV";
            case instr_kind::SEC: return os << "SEC";
            case instr_kind::SED: return os << "SED";
            case instr_kind::SEI: return os << "SEI";
            case instr_kind::CMP: return os << "CMP";
            case instr_kind::CPX: return os << "CPX";
            case instr_kind::CPY: return os << "CPY";
            case instr_kind::DEC: return os << "DEC";
            case instr_kind::INC: return os << "INC";
            case instr_kind::EOR: return os << "EOR";
            case instr_kind::JMP: return os << "JMP";
            case instr_kind::JSR: return os << "JSR";
            case instr_kind::LDA: return os << "LDA";
            case instr_kind::LDX: return os << "LDX";
            case instr_kind::LDY: return os << "LDY";
            case instr_kind::LSR: return os << "LSR";
            case instr_kind::NOP: return os << "NOP";
            case instr_kind::OR: return os << "OR";
            case instr_kind::PHA: return os << "PHA";
            case instr_kind::PHP: return os << "PHP";
            case instr_kind::PHX: return os << "PHX";
            case instr_kind::PLA: return os << "PLA";
            case instr_kind::PLP: return os << "PLP";
            case instr_kind::PLX: return os << "PLX";
            case instr_kind::PLY: return os << "PLY";
            case instr_kind::ROL: return os << "ROL";
            case instr_kind::RTI: return os << "RTI";
            case instr_kind::RTS: return os << "RTS";
            case instr_kind::ROR: return os << "ROR";
            case instr_kind::STA: return os << "STA";
            case instr_kind::STY: return os << "STY";
            case instr_kind::STX: return os << "STX";
            case instr_kind::TAX: return os << "TAX";
            case instr_kind::TXA: return os << "TXA";
            case instr_kind::TAY: return os << "TAY";
            case instr_kind::TYA: return os << "TYA";
            case instr_kind::TSX: return os << "TSX";
            case instr_kind::TXS: return os << "TXS";
            default: return os << "???";
        }
    }

    enum class addressing_mode {
        imm,  // byte: $12
        abs,  // addr: @label
        absx, // addr: @label + #X
        absy, // addr: @label + #Y
        zpg,  // addr: @label (@label is in page zero)
        zpgx, // addr: @label + #X (@label is in page zero)
        zpgy, // addr: @label + #Y (@label is in page zero)
        ind,  // addr: [@label]
        indx, // addr: [@label + #X]
        indy, // addr: [@label] + #Y
        acc,  // byte: #A
        reg_x,// byte: #X
        reg_y // byte: #Y
    };

    inline auto operator << (std::ostream &os, addressing_mode et) -> std::ostream& {
        switch(et) {
            case addressing_mode::imm: return os << "imm";
            case addressing_mode::abs: return os << "abs";
            case addressing_mode::absx: return os << "absx";
            case addressing_mode::absy: return os << "absy";
            case addressing_mode::zpg: return os << "zpg";
            case addressing_mode::zpgx: return os << "zpgx";
            case addressing_mode::zpgy: return os << "zpgy";
            case addressing_mode::ind: return os << "ind";
            case addressing_mode::indx: return os << "indx";
            case addressing_mode::indy: return os << "indy";
            case addressing_mode::acc: return os << "acc";
            case addressing_mode::reg_x: return os << "reg_x";
            case addressing_mode::reg_y: return os << "reg_y";
            default: return os << "unknown";
        }
    }

    typedef std::pair<__int64_t, __int64_t> value_range;

    struct instr {
        std::string _label;
        std::string _code;
        instr_kind _kind;
        std::shared_ptr<expression> _parameter;

        value_range _address_range;
        value_range _instr_size_range;

        REG16 _address;
        size_t _instr_size;

        addressing_mode _addressing_mode;
        std::shared_ptr<expression> _addressing_mode_parameter;
        std::vector<REG8> _binary_representation;
    };

    struct line {
        int _line_no;
        std::string _text;
        std::unique_ptr<instr> _instr;
    };

    struct file {
        std::string filename;
        std::vector<line> lines;
    };
}

#endif
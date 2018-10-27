#include "addressing.h"

#include "ast.h"
#include "logger.h"
#include <unordered_map>

namespace dave
{

/*

X: regx
Y: regy
A: acc
value: imm
address:
    <  256: zpg
    >= 256: abs

address + X:
X + address
    address <  256: zpgx
    address >= 256: absx
    :: zpg + regX = zpgx
    :: regX + zpg = zpgx
    :: abs + regX = absx
    :: regX + abs = absx

address + Y:
Y + address
    address <  256: zpgy
    address >= 256: absy
    :: zpg + regy = zpgY
    :: regY + zpg = zpgY
    :: abs + regy = absY
    :: regY + abs = absY

ind(address): ind
    :: ind(zpg) = ind
ind(address + X): indx
    :: ind(zpgx) = indx
ind(X + address): indx
    :: ind(zpgx) = indx
ind(address) + Y: indy
    :: ind + regy = indy
Y + ind(address): indy
    :: regy + ind = indy
*/

enum class parameter_kind {
    complex,
    regA,
    regX,
    regY,
    value,
    address
};

struct classify_parameter : public const_expression_visitor {
private:
    const std::unordered_map<std::string, value_range> &_labels;
public:
    classify_parameter(const std::unordered_map<std::string, value_range> &labels)
    : _labels(labels)
    {}

    parameter_kind _kind;
    value_range _value;

    virtual void visit(const value_expression* exp) override {
        _kind = parameter_kind::value;
        _value.first = _value.second = exp->value;
    }

    virtual void visit(const label_expression* exp) override {
        _kind = parameter_kind::address;
        auto f = _labels.find(exp->name);
        if (f == _labels.end()) {
            _value.first = 0x0000;
            _value.second = 0xFFFF;
        }
        else {
            _value = f->second;
        }
    }

    virtual void visit(const addr_expression *exp) override {
        exp->value->accept(this);
        if (_kind == parameter_kind::value || _kind == parameter_kind::address) {
            _kind = parameter_kind::address;
        }
        else {
            _kind = parameter_kind::complex;
        }
    }

    virtual void visit(const reg_expression* exp) override {
        switch(exp->reg) {
            case 'A': _kind = parameter_kind::regA; break;
            case 'X': _kind = parameter_kind::regX; break;
            case 'Y': _kind = parameter_kind::regY; break;
            default: _kind = parameter_kind::complex; break;
        }
    }

    virtual void visit(const add_expression* exp) override {
        classify_parameter rhs(_labels);
        exp->rhs->accept(&rhs);
        exp->lhs->accept(this);
        /*          regA    regX    regY    value       address
        regA        X       X       X       X           X
        regX        X       X       X       X           X
        regY        X       X       X       X           X
        value       X       X       X       value       address
        address     X       X       X       address     X

        We asume a single byte value + a single byte value
        stays a single byte value, since the only other option is
        an address in page 1, which
        */
        if (rhs._kind == parameter_kind::value && (_kind == parameter_kind::value || _kind == parameter_kind::address)) {
            // kind stays unchanged
            _value.first += rhs._value.first;
            _value.second += rhs._value.second;
        }
        else if (rhs._kind == parameter_kind::address && _kind == parameter_kind::value) {
            _kind = parameter_kind::address;
            _value.first += rhs._value.first;
            _value.second += rhs._value.second;
        }
        else {
            _kind = parameter_kind::complex;
        }
    }

    virtual void visit(const subtract_expression* exp) override {
        classify_parameter rhs(_labels);
        exp->rhs->accept(&rhs);
        exp->lhs->accept(this);
        /*          regA    regX    regY    value       address
        regA        X       X       X       X           X
        regX        X       X       X       X           X
        regY        X       X       X       X           X
        value       X       X       X       value       X
        address     X       X       X       address     value
        */
        if (rhs._kind == parameter_kind::value && (_kind == parameter_kind::value || _kind == parameter_kind::address)) {
            // _kind stays unchanged
            // (4,8) - (2,3) = (1,6)
            _value.first -= rhs._value.second;
            _value.second -= rhs._value.first;
        }
        else if (rhs._kind == parameter_kind::address && _kind == parameter_kind::address) {
            _kind = parameter_kind::value;
            _value.first -= rhs._value.second;
            _value.second -= rhs._value.first;
            if (_value.first < 0) _value.first = 0;
            if (_value.second < _value.first) _value.second = _value.first;
        }
        else {
            _kind = parameter_kind::complex;
        }
    }

    virtual void visit(const multiply_expression* exp) override {
        classify_parameter rhs(_labels);
        exp->rhs->accept(&rhs);
        exp->lhs->accept(this);
        /*          regA    regX    regY    value       address
        regA        X       X       X       X           X
        regX        X       X       X       X           X
        regY        X       X       X       X           X
        value       X       X       X       value       X
        address     X       X       X       X           X
        */
        if(rhs._kind == parameter_kind::value && _kind == parameter_kind::value) {
            // kind stays unchanged
            // (a,b) where a<b, and (x,y) where x<y
            // which is smallest? a*x, a*y, b*x or b*y
            auto a = _value.first;
            auto b = _value.second;
            _value.first = _value.second = a*rhs._value.first;
            auto x = a*rhs._value.second;
            if (x < _value.first) _value.first = x;
            if (x > _value.second) _value.second = x;
            x = b*rhs._value.first;
            if (x < _value.first) _value.first = x;
            if (x > _value.second) _value.second = x;
            x = b*rhs._value.second;
            if (x < _value.first) _value.first = x;
            if (x > _value.second) _value.second = x;
        }
        else {
            _kind = parameter_kind::complex;
        }
    }

    virtual void visit(const indirect_addr_expression* exp) override {
        _kind = parameter_kind::complex;
    }

    virtual void visit(const func_expression* exp) override {
        // if parameter is value or address, the result is value, else complex
        exp->parameter->accept(this);
        if (_kind == parameter_kind::value || _kind == parameter_kind::address) {
            _kind = parameter_kind::value;
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
        else {
            _kind = parameter_kind::complex;
        }
    }
};

struct classify_complex_parameter : public const_expression_visitor {
private:
    const std::unordered_map<std::string, value_range> &_labels;
    std::shared_ptr<expression> &_value;
public:
    classify_complex_parameter(const std::unordered_map<std::string, value_range> &labels, std::shared_ptr<expression> &value)
    : _labels(labels), _value(value)
    {}

    addressing_mode _mode;
    bool _ok = true;

    virtual void visit(const value_expression* exp) override {
        logger::def->log("Unsupported expression");
        _ok = false;
    }

    virtual void visit(const label_expression* exp) override {
        logger::def->log("Unsupported expression");
        _ok = false;
    }

    virtual void visit(const addr_expression* exp) override {
        logger::def->log("Unsupported expression");
        _ok = false;
    }

    virtual void visit(const reg_expression* exp) override {
        logger::def->log("Unsupported expression");
        _ok = false;
    }

    virtual void visit(const add_expression* exp) override {
        // zpg+X, abs+X, zpg+Y, abs+Y, ind+Y
        addressing_mode lhs, rhs;
        std::shared_ptr<expression> vlhs, vrhs;
        _ok = tryDetermineAddressingMode(_labels, exp->lhs, lhs, vlhs);
        _ok = tryDetermineAddressingMode(_labels, exp->rhs, rhs, vrhs);
        if (_ok) {
            switch(lhs) {
                case addressing_mode::zpg: // zpg+X or zpg+Y
                    _value = vlhs;
                    switch(rhs) {
                        case addressing_mode::reg_x:
                            _mode = addressing_mode::zpgx;
                            break;
                        case addressing_mode::reg_y:
                            _mode = addressing_mode::zpgy;
                            break;
                        default:
                            logger::def->log("Unsupported additive addressing mode");
                            _ok = false;
                            break;
                    }
                    break;
                case addressing_mode::abs: // abs+X or abs+Y
                    _value = vlhs;
                    switch(rhs) {
                        case addressing_mode::reg_x:
                            _mode = addressing_mode::absx;
                            break;
                        case addressing_mode::reg_y:
                            _mode = addressing_mode::absy;
                            break;
                        default:
                            logger::def->log("Unsupported additive addressing mode");
                            _ok = false;
                            break;
                    }
                    break;
                case addressing_mode::ind: // ind+Y
                    _value = vlhs;
                    switch(rhs) {
                        case addressing_mode::reg_y:
                            _mode = addressing_mode::indy;
                            break;
                        default:
                            logger::def->log("Unsupported additive addressing mode");
                            _ok = false;
                            break;
                    }
                    break;
                case addressing_mode::reg_x: // X+zpg or X+abs
                    _value = vrhs;
                    switch(rhs) {
                        case addressing_mode::zpg:
                            _mode = addressing_mode::zpgx;
                            break;
                        case addressing_mode::abs:
                            _mode = addressing_mode::absx;
                            break;
                        default:
                            logger::def->log("Unsupported additive addressing mode");
                            _ok = false;
                            break;
                    }
                    break;
                case addressing_mode::reg_y: // Y+zpg or Y+abs or Y+ind
                    _value = vrhs;
                    switch(rhs) {
                        case addressing_mode::zpg:
                            _mode = addressing_mode::zpgy;
                            break;
                        case addressing_mode::abs:
                            _mode = addressing_mode::absy;
                            break;
                        case addressing_mode::ind:
                            _mode = addressing_mode::indy;
                            break;
                        default:
                            logger::def->log("Unsupported additive addressing mode");
                            _ok = false;
                            break;
                    }
                    break;
                default:
                    logger::def->log("Unsupported additive addressing mode");
                    _ok = false;
                    break;
            }
        }
    }

    virtual void visit(const subtract_expression* exp) override {
        logger::def->log("Unsupported expression");
        _ok = false;
    }

    virtual void visit(const multiply_expression* exp) override {
        logger::def->log("Unsupported expression");
        _ok = false;
    }

    virtual void visit(const indirect_addr_expression* exp) override {
        addressing_mode mode;
        _ok = tryDetermineAddressingMode(_labels, exp->addr, mode, _value);
        if (_ok) {
            switch(mode) {
                case addressing_mode::zpg:
                    // [$12]
                    _mode = addressing_mode::ind;
                    break;
                case addressing_mode::zpgx:
                    // [$12 + #X]
                    _mode = addressing_mode::indx;
                    break;
                default:
                    logger::def->log("Unsupported indirect addressing.");
                    _ok = false;
                    break;
            }
        }
    }

    virtual void visit(const func_expression* exp) override {
        logger::def->log("Unsupported expression");
        _ok = false;
    }
};

inline void log_out_of_range(const value_range &value, const REG16 &lo, const REG16 &hi, const std::string &addressing) {
    if (value.first < lo) {
        logger::def->log() << "The value (" << value.first << ") for " << addressing << " addressing is out-of-range" << end();
    }
    else {
        logger::def->log() << "The value (" << value.second << ") for " << addressing << " addressing is out-of-range" << end();
    }
}

bool tryDetermineAddressingMode(const std::unordered_map<std::string, value_range> &labels, const std::shared_ptr<expression> &parameter, addressing_mode &mode, std::shared_ptr<expression> &value)
{
    classify_parameter cp(labels);
    parameter->accept(&cp);
    switch(cp._kind) {
        case parameter_kind::complex:
            if (true) {
                classify_complex_parameter ccp(labels, value);
                parameter->accept(&ccp);
                mode = ccp._mode;
                return ccp._ok;
            }
            break;
        case parameter_kind::value:
            if (cp._value.first < 0 || cp._value.second > 255) {
                log_out_of_range(cp._value, 0, 255, "immediate");
                return false;
            }
            else {
                mode = addressing_mode::imm;
                value = parameter;
                return true;
            }
        case parameter_kind::regA:
            mode = addressing_mode::acc;
            return true;
        case parameter_kind::regX:
            mode = addressing_mode::reg_x;
            return true;
        case parameter_kind::regY:
            mode = addressing_mode::reg_y;
            return true;
        case parameter_kind::address:
            if (cp._value.first < 0 || cp._value.second > 0xFFFF) {
                log_out_of_range(cp._value, 0, 0xFFFF, "absolute");
                return false;
            }
            else if (cp._value.first < 256 && cp._value.second >= 256) {
                logger::def->log() << "Unable to determine whether an address is in page zero. The value span the address space " << cp._value.first << " to " << cp._value.second << end();
                return false;
            }
            else if (cp._value.second < 256) {
                mode = addressing_mode::zpg;
            }
            else {
                mode = addressing_mode::abs;
            }
            value = parameter;
            return true;
        default:
            break;
    }
    logger::def->log("Unsupported expression");
    return false;
}

}
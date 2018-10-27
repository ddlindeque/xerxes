#include "parser.h"

#include "ast.h"
#include "lexer.h"
#include "logger.h"

#include <unordered_map>
#include <fstream>

namespace dave
{

typedef std::unordered_map<std::string, std::shared_ptr<expression> > constants_t;

struct instr_traits {
    instr_kind kind;
    bool has_parameter;
};

bool try_get_instr_traits(const std::string &code, instr_traits &t)
{
    static std::unordered_map<std::string, instr_traits> itraits;
    if (itraits.empty()) {
        itraits.emplace("BASE", instr_traits { instr_kind::BASE, true });
        itraits.emplace("START", instr_traits { instr_kind::START, true });
        itraits.emplace("DATA", instr_traits { instr_kind::DATA, true });
        itraits.emplace("BRK", instr_traits { instr_kind::BRK, false });
        itraits.emplace("ADC", instr_traits { instr_kind::ADC, true });
        itraits.emplace("SBC", instr_traits { instr_kind::SBC, true });
        itraits.emplace("AND", instr_traits { instr_kind::AND, true });
        itraits.emplace("ASL", instr_traits { instr_kind::ASL, true });
        itraits.emplace("BCC", instr_traits { instr_kind::BCC, true });
        itraits.emplace("BCS", instr_traits { instr_kind::BCS, true });
        itraits.emplace("BEQ", instr_traits { instr_kind::BEQ, true });
        itraits.emplace("BMI", instr_traits { instr_kind::BMI, true });
        itraits.emplace("BNE", instr_traits { instr_kind::BNE, true });
        itraits.emplace("BPL", instr_traits { instr_kind::BPL, true });
        itraits.emplace("BRA", instr_traits { instr_kind::BRA, true });
        itraits.emplace("BVC", instr_traits { instr_kind::BVC, true });
        itraits.emplace("BVS", instr_traits { instr_kind::BVS, true });
        itraits.emplace("BIT", instr_traits { instr_kind::BIT, true });
        itraits.emplace("CLC", instr_traits { instr_kind::CLC, false });
        itraits.emplace("CLD", instr_traits { instr_kind::CLD, false });
        itraits.emplace("CLI", instr_traits { instr_kind::CLI, false });
        itraits.emplace("CLV", instr_traits { instr_kind::CLV, false });
        itraits.emplace("SEC", instr_traits { instr_kind::SEC, false });
        itraits.emplace("SED", instr_traits { instr_kind::SED, false });
        itraits.emplace("SEI", instr_traits { instr_kind::SEI, false });
        itraits.emplace("CMP", instr_traits { instr_kind::CMP, true });
        itraits.emplace("CPX", instr_traits { instr_kind::CPX, true });
        itraits.emplace("CPY", instr_traits { instr_kind::CPY, true });
        itraits.emplace("DEC", instr_traits { instr_kind::DEC, true });
        itraits.emplace("INC", instr_traits { instr_kind::INC, true });
        itraits.emplace("EOR", instr_traits { instr_kind::EOR, true });
        itraits.emplace("JMP", instr_traits { instr_kind::JMP, true });
        itraits.emplace("JSR", instr_traits { instr_kind::JSR, true });
        itraits.emplace("LDA", instr_traits { instr_kind::LDA, true });
        itraits.emplace("LDX", instr_traits { instr_kind::LDX, true });
        itraits.emplace("LDY", instr_traits { instr_kind::LDY, true });
        itraits.emplace("LSR", instr_traits { instr_kind::LSR, true });
        itraits.emplace("NOP", instr_traits { instr_kind::NOP, false });
        itraits.emplace("OR", instr_traits { instr_kind::OR, true });
        itraits.emplace("PHA", instr_traits { instr_kind::PHA, false });
        itraits.emplace("PHP", instr_traits { instr_kind::PHP, false });
        itraits.emplace("PHX", instr_traits { instr_kind::PHX, false });
        itraits.emplace("PLA", instr_traits { instr_kind::PLA, false });
        itraits.emplace("PLP", instr_traits { instr_kind::PLP, false });
        itraits.emplace("PLX", instr_traits { instr_kind::PLX, false });
        itraits.emplace("PLY", instr_traits { instr_kind::PLY, false });
        itraits.emplace("ROL", instr_traits { instr_kind::ROL, true });
        itraits.emplace("RTI", instr_traits { instr_kind::RTI, false });
        itraits.emplace("RTS", instr_traits { instr_kind::RTS, false });
        itraits.emplace("ROR", instr_traits { instr_kind::ROR, true });
        itraits.emplace("STA", instr_traits { instr_kind::STA, true });
        itraits.emplace("STY", instr_traits { instr_kind::STY, true });
        itraits.emplace("STX", instr_traits { instr_kind::STX, true });
        itraits.emplace("TAX", instr_traits { instr_kind::TAX, false });
        itraits.emplace("TXA", instr_traits { instr_kind::TXA, false });
        itraits.emplace("TAY", instr_traits { instr_kind::TAY, false });
        itraits.emplace("TYA", instr_traits { instr_kind::TYA, false });
        itraits.emplace("TSX", instr_traits { instr_kind::TSX, false });
        itraits.emplace("TXS", instr_traits { instr_kind::TXS, false });
    }
    auto f = itraits.find(code);
    if (f == itraits.end()) {
        return false;
    }
    else {
        t = f->second;
        return true;
    }
};

bool tryParseExpression(std::vector<token>::const_iterator &it, const std::vector<token>::const_iterator &end, std::shared_ptr<expression> &expr, const constants_t &constants);

bool tryParseSimpleExpression(std::vector<token>::const_iterator &it, const std::vector<token>::const_iterator &end, std::shared_ptr<expression> &expr, const constants_t &constants)
{
    if (it == end) {
        return false;
    }
    auto tkn = *it;
    switch(tkn.kind) {
        case token_kind::tk_hash:
            ++it;
            if (!tryParseExpression(it, end, expr, constants)) return false;
            expr.reset(new addr_expression(expr));
            break;
        case token_kind::tk_lit_value:
            expr.reset(new value_expression(it->_w));
            ++it;
            break;
        case token_kind::tk_constant:
            // Find the value for the expression
            if (true) {
                auto f = constants.find(it->_t);
                if (f == constants.end()) {
                    logger::def->log() << "The constant '" << it->_t << "' was not defined" << dave::end();
                    return false;
                }
                else {
                    expr = f->second;
                }
            }
            ++it;
            break;
        case token_kind::tk_label:
            expr.reset(new label_expression(it->_t));
            ++it;
            break;
        case token_kind::tk_regA:
            expr.reset(new reg_expression('A'));
            ++it;
            break;
        case token_kind::tk_regX:
            expr.reset(new reg_expression('X'));
            ++it;
            break;
        case token_kind::tk_regY:
            expr.reset(new reg_expression('Y'));
            ++it;
            break;
        case token_kind::tk_identifier:
            if (true) {
                std::string name = it->_t;
                if (name != "lo" && name != "hi") {
                    logger::def->log() << "Unsupported function '" << name << "'" << dave::end();
                    return false;
                }
                ++it;
                if (!tryParseExpression(it, end, expr, constants)) return false;
                expr.reset(new func_expression(name, expr));
            }
            break;
        case token_kind::tk_open_paren:
            ++it;
            if (it == end) {
                logger::def->log("Unexpected end-of-line");
                return false;
            }
            // Precedence
            if (!tryParseExpression(it, end, expr, constants)) return false;
            if (it == end || it->kind != token_kind::tk_close_paren) {
                logger::def->log("Expected a ')'");
                return false;
            }
            ++it;
            break;
        case token_kind::tk_open_bracket:
            ++it;
            if (it == end) {
                logger::def->log("Unexpected end-of-line");
                return false;
            }
            if (!tryParseExpression(it, end, expr, constants)) return false;
            if (it == end || it->kind != token_kind::tk_close_bracket) {
                logger::def->log("Expected a ']'");
                return false;
            }
            ++it;
            expr.reset(new indirect_addr_expression(expr));
            break;
        default:
            logger::def->log() << "Syntax error. Unexpected token '" << tkn << '\'' << dave::end();
            return false;
    }

    return true;
}

bool tryParseExpression(std::vector<token>::const_iterator &it, const std::vector<token>::const_iterator &end, std::shared_ptr<expression> &expr, const constants_t &constants)
{
    // We've just read an expression, now check the follow
    // We can have
    // [exp] + <exp> + : reduce
    // [exp] + <exp> - : reduce
    // [exp] + <exp> * : shift
    // [exp] - <exp> + : reduce
    // [exp] - <exp> - : reduce
    // [exp] - <exp> * : shift
    // [exp] * <exp> + : reduce
    // [exp] * <exp> - : reduce
    // [exp] * <exp> * : reduce

    if (!tryParseSimpleExpression(it, end, expr, constants)) return false;
    // [exp] ?
    //       ^
    while(it != end) {
        std::shared_ptr<expression> rhs;
        auto tkn = *it;
        switch(tkn.kind) {
            case token_kind::tk_plus:
                // [exp] +
                //       ^
                ++it;
                // [exp] + <exp>
                //         ^
                if (it == end) {
                    logger::def->log("Unexpected end-of-line");
                    return false;
                }
                if (!tryParseSimpleExpression(it, end, rhs, constants)) return false;
                // [exp] + <exp> *
                //               ^
                while(it != end && it->kind == token_kind::tk_asterisk) {
                    ++it;
                    // [exp] + <exp> * <x>
                    //                 ^
                    std::shared_ptr<expression> x;
                    if (!tryParseSimpleExpression(it, end, x, constants)) return false;
                    // [exp] + <exp> * <x> ?
                    //                     ^
                    rhs.reset(new multiply_expression(rhs, x));
                }
                expr.reset(new add_expression(expr, rhs));
                break;
            case token_kind::tk_minus:
                // [exp] -
                //       ^
                ++it;
                // [exp] - <exp>
                //         ^
                if (it == end) {
                    logger::def->log("Unexpected end-of-line");
                    return false;
                }
                if (!tryParseSimpleExpression(it, end, rhs, constants)) return false;
                // [exp] - <exp> *
                //               ^
                while(it != end && it->kind == token_kind::tk_asterisk) {
                    ++it;
                    // [exp] - <exp> * <x>
                    //                 ^
                    std::shared_ptr<expression> x;
                    if (!tryParseSimpleExpression(it, end, x, constants)) return false;
                    // [exp] - <exp> * <x> ?
                    //                     ^
                    rhs.reset(new multiply_expression(rhs, x));
                }
                expr.reset(new subtract_expression(expr, rhs));
                break;
            case token_kind::tk_asterisk:
                // [exp] *
                //       ^
                ++it;
                // [exp] * <exp>
                //         ^
                if (it == end) {
                    logger::def->log("Unexpected end-of-line");
                    return false;
                }
                if (!tryParseSimpleExpression(it, end, rhs, constants)) return false;
                expr.reset(new multiply_expression(expr, rhs));
                break;
            case token_kind::tk_close_bracket:
            case token_kind::tk_close_paren:
                return true;
            default:
                logger::def->log() << "Syntax error. Unexpected token '" << tkn << '\'' << dave::end();
                return false;
        }
    }

    return true;
}

bool tryParseConstAssign(std::vector<token>::const_iterator &it, const std::vector<token>::const_iterator &end, constants_t &constants)
{
    // 'it' is pointing at the constant name
    std::string name;
    name = it->_t;

    ++it;
    if (it == end) {
        logger::def->log("Unexpected end-of-line");
        return false;
    }
    else if (it->kind != token_kind::tk_eq) {
        logger::def->log("Expected an '='");
        return false;
    }
    ++it;
    if (it == end) {
        logger::def->log("Unexpected end-of-line");
        return false;
    }
    else {
        std::shared_ptr<expression> exp;
        if (!tryParseExpression(it, end, exp, constants)) {
            return false;
        }

        // Add to constants map
        constants.emplace(name, std::move(exp));

        return true;
    }
}

bool tryParseInstr(std::vector<token>::const_iterator &it, const std::vector<token>::const_iterator &end, std::unique_ptr<instr> &instr, const constants_t &constants)
{
    // 'it' is pointing to the instruction
    instr.reset(new dave::instr());
    instr->_code = it->_t;
    // Decode the code
    instr_traits itraits;
    if (!try_get_instr_traits(instr->_code, itraits)) {
        logger::def->log() << " - The instruction '" << instr->_code << "' was not recognised" << dave::end();
        return false;
    }
    instr->_kind = itraits.kind;
    
    ++it;
    if (it == end) {
        if (itraits.has_parameter) {
            logger::def->log() << " - The instruction '" << instr->_code << "' requires a parameter" << dave::end();
            return false;
        }
        return true;
    }
    else {
        if (!itraits.has_parameter) {
            logger::def->log() << " - The instruction '" << instr->_code << "' does not expect a parameter" << dave::end();
            return false;
        }
        if (!tryParseExpression(it, end, instr->_parameter, constants)) {
            return false;
        }

        return true;
    }
}

bool tryParseInstruction(std::vector<token>::const_iterator &it, const std::vector<token>::const_iterator &end, std::unique_ptr<instr> &instr, constants_t &constants, std::string &current_label)
{
    if (it == end) {
        return true;
    }
    bool ok = true;
    auto tkn = *it;
    switch(tkn.kind) {
        case token_kind::tk_label:
            current_label = it->_t;
            ++it;
            if (it == end || it->kind != token_kind::tk_colon) {
                logger::def->log("Expected ':'");
                return false;
            }
            ++it;
            if (it != end) {
                ok = tryParseInstruction(it, end, instr, constants, current_label);
            }
            break;
        case token_kind::tk_constant:
            ok = tryParseConstAssign(it, end, constants);
            break;
        case token_kind::tk_identifier:
            ok = tryParseInstr(it, end, instr, constants);
            instr->_label = current_label;
            current_label.clear();
            break;
        default:
            logger::def->log() << " - Unexpected token '" << tkn << "'. Expected 'label', 'type' or 'instruction'." << dave::end();
            return false;
    }

    return ok;
}

bool tryParse(file &f)
{
    logger::def->_filename = f.filename;

    int line_no = 0;
    char line[1024];

    std::string current_label;
    constants_t constants;

    std::ifstream source(f.filename);
    if (!source) {
        logger::def->log("Unable to open the file for input");
        return false;
    }

    while(!source.eof()) {
        line_no++;
        logger::def->_line_no = line_no;
        char line[1024];
        source.getline(line, 1023);
        f.lines.emplace_back();
        f.lines.back()._line_no = line_no;
        f.lines.back()._text = line;
        std::vector<token> tokens;
        if (!tryLexicalAnalysis(f.lines.back()._text, tokens)) {
            return false;
        }
        if (tokens.empty()) continue;
        auto it = tokens.cbegin();
        if (!tryParseInstruction(it, tokens.cend(), f.lines.back()._instr, constants, current_label)) {
            return false;
        }
    }
    return true;
}

}
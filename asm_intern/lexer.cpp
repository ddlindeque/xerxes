#include "lexer.h"

#include "logger.h"

namespace dave
{

auto operator << (std::ostream &os, const token &tkn) -> std::ostream&
{
    switch(tkn.kind) {
        case token_kind::tk_constant: return os << "const(" << tkn._t << ")";
        case token_kind::tk_eq: return os << "=";
        case token_kind::tk_identifier: return os << "identifier(" << tkn._t << ")";
        case token_kind::tk_label: return os << "label(" << tkn._t << ")";
        case token_kind::tk_colon: return os << ":";
        case token_kind::tk_lit_value: return os << "value(" << (unsigned int)tkn._w << ")";
        case token_kind::tk_regA: return os << "regA";
        case token_kind::tk_regX: return os << "regX";
        case token_kind::tk_regY: return os << "regY";
        case token_kind::tk_open_paren: return os << "(";
        case token_kind::tk_close_paren: return os << ")";
        case token_kind::tk_plus: return os << "+";
        case token_kind::tk_minus: return os << "-";
        case token_kind::tk_asterisk: return os << "*";
        case token_kind::tk_open_bracket: return os << '[';
        case token_kind::tk_close_bracket: return os << ']';
        case token_kind::tk_hash: return os << '#';
    }
}

void readIdentifier(const char **p, std::string &res) {
    while (**p == '_' || (**p >= 'a' && **p <= 'z') || (**p >= 'A' && **p <= 'Z') || (**p >= '0' && **p <= '9')) {
        res += **p;
        (*p)++;
    }
}

bool tryReadHexLit(const char **p, token &tkn)
{
    std::string chars;
    while((**p >= '0' && **p <= '9') || (**p >= 'a' && **p <= 'f') || (**p >= 'A' && **p <= 'F')) {
        chars.push_back(**p);
        (*p)++;
    }
    if(chars.size() == 2) {
        tkn.kind = token_kind::tk_lit_value;
        tkn._w = std::strtoul(chars.c_str(), 0, 16);
    }
    else if (chars.size() == 4) {
        tkn.kind = token_kind::tk_lit_value;
        tkn._w = std::strtoul(chars.c_str(), 0, 16);
    }
    else {
        logger::def->log("Invalid size of hex literal. Expected 2 or 4 characters");
        return false;
    }
    return true;
}

bool tryLexicalAnalysis(const std::string &text, std::vector<token> &tokens)
{
    const char *p = text.c_str();
    while(*p != 0) {
        if (*p == ';') break;
        switch(*p) {
            case ' ':
            case '\t':
                p++;
                break;
            case '=':
                p++;
                tokens.push_back(token { token_kind::tk_eq });
                break;
            case ':':
                p++;
                tokens.push_back(token { token_kind::tk_colon });
                break;
            case '(':
                p++;
                tokens.push_back(token { token_kind::tk_open_paren });
                break;
            case ')':
                p++;
                tokens.push_back(token { token_kind::tk_close_paren });
                break;
            case '[':
                p++;
                tokens.push_back(token { token_kind::tk_open_bracket });
                break;
            case ']':
                p++;
                tokens.push_back(token { token_kind::tk_close_bracket });
                break;
            case '+':
                p++;
                tokens.push_back(token { token_kind::tk_plus } );
                break;
            case '-':
                p++;
                tokens.push_back(token { token_kind::tk_minus });
                break;
            case '*':
                p++;
                tokens.push_back(token { token_kind::tk_asterisk });
                break;
            case '%':
                if (true) {
                    p++;
                    token tkn;
                    tkn.kind = token_kind::tk_constant;
                    readIdentifier(&p, tkn._t);
                    tokens.push_back(std::move(tkn));
                }
                break;
            case '@':
                if (true) {
                    p++;
                    token tkn;
                    tkn.kind = token_kind::tk_label;
                    readIdentifier(&p, tkn._t);
                    tokens.push_back(std::move(tkn));
                }
                break;
            case '$':
                if (true) {
                    p++;
                    tokens.emplace_back();
                    if (!tryReadHexLit(&p, tokens.back())) {
                        return false;
                    }
                }
                break;
            case '#':
                p++;
                tokens.emplace_back();
                tokens.back().kind = token_kind::tk_hash;
                break;
            default:
                if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || *p == '_') {
                    tokens.emplace_back();
                    readIdentifier(&p, tokens.back()._t);
                    // identifier or keyword
                    if (tokens.back()._t == "A") {
                        tokens.back().kind = token_kind::tk_regA;
                    }
                    else if (tokens.back()._t == "X") {
                        tokens.back().kind = token_kind::tk_regX;
                    }
                    else if (tokens.back()._t == "Y") {
                        tokens.back().kind = token_kind::tk_regY;
                    }
                    else {
                        tokens.back().kind = token_kind::tk_identifier;
                    }
                }
                else {
                    logger::def->log() << "Syntax error. Unexpected character '" << *p << "'." << end();
                    return false;
                }
        }
    }
    return true;
}

}
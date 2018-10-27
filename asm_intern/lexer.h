#ifndef __LEXERH
#define __LEXERH

#include <string>
#include <vector>
#include <ostream>
#include "../xerxes_lib/common.h"

namespace dave
{
    enum class token_kind
    {
        tk_constant, // %..
        tk_eq, // =
        tk_identifier, // identfier
        tk_label, // @..
        tk_colon, // :
        tk_lit_value, // $????
        tk_regA, // A
        tk_regX, // X
        tk_regY, // Y
        tk_open_paren, // (
        tk_close_paren, // )
        tk_plus, // +
        tk_minus, // -
        tk_asterisk, // *
        tk_open_bracket, // [
        tk_close_bracket, // ]
        tk_hash // #
    };

    struct token {
        token_kind kind;
        std::string _t;
        REG16 _w;
    };

    auto operator << (std::ostream &os, const token &tkn) -> std::ostream&;

    bool tryLexicalAnalysis(const std::string &text, std::vector<token> &tokens);
}

#endif
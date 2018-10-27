#ifndef __CODE_GENERATORH
#define __CODE_GENERATORH

#include "ast.h"
#include <vector>

namespace dave
{
    class code_generator {
    public:
        code_generator() = default;
        code_generator(const code_generator&) = delete;
        code_generator(code_generator&&) = delete;

        virtual ~code_generator();

        auto operator =(const code_generator&) -> code_generator& = delete;
        auto operator =(code_generator&&) -> code_generator& = delete;

        virtual bool try_generate(const std::vector<file> &files, const REG16 startAddress) = 0;
    };
}

#endif
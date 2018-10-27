#ifndef __ROM_CODE_GENERATORH
#define __ROM_CODE_GENERATORH

#include "code_generator.h"
#include <ostream>

namespace dave
{
    class rom_code_generator : public code_generator {
    private:
        std::ostream &_output;
    public:
        rom_code_generator() = delete;
        rom_code_generator(const rom_code_generator&) = delete;
        rom_code_generator(rom_code_generator&&) = delete;
        explicit rom_code_generator(std::ostream &output);

        virtual ~rom_code_generator();

        auto operator =(const rom_code_generator&) -> rom_code_generator& = delete;
        auto operator =(rom_code_generator&&) -> rom_code_generator& = delete;

        virtual bool try_generate(const std::vector<file> &files, const REG16 startAddress) override;
    };
}

#endif
#ifndef __PC_CODE_GENERATORH
#define __PC_CODE_GENERATORH

#include "code_generator.h"
#include <ostream>

namespace dave
{
    class pc_code_generator : public code_generator {
    private:
        std::ostream &_output;
    public:
        pc_code_generator() = delete;
        pc_code_generator(const pc_code_generator&) = delete;
        pc_code_generator(pc_code_generator&&) = delete;
        explicit pc_code_generator(std::ostream &output);

        virtual ~pc_code_generator();

        auto operator =(const pc_code_generator&) -> pc_code_generator& = delete;
        auto operator =(pc_code_generator&&) -> pc_code_generator& = delete;

        virtual bool try_generate(const std::vector<file> &files, const REG16 startAddress) override;
    };
}

#endif
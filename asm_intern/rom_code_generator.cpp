#include "rom_code_generator.h"

#include "logger.h"
#include <iomanip>

namespace dave
{

rom_code_generator::rom_code_generator(std::ostream &output)
: _output(output)
{
}

rom_code_generator::~rom_code_generator()
{}

bool rom_code_generator::try_generate(const std::vector<file> &files, const REG16 startAddress)
{
    _output << "template<typename TRom> void initialize_kernel_rom(TRom *rom)" << std::endl;
    _output << '{' << std::endl;
    for(auto &f : files) {
        _output << "    // " << f.filename << std::endl;
        for(auto &l : f.lines) {
            _output << "    // " << std::dec << l._line_no << ": " << l._text << std::endl;
            if (l._instr != nullptr) {
                REG16 addr = l._instr->_address;
                for(auto &i : l._instr->_binary_representation) {
                    _output << "    rom->program(0x" << std::setfill('0') << std::hex << std::uppercase << std::setw(4) << addr << ",0x" << std::setfill('0') << std::hex << std::uppercase << std::setw(2) << (unsigned int)i << ");" << std::endl;
                    addr++;
                }
            }
        }
    }
    _output << '}' << std::endl;
    return true;
}

}
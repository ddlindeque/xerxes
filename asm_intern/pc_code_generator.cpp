#include "pc_code_generator.h"

#include "logger.h"
#include <iomanip>

namespace dave
{

pc_code_generator::pc_code_generator(std::ostream &output)
: _output(output)
{
}

pc_code_generator::~pc_code_generator()
{}

void write_byte(std::ostream &output, REG8 byte, REG16 addr) {
    output << ' ' << ((byte & 0x80) == 0 ? '_' : 'O');
    output << ' ' << ((byte & 0x40) == 0 ? '_' : 'O');
    output << ' ' << ((byte & 0x20) == 0 ? '_' : 'O');
    output << ' ' << ((byte & 0x10) == 0 ? '_' : 'O');
    output << "  " << ((byte & 0x08) == 0 ? '_' : 'O');
    output << ' ' << ((byte & 0x04) == 0 ? '_' : 'O');
    output << ' ' << ((byte & 0x02) == 0 ? '_' : 'O');
    output << ' ' << ((byte & 0x01) == 0 ? '_' : 'O');
    output << " ; ";
    if (addr != 0) {
        output << " ; 0x" << std::setfill('0') << std::hex << std::uppercase << std::setw(4) << (int)addr;
    }
    output << " ; 0x" << std::setfill('0') << std::hex << std::uppercase << std::setw(2) << (int)byte;
}

bool pc_code_generator::try_generate(const std::vector<file> &files, const REG16 startAddress)
{
    REG16 cur = 0;
    for(auto &f : files) {
        _output << "; " << f.filename << std::endl;
        for(auto &l : f.lines) {
            _output << "; " << std::dec << l._line_no << ": " << l._text << std::endl;
            if (l._instr != nullptr) {
                REG16 addr = l._instr->_address;
                if (addr != cur) {
                    _output << "; Change address to 0x" << std::setfill('0') << std::hex << std::uppercase << std::setw(4) << addr << std::endl;
                    _output << "O _ ";
                    write_byte(_output, addr & 0xFF, 0);
                    _output << std::endl;
                    _output << "O O ";
                    write_byte(_output, addr >> 8, 0);
                    _output << std::endl;
                }
                for(auto &i : l._instr->_binary_representation) {
                    _output << "_ O ";
                    write_byte(_output, i, addr);
                    _output << std::endl;
                    addr++;
                }
                cur = addr;
            }
        }
    }
    // Goto start address
    _output << "; Execute start address" << std::endl;
    _output << "O _ ";
    write_byte(_output, startAddress & 0xFF, 0);
    _output << std::endl;
    _output << "O O ";
    write_byte(_output, startAddress >> 8, 0);
    _output << std::endl;
    return true;
}

}
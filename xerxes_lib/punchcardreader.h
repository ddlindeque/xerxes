#ifndef __PUNCHCARDREADERH
#define __PUNCHCARDREADERH

#include "common.h"
#include "device.h"
#include "system_bus.h"

#include <fstream>
#include <random>

namespace dave
{
    template<REG16 _Control, REG16 _Status, REG16 _Register> class punchcardreader : public device {
    public:
        enum class instruction {
            no_instruction = 0,
            read_data = 1,
            read_lo_addr = 2,
            read_hi_addr = 3,
            run_program = 4 
        };
    private:
        bool _irq;
        size_t _ticks_to_interupt;
        
        REG8 _status;
        REG8 _register;

        size_t _next_line;
        std::vector<std::pair<REG8, REG8> > _card;
    public:
        punchcardreader(system_bus *bus, debugger *debugger, const std::string &datafn)
            : device(bus, debugger), _status(0), _ticks_to_interupt(0), _irq(false), _next_line(0)
        {
            // Read the file
            std::ifstream stm(datafn);
            _card.reserve(16);
            while(!stm.eof()) {
                char line[1024];
                stm.getline(line, 1023);
                line[strcspn(line, ";")] = 0;
                std::vector<bool> bytes;
                const char *p = line;
                while(*p != 0) {
                    if (*p == ' ' || *p == '\t') {
                        p++;
                    }
                    else if (*p == '_') {
                        bytes.push_back(false);
                        p++;
                    }
                    else if (*p == 'O') {
                        bytes.push_back(true);
                        p++;
                    }
                    else {
                        bytes.clear();
                        break;
                    }
                }
                if (bytes.size() == 10) {
                    _card.emplace_back(0, 0);
                    for(size_t i = 0; i < 2; i++) {
                        _card.back().first <<= 1;
                        _card.back().first |= (bytes[i] ? 1 : 0);
                    }
                    for(size_t i = 2; i < 10; i++) {
                        _card.back().second <<= 1;
                        _card.back().second |= (bytes[i] ? 1 : 0);
                    }
                }
            }
            _status = _card.empty() ? (REG8)instruction::no_instruction : _card[0].first;
            _register = _card.empty() ? 0 : _card[0].second;
        }
        punchcardreader() = delete;
        punchcardreader(const punchcardreader&) = delete;
        punchcardreader(punchcardreader &&) = delete;
        auto operator =(const punchcardreader&)->punchcardreader& = delete;
        auto operator =(punchcardreader &&)->punchcardreader& = delete;

        virtual bool irq() override { return _irq; }
        virtual void tick() override {
            if (_ticks_to_interupt > 0) {
                _ticks_to_interupt--;
                if (_ticks_to_interupt == 0) {
                    // Interupt
                    _irq = true;
                }
            }
            _debugger->report_punchcardreader_status(_irq, _ticks_to_interupt > 0, _status, _register);
        }
        virtual void nop() override { }
        virtual void write(const REG16 &address, const REG8 *data) override {
            if (address == _Control) {
                _irq = false;
                _status = 0;
                switch(*data) {
                    case 0x01: // Initialise
                        _next_line = 0;
                        break;
                    case 0x02: // Request next instruction
                        if (_next_line == _card.size()) {
                            _status = (REG8)instruction::run_program;
                            _ticks_to_interupt = (rand() % 300) + 200;
                        }
                        else {
                            _register = _card[_next_line].second;
                            _status = _card[_next_line].first;
                            _next_line++;
                            _ticks_to_interupt = (rand() % 300) + 200;
                        }
                        break;
                }
                _debugger->report_punchcardreader_status(_irq, _ticks_to_interupt > 0, _status, _register);
            }
        }
        virtual void read(const REG16 &address, REG8 *dest) override {
            switch(address) {
                case _Status:
                    _irq = false;
                    *dest = _status;
                    _status = 0;
                    break;
                case _Register:
                    _irq = false;
                    _status = 0;
                    *dest = _register;
                    break;
            } 
        }
    };
}


#endif
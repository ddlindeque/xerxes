#ifndef __ROMH
#define __ROMH

#include "common.h"
#include "device.h"
#include "system_bus.h"

namespace dave
{
    template<REG16 addr_lower, REG16 addr_upper> class rom : public device {
    private:
        REG8 _data[addr_upper - addr_lower + 1] = {};
    public:
        rom(system_bus *bus, debugger *debugger)
            : device(bus, debugger)
        {}
        rom() = delete;
        rom(const rom&) = delete;
        rom(rom &&) = delete;
        auto operator =(const rom&)->rom& = delete;
        auto operator =(rom &&)->rom& = delete;

        virtual void nop() override { }
        virtual void write(const REG16 &address, const REG8 *data) override {
            // The ROM does not respond to writes
        }
        virtual void read(const REG16 &address, REG8 *dest) override {
            if (address >= addr_lower && address <= addr_upper) {
                *dest = _data[address - addr_lower];
            }
        }
        void program(const REG16 &address, const REG8 &data) {
            if (address >= addr_lower && address <= addr_upper) {
                _data[address - addr_lower] = data;
            }
        }
    };
}

#endif
#ifndef __RAMH
#define __RAMH

#include "common.h"
#include "device.h"
#include "system_bus.h"

namespace dave
{
    template<REG16 addr_lower, REG16 addr_upper> class ram : public device {
    private:
        REG8 _data[addr_upper - addr_lower + 1] = {};
    public:
        ram(system_bus *bus, debugger *debugger)
            : device(bus, debugger)
        {}
        ram() = delete;
        ram(const ram&) = delete;
        ram(ram &&) = delete;
        auto operator =(const ram&)->ram& = delete;
        auto operator =(ram &&)->ram& = delete;

        virtual void nop() override { }
        virtual void write(const REG16 &address, const REG8 *data) override {
            if (address >= addr_lower && address <= addr_upper) {
                _data[address - addr_lower] = *data;
            }
        }
        virtual void read(const REG16 &address, REG8 *dest) override {
            if (address >= addr_lower && address <= addr_upper) {
                *dest = _data[address - addr_lower];
            }
        }
    };
}

#endif
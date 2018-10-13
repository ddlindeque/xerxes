#ifndef __MONITORH
#define __MONITORH

#include "../xerxes_lib/device.h"
#include "console.h"

namespace dave
{
    template<REG16 addr_lower, REG16 addr_upper> class monitor : public device {
    private:
        REG8 _data[addr_upper - addr_lower + 1] = {};
        void project_to_monitor(const REG16 &addr) {
            console::update_char_on_virtual_monitor(addr-addr_lower, _data[addr-addr_lower]);
        }
    public:
        monitor(system_bus *bus, debugger *debugger)
            : device(bus, debugger)
        {}
        monitor() = delete;
        monitor(const monitor&) = delete;
        monitor(monitor &&) = delete;
        auto operator =(const monitor&)->monitor& = delete;
        auto operator =(monitor &&)->monitor& = delete;

        virtual void powerup() override {
            // Map to the monitor the current state
            for(REG16 addr = addr_lower; addr <= addr_upper; addr++) {
                project_to_monitor(addr);
            }
            console::reset_cursor();
        }
        virtual void nop() override { }
        virtual void write(const REG16 &address, const REG8 *data) override {
            if (address >= addr_lower && address <= addr_upper) {
                _data[address - addr_lower] = *data;
                project_to_monitor(address);
                console::reset_cursor();
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
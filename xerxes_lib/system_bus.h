#ifndef __SYSTEMBUSH
#define __SYSTEMBUSH

#include <vector>
#include <memory>
#include <utility>

#include "device.h"
#include "cpu.h"
#include "debugger.h"

namespace dave
{
    class cpu;

    class system_bus {
    private:
        bool &_irq_line;
        bool &_nmi_line;
        debugger *_debugger;
        std::vector<std::unique_ptr<cpu>> _cpus;
        std::vector<std::unique_ptr<device>> _devices;
        bool _break_addr_written;
    public:
        system_bus(debugger *debugger, bool &irq_line, bool &nmi_line)
        : _debugger(debugger), _irq_line(irq_line), _nmi_line(nmi_line)
        {}

        system_bus(const system_bus&) = delete;
        system_bus(system_bus &&) = delete;
        auto operator =(const system_bus&)->system_bus& = delete;
        auto operator =(system_bus &&)->system_bus& = delete;

        bool irq();
        bool nmi();
        bool reset;

        bool tick();

        void nop() {
            for (auto &d : _devices) {
                d->nop();
            }
        }
        void write(const REG16 &address, const REG8 *data) {
            _debugger->report_address_write(address, data);
            for (auto &d : _devices) {
                d->write(address, data);
            }
            _break_addr_written = _debugger->break_on_bus_address_changed(address);
        }
        void read(const REG16 &address, REG8 *dest) {
            for (auto &d : _devices) {
                d->read(address, dest);
            }
        }

        auto attach_cpu(std::unique_ptr<cpu> &&cpu) -> dave::cpu*;
        auto attach_device(std::unique_ptr<device> &&device) -> dave::device*;

        void report_cpu_status();

        void powerup();
    };
}

#endif
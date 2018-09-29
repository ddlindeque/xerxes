#ifndef __MACHINEH
#define __MACHINEH

#include <memory>

#include "system_bus.h"
#include "cpu.h"
#include "device.h"

namespace dave
{
    class machine {
    private:
        system_bus _bus;
        debugger *_debugger;
    public:
        machine(debugger *debugger);

        machine(const machine&) = delete;
        machine(machine &&) = delete;
        machine& operator =(const machine&) = delete;
        machine& operator =(machine &&) = delete;

        template<typename TCpu> TCpu* install_cpu() {
            return (TCpu*)_bus.attach_cpu(std::make_unique<TCpu>(&_bus, _debugger));
        }

        template<typename TDevice> TDevice* install_device() {
            return (TDevice*)_bus.attach_device(std::make_unique<TDevice>(&_bus));
        }

        void reset(bool value);
        void nmi(bool value);
        void irq(bool value);

        void toggle_reset();
        void toggle_nmi();
        void toggle_irq();

        void report_cpu_status();

        void powerup();
        void run();
    };
}

#endif
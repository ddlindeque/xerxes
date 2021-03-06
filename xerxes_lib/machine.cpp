#include "machine.h"

#include <thread>

namespace dave
{

machine::machine(debugger *debugger)
: _debugger(debugger), _bus(debugger, _irq_line, _nmi_line)
{
    _debugger->attach_system_bus(&_bus);
}

void machine::powerup()
{
    // Powerup the system bus
    _bus.powerup();

    if (_debugger->break_on_started()) {
        return;
    }

    run();
}

void machine::run()
{
    // The 6502 runs at 1-3 MHz clock. This means each tick takes around 0.5 microseconds
    // We run at CLOCKS_PER_SEC (1 mil). We cannot measure this - it should be close enough

    while(!_bus.tick()) {
        _debugger->tick();
        // Carry on - but sleep so we have the correct speed
        std::this_thread::yield();
        if (_debugger->break_asap()) {
            break;
        }
    }
    _debugger->tick();
}

void machine::report_cpu_status()
{
    _bus.report_cpu_status();
}

void machine::reset(bool value)
{
    _bus.reset = value;
}

void machine::nmi(bool value)
{
    _nmi_line = value;
}

void machine::irq(bool value)
{
    _irq_line = value;
}

void machine::toggle_reset()
{
    _bus.reset = !_bus.reset;
}

void machine::toggle_nmi()
{
    _nmi_line = !_nmi_line;
}

void machine::toggle_irq()
{
    _irq_line = !_irq_line;
}

}
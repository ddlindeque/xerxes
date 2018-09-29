#include "emulator_debugger.h"

#include "console.h"

namespace dave
{

bool emulator_debugger::break_on_started()
{
    return _break_on_started;
}

bool emulator_debugger::break_on_next_instruction_ready(const REG16 &next_instruction_addr)
{
    return _break_on_next_instruction_ready;
}

bool emulator_debugger::break_after_instruction()
{
    return _break_after_instruction;
}

bool emulator_debugger::break_on_reset()
{
    console::alert("reset");
    return _break_on_reset;
}

bool emulator_debugger::break_on_nmi()
{
    console::alert("nmi");
    return _break_on_nmi;
}

bool emulator_debugger::break_on_interupt()
{
    console::alert("interupt");
    return _break_on_interupt;
}

bool emulator_debugger::break_on_break()
{
    console::alert("break");
    return _break_on_break;
}

void emulator_debugger::report_cpu_register(const std::string &name, const uint8_t &value)
{
    console::update_cpu_register(name, value);
    if (name == "S") {
        console::report_s(_bus, value);
    }
}

void emulator_debugger::report_cpu_register(const std::string &name, const uint16_t &value)
{
    console::update_cpu_register(name, value);
    if (name == "PC") {
        console::report_pc(_bus, value);
    }
}

void emulator_debugger::report_cpu_register(const std::string &name, const bool &value)
{
    console::update_cpu_register(name, value);
}

void emulator_debugger::report_nmi_line(bool value)
{
    console::update_nmi_line(value);
}

void emulator_debugger::report_irq_line(bool value)
{
    console::update_irq_line(value);
}

void emulator_debugger::report_reset_line(bool value)
{
    console::update_reset_line(value);
}

void emulator_debugger::tick()
{
    _ticks++;
    console::update_ticks(_ticks);
}

void emulator_debugger::report_address_write(const REG16 &addr, const REG8 *data)
{
    console::add_bus(addr, data);
}

void emulator_debugger::attach_system_bus(system_bus *bus)
{
    _bus = bus;
}

void emulator_debugger::add_watch(const REG16 &addr)
{
    auto f = _mem.emplace(addr, memory_info());
    f.first->second.watch = true;
    refresh_watches();
}

void emulator_debugger::refresh_watches() {
    console::clear_watches();
    for(auto &m : _mem) {
        if (m.second.watch) {
            REG8 cur_value = 0;
            _bus->read(m.first, &cur_value);
            console::add_watch(m.first, cur_value, cur_value != m.second.value);
            m.second.value = cur_value;
        }
    }
    console::reset_cursor();
}

}
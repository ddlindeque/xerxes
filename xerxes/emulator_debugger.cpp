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
    if (next_instruction_addr == _last_pc_broken) return false;
    _last_pc_broken = next_instruction_addr;
    return _pc_breakpoints.find(next_instruction_addr) != _pc_breakpoints.end();
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

bool emulator_debugger::break_asap()
{
    int key;
    if(console::try_getkey(key)) {
        return key == 'b';
    }
    else {
        return false;
    }
}

bool emulator_debugger::break_on_bus_address_changed(const REG16 &addr)
{
    auto f = _mem.find(addr);
    if (f != _mem.end()) {
        return f->second.must_break;
    }
    else {
        return false;
    }
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

void emulator_debugger::delete_watch(const REG16 &addr)
{
    auto f = _mem.find(addr);
    if (f != _mem.end()) {
        f->second.watch = false;
        if (!f->second.must_break) {
            _mem.erase(f);
        }
        refresh_watches();
    }
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

void emulator_debugger::add_pc_breakpoint(const REG16 &addr) {
    if (_pc_breakpoints.emplace(addr).second) {
        refresh_breakpoints();
    }
}

void emulator_debugger::delete_pc_breakpoint(const REG16 &addr) {
    _pc_breakpoints.erase(addr);
    refresh_breakpoints();
}

void emulator_debugger::add_bus_breakpoint(const REG16 &addr) {
    auto f = _mem.emplace(addr, memory_info());
    f.first->second.must_break = true;
    refresh_breakpoints();
}

void emulator_debugger::delete_bus_breakpoint(const REG16 &addr) {
    auto f = _mem.find(addr);
    if (f != _mem.end()) {
        f->second.must_break = false;
        if (!f->second.watch) {
            _mem.erase(f);
        }
        refresh_breakpoints();
    }
}

void emulator_debugger::refresh_breakpoints() {
    console::clear_breakpoints();
    for(auto &m : _mem) {
        if (m.second.must_break) {
            REG8 cur_value = 0;
            _bus->read(m.first, &cur_value);
            console::add_bus_breakpoint(m.first);
        }
    }
    for(auto &p : _pc_breakpoints) {
        console::add_pc_breakpoint(p);
    }
    console::reset_cursor();
}

void emulator_debugger::report_punchcardreader_status(bool irqHigh, bool nextByteRequested, REG8 status, REG8 byteInBuffer)
{
    console::report_punchcardreader_status(irqHigh, nextByteRequested, status, byteInBuffer);   
}

void emulator_debugger::toggle_break_on_nmi()
{
    _break_on_nmi = !_break_on_nmi;
    console::set_break_config(_break_on_nmi, _break_on_interupt, _break_on_reset);
}

void emulator_debugger::toggle_break_on_irq()
{
    _break_on_interupt = !_break_on_interupt;
    console::set_break_config(_break_on_nmi, _break_on_interupt, _break_on_reset);
}

void emulator_debugger::toggle_break_on_reset()
{
    _break_on_reset = !_break_on_reset;
    console::set_break_config(_break_on_nmi, _break_on_interupt, _break_on_reset);
}

}
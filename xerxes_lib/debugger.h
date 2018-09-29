#ifndef __DEBUGGERH
#define __DEBUGGERH

#include <string>
#include "common.h"

namespace dave
{
    class system_bus;

    class debugger {
    public:
        virtual void attach_system_bus(system_bus *bus) = 0;

        virtual bool break_on_started() = 0;
        virtual bool break_on_next_instruction_ready(const REG16 &next_instruction_addr) = 0;
        virtual bool break_after_instruction() = 0;
        virtual bool break_on_reset() = 0;
        virtual bool break_on_nmi() = 0;
        virtual bool break_on_interupt() = 0;
        virtual bool break_on_break() = 0;

        virtual void report_cpu_register(const std::string &name, const uint8_t &value) = 0;
        virtual void report_cpu_register(const std::string &name, const uint16_t &value) = 0;
        virtual void report_cpu_register(const std::string &name, const bool &value) = 0;
        virtual void tick() = 0;
        virtual void report_address_write(const REG16 &addr, const REG8 *data) = 0;

        virtual void report_nmi_line(bool value) = 0;
        virtual void report_irq_line(bool value) = 0;
        virtual void report_reset_line(bool value) = 0;
    };
}

#endif
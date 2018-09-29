#ifndef __EMULATOR_DEBUGGERH
#define __EMULATOR_DEBUGGERH

#include <map>
#include "../xerxes_lib/debugger.h"

namespace dave
{
    class emulator_debugger : public debugger {
    private:
        struct memory_info {
            bool must_break;
            bool watch;
            REG8 value;
        };

        size_t _ticks = 0;
        system_bus *_bus = nullptr;
        std::map<REG16, memory_info> _mem;
    public:
        bool _break_on_started = true;
        bool _break_on_next_instruction_ready = false;
        bool _break_after_instruction = false;
        bool _break_on_reset = true;
        bool _break_on_nmi = true;
        bool _break_on_interupt = true;
        bool _break_on_break = true;

        virtual void attach_system_bus(system_bus *bus) override;

        virtual bool break_on_started() override;
        virtual bool break_on_next_instruction_ready(const REG16 &next_instruction_addr) override;
        virtual bool break_after_instruction() override;
        virtual bool break_on_reset() override;
        virtual bool break_on_nmi() override;
        virtual bool break_on_interupt() override;
        virtual bool break_on_break() override;

        virtual void report_cpu_register(const std::string &name, const uint8_t &value) override;
        virtual void report_cpu_register(const std::string &name, const uint16_t &value) override;
        virtual void report_cpu_register(const std::string &name, const bool &value) override;
        virtual void tick() override;
        virtual void report_address_write(const REG16 &addr, const REG8 *data) override;

        virtual void report_nmi_line(bool value) override;
        virtual void report_irq_line(bool value) override;
        virtual void report_reset_line(bool value) override;

        void add_watch(const REG16 &addr);
        void refresh_watches();
    };
}

#endif
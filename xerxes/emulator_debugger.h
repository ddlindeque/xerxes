#ifndef __EMULATOR_DEBUGGERH
#define __EMULATOR_DEBUGGERH

#include <map>
#include <set>
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
        std::set<REG16> _pc_breakpoints;

        size_t _ticks = 0;
        system_bus *_bus = nullptr;
        std::map<REG16, memory_info> _mem;

        REG16 _last_pc_broken = 0;
    public:
        bool _break_on_started = true;
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
        virtual bool break_asap() override;
        virtual bool break_on_bus_address_changed(const REG16 &addr) override;

        virtual void report_cpu_register(const std::string &name, const uint8_t &value) override;
        virtual void report_cpu_register(const std::string &name, const uint16_t &value) override;
        virtual void report_cpu_register(const std::string &name, const bool &value) override;
        virtual void tick() override;
        virtual void report_address_write(const REG16 &addr, const REG8 *data) override;

        virtual void report_nmi_line(bool value) override;
        virtual void report_irq_line(bool value) override;
        virtual void report_reset_line(bool value) override;

        void add_watch(const REG16 &addr);
        void delete_watch(const REG16 &addr);
        void refresh_watches();

        void add_pc_breakpoint(const REG16 &addr);
        void delete_pc_breakpoint(const REG16 &addr);
        void add_bus_breakpoint(const REG16 &addr);
        void delete_bus_breakpoint(const REG16 &addr);
        void refresh_breakpoints();

        virtual void report_punchcardreader_status(bool irqHigh, bool nextByteRequested, REG8 status, REG8 byteInBuffer) override;

        void toggle_break_on_nmi();
        void toggle_break_on_irq();
        void toggle_break_on_reset();
    };
}

#endif
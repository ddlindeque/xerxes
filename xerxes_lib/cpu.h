#ifndef __CPUH
#define __CPUH

#include "system_bus.h"
#include "debugger.h"

namespace dave
{
    class cpu {
    protected:
        system_bus *_bus;
        debugger *_debugger;
    public:
        cpu() = delete;
        cpu(const cpu&) = delete;
        cpu(cpu &&) = delete;
        virtual ~cpu();
        auto operator =(const cpu&)->cpu& = delete;
        auto operator =(cpu &&)->cpu& = delete;

        explicit cpu(system_bus *bus, debugger *debugger);

        virtual void powerup() {}
        virtual bool tick() = 0;

        virtual void report_status() {}

        virtual void set_pc(const REG16 &addr) = 0;
    };
}

#endif
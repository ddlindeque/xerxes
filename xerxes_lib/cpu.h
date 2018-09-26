#pragma once

#include "system_bus.h"

namespace dave
{
    class cpu {
    protected:
        system_bus *_bus;
    public:
        cpu() = delete;
        cpu(const cpu&) = delete;
        cpu(cpu &&) = delete;
        virtual ~cpu();
        auto operator =(const cpu&)->cpu& = delete;
        auto operator =(cpu &&)->cpu& = delete;

        explicit cpu(system_bus *bus);

        virtual void clock() = 0;
    };
}
#include "stdafx.h"
#include "system_bus.h"
#include "cpu.h"

namespace dave
{
    void system_bus::clock()
    {
        for (auto &c : _cpus) {
            c->clock();
        }
    }
}

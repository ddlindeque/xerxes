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

    void system_bus::attach_cpu(std::unique_ptr<cpu> &&cpu)
    {
        _cpus.push_back(std::move(cpu));
    }

    void system_bus::attach_device(std::unique_ptr<device> &&device)
    {
        _devices.push_back(std::move(device));
    }
}

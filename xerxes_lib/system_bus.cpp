#include "system_bus.h"

namespace dave
{
    bool system_bus::tick()
    {
        bool must_break = false;
        for (auto &c : _cpus) {
            must_break |= c->tick();
        }
        return must_break;
    }

    dave::cpu* system_bus::attach_cpu(std::unique_ptr<cpu> &&cpu)
    {
        _cpus.push_back(std::move(cpu));
        return _cpus.back().get();
    }

    dave::device* system_bus::attach_device(std::unique_ptr<device> &&device)
    {
        _devices.push_back(std::move(device));
        return _devices.back().get();
    }

    void system_bus::powerup()
    {
        for(auto &d : _devices) {
            d->powerup();
        }
        for(auto &c : _cpus) {
            c->powerup();
        }
    }

    void system_bus::report_cpu_status()
    {
        for(auto &c : _cpus) {
            c->report_status();
        }
    }
}

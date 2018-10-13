#include "system_bus.h"

namespace dave
{
    bool system_bus::irq()
    {
        if (_irq_line) return true;
        for(auto &d : _devices) {
            if (d->irq()) return true;
        }
        return false;
    }

    bool system_bus::nmi()
    {
        if (_nmi_line) return true;
        for(auto &d : _devices) {
            if (d->nmi()) return true;
        }
        return false;
    }

    bool system_bus::tick()
    {
        _break_addr_written = false;
        bool must_break = false;
        for (auto &d : _devices) {
            d->tick();
        }
        for (auto &c : _cpus) {
            must_break |= c->tick();
        }
        return must_break || _break_addr_written;
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

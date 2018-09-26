#pragma once

#include "system_bus.h"
#include "cpu.h"
#include "device.h"

namespace dave
{
    class machine {
    private:
        system_bus _bus;
    public:
        machine();

        machine(const machine&) = delete;
        machine(machine &&) = delete;
        auto operator =(const machine&)->machine& = delete;
        auto operator =(machine &&)->machine& = delete;

        template<typename TCpu> auto install_cpu() -> void {
            _bus.attach_cpu(std::make_unique<TCpu>(&_bus));
        }
        template<typename TDevice> auto install_device() -> void {
            _bus.attach_device(std::make_unique<TDevice>(&_bus));
        }
    };
}
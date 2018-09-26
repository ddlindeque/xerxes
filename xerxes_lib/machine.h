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

        auto install_cpu(std::unique_ptr<cpu> &&cpu) -> void;
        auto install_device(std::unique_ptr<device> &&device) -> void;
    };
}
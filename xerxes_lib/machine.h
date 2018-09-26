#pragma once

#include "system_bus.h"

namespace dave
{
    class machine {
    private:
        system_bus _bus;
    public:
        machine()
        {}

        machine(const machine&) = delete;
        machine(machine &&) = delete;
        auto operator =(const machine&)->machine& = delete;
        auto operator =(machine &&)->machine& = delete;

        template<typename TCpu, typename ... TArgs> inline auto add_cpu(TArgs&& ... args) -> void {
            _bus.add_cpu<TCpu, TArgs...>(std::forward<TArgs...>(args)...);
        }

        template<typename TDevice, typename ... TArgs> inline auto add_device(TArgs&& ... args) -> void {
            _bus.add_device<TDevice, TArgs...>(std::forward<TArgs...>(args)...);
        }
    };
}
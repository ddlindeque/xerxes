#pragma once

#include <vector>
#include <memory>
#include <utility>
#include "device.h"

namespace dave
{
    class cpu;

    class system_bus {
    private:
        std::vector<std::unique_ptr<cpu>> _cpus;
        std::vector<std::unique_ptr<device>> _devices;
    public:
        system_bus()
        {}

        system_bus(const system_bus&) = delete;
        system_bus(system_bus &&) = delete;
        auto operator =(const system_bus&)->system_bus& = delete;
        auto operator =(system_bus &&)->system_bus& = delete;

        bool irq;
        bool nmi;
        bool reset;

        void clock();

        void nop() {
            for (auto &d : _devices) {
                d->nop();
            }
        }
        void write(const REG16 &address, const REG8 *data) {
            for (auto &d : _devices) {
                d->write(address, data);
            }
        }
        void read(const REG16 &address, REG8 *dest) {
            for (auto &d : _devices) {
                d->read(address, dest);
            }
        }

        template<typename TCpu, typename ... TArgs> inline auto add_cpu(TArgs&& ... args) -> void {
            _cpus.push_back(std::make_unique<TCpu>(this, std::forward<TArgs...>(args)...));
        }

        template<typename TDevice, typename ... TArgs> inline auto add_device(TArgs&& ... args) -> void {
            _devices.push_back(std::make_unique<TDevice>(this, std::forward<TArgs...>(args)...));
        }
    };
}
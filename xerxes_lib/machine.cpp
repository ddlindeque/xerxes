#include "machine.h"

namespace dave
{

machine::machine()
{}

auto machine::install_cpu(std::unique_ptr<cpu> &&cpu) -> void
{
    _bus.attach_cpu(std::move(cpu));
}

auto machine::install_device(std::unique_ptr<device> &&device) -> void
{
    _bus.attach_device(std::move(device));
}

}
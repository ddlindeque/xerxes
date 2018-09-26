#include "device.h"

namespace dave
{

device::device(system_bus *bus)
    : _bus(bus)
{}

device::~device()
{}

}
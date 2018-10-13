#include "device.h"

namespace dave
{

device::device(system_bus *bus, debugger *debugger)
    : _bus(bus), _debugger(debugger)
{}

device::~device()
{}

}
#include "cpu.h"

namespace dave
{

cpu::cpu(system_bus *bus, debugger *debugger)
: _bus(bus), _debugger(debugger)
{}

cpu::~cpu()
{}

}
#include "cpu.h"

namespace dave
{

cpu::cpu(system_bus *bus)
: _bus(bus)
{}

cpu::~cpu()
{}

}
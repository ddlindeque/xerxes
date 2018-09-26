#include "../xerxes_lib/machine.h"
#include "../xerxes_lib/cpu6502.h"
#include "../xerxes_lib/rom.h"

#include <iostream>

int main()
{
    dave::machine machine;

    machine.install_cpu<dave::cpu6502>();
    machine.install_device<dave::rom<0xA000, 0xBFFF>>();
    machine.install_device<dave::rom<0xE000, 0xFFFF>>();

    std::cout << "Machine assembled" << std::endl;

    return 0;
}


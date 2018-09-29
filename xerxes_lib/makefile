CC=clang++ -c -std=c++14 -g

default: ../bin/xerxes_lib.a

../bin/common.o: common.h common.cpp
	$(CC) common.cpp -o $@

../bin/cpu.o: cpu.h debugger.h system_bus.h cpu.cpp
	$(CC) cpu.cpp -o $@

../bin/cpu6502.o: system_bus.h common.h cpu.h debugger.h cpu6502.h cpu6502.cpp
	$(CC) cpu6502.cpp -o $@

../bin/device.o: common.h device.h device.cpp
	$(CC) device.cpp -o $@

../bin/machine.o: system_bus.h machine.h cpu.h device.h machine.cpp
	$(CC) machine.cpp -o $@

../bin/system_bus.o: system_bus.h device.h cpu.h debugger.h system_bus.cpp
	$(CC) system_bus.cpp -o $@

../bin/xerxes_lib.a: ../bin/common.o ../bin/cpu.o ../bin/cpu6502.o ../bin/device.o ../bin/machine.o ../bin/system_bus.o
	~/llvm/obj/bin/llvm-ar -rc $@ $^

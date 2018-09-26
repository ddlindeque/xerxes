CC=clang++ -c -std=c++14

default: ../bin/xerxes_lib.a

../bin/common.o: common.h common.cpp
	$(CC) common.cpp -o ../bin/$@

../bin/cpu.o: cpu.h system_bus.h cpu.cpp
	$(CC) cpu.cpp -o ../bin/$@

../bin/cpu6502.o: system_bus.h cpu.h cpu6502.h cpu6502.cpp
	$(CC) cpu6502.cpp -o ../bin/$@

../bin/device.o: common.h device.h device.cpp
	$(CC) device.cpp -o ../bin/$@

../bin/machine.o: system_bus.h machine.h cpu.h device.h machine.cpp
	$(CC) machine.cpp -o ../bin/$@

../bin/system_bus.o: system_bus.h device.h system_bus.cpp
	$(CC) system_bus.cpp -o ../bin/$@

../bin/xerxes_lib.a: ../bin/common.o ../bin/cpu.o ../bin/cpu6502.o ../bin/device.o ../bin/machine.o ../bin/system_bus.o
	~/llvm/obj/bin/llvm-ar -rc $@ $^

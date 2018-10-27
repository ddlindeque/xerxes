CC=clang++ -c -std=c++14 -g

default: ../bin/xerxes

../bin/monitor.o: monitor.h console.h monitor.cpp ../xerxes_lib/device.h
	$(CC) monitor.cpp -o $@

../bin/emulator_debugger.o: emulator_debugger.h console.h ../xerxes_lib/debugger.h emulator_debugger.cpp
	$(CC) emulator_debugger.cpp -o $@

../bin/console.o: console.h ../xerxes_lib/common.h ../xerxes_lib/system_bus.h console.cpp
	$(CC) console.cpp -o $@

../bin/xerxes.m.o: ../xerxes_lib/machine.h ../xerxes_lib/cpu.h ../xerxes_lib/rom.h ../xerxes_lib/ram.h monitor.h emulator_debugger.h console.h xerxes.m.cpp ../software/romv1.h ../software/romv2.h
	$(CC) xerxes.m.cpp -o $@

../bin/xerxes: ../bin/xerxes.m.o ../bin/monitor.o ../bin/emulator_debugger.o ../bin/console.o ../bin/xerxes_lib.a
	clang++ $^ -lncurses -o $@
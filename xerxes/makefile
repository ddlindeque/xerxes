CC=clang++ -c -std=c++14

default: ../bin/xerxes

../bin/xerxes.m.o: xerxes.m.cpp
	$(CC) xerxes.m.cpp -o ../bin/$@

../bin/xerxes: ../bin/xerxes.m.o ../bin/xerxes_lib.a
	clang++ $^ -o $@

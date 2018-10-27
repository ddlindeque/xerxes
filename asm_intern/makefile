CC=clang++ -c -std=c++14 -g

default: ../bin/intern

../bin/code_generator.o: code_generator.cpp code_generator.h
	$(CC) code_generator.cpp -o $@

../bin/pc_code_generator.o: pc_code_generator.cpp pc_code_generator.h code_generator.h
	$(CC) pc_code_generator.cpp -o $@

../bin/rom_code_generator.o: rom_code_generator.cpp rom_code_generator.h code_generator.h
	$(CC) rom_code_generator.cpp -o $@

../bin/lexer.o: lexer.cpp lexer.h ../xerxes_lib/common.h
	$(CC) lexer.cpp -o $@

../bin/parser.o: parser.cpp parser.h lexer.h ../xerxes_lib/common.h
	$(CC) parser.cpp -o $@

../bin/layout.o: layout.cpp layout.h ast.h ../xerxes_lib/common.h
	$(CC) layout.cpp -o $@

../bin/addressing.o: addressing.cpp addressing.h ast.h ../xerxes_lib/common.h
	$(CC) addressing.cpp -o $@

../bin/logger.o: logger.cpp logger.h
	$(CC) logger.cpp -o $@

../bin/asm_intern.m.o: asm_intern.m.cpp code_generator.h pc_code_generator.h rom_code_generator.h parser.h
	$(CC) asm_intern.m.cpp -o $@

../bin/intern: ../bin/asm_intern.m.o ../bin/code_generator.o ../bin/pc_code_generator.o ../bin/rom_code_generator.o ../bin/lexer.o ../bin/parser.o ../bin/layout.o ../bin/addressing.o ../bin/logger.o
	clang++ $^ -o $@

clean:
	rm -r ../bin/*
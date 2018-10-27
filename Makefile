default: buildall

buildall:
	cd xerxes_lib; make
	cd xerxes; make
	cd asm_intern; make
	./bin/intern -i ./software/main.asm -i ./software/monitor-driver.asm -i ./software/data.asm -fmt punchcard -o ./software/software.pc

clean:
	rm -r ./bin/*
	rm -r ./software/software.pc
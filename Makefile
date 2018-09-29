default: buildall

buildall:
	cd xerxes_lib; make
	cd xerxes; make

clean:
	rm -r ./bin/*
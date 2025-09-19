test:
	mkdir -p build
	rm -f examples/forge
	g++ src/main.cpp -o examples/forge

run:
	mkdir -p build
	rm -f build/forge
	g++ src/* -o build/forge
	./build/forge examples/main.fgl

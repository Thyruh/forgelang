
test:
	mkdir -p build
	rm -f build/forge
	g++ src/* -o build/forge



run:
	mkdir -p build
	rm -f build/forge
	g++ src/* -o build/forge
	./build/forge tests/main.fgl





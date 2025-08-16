main:
	mkdir -p build
	rm -f build/forge
	g++ src/* -Iinclude -o build/forge

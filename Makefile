main:
	mkdir -p build
	rm -f build/forge
	clang++ src/* -Iinclude -o build/forge

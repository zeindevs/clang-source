run: build64
	./cpu.exe

build64:
	clang++ -O3 -DNDEBUG -o cpu.exe -m64 cpu.cpp

build32:
	clang++ -O3 -DNDEBUG -o cpu.exe -m32 cpu.cpp

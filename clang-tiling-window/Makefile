CXX = clang

all: clean dll exe

exe:
	$(CXX) -Os wm.c -o wm.exe -luser32

dll:
	$(CXX) -shared -o wm_dll.dll -fuse-ld=lld -target x86_64-pc-windows-msvc -m64 wm_dll.c -luser32

clean:
	rm *.dll
	rm *.lib
	rm *.exe

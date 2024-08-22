debug: compile-debug

release: compile-release

compile-debug:
	cmake --build build --config Debug

compile-release:
	cmake --build build --config Release

init-debug:
	cmake -S . -B build -G "Ninja" -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug

init-release:
	cmake -S . -B build -G "Ninja" -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Release 


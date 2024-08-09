run: build
	./curl_request.exe

build:
	clang++ -Os -o curl_request.exe -m64 -target x86_64-pc-windows-msvc -I"./curl/include" -L"./curl/build/lib" curl_request.cpp -lcurl -static

# Chapter 1: Towards Modern C++

**Compilation environment**

```sh
> clang++ -v
clang version 18.1.8
Target: x86_64-pc-windows-msvc
Thread model: posix
InstalledDir: D:\dev\clang\bin
```

## 1.1 Deprecated Features

- The string literal constant is no longer allowed to be assigned to a `char *`. If you need to assign and initialize a `char *` with a string literal constant, you should use const `char *` or auto.
    ```cpp 
    char *str = 
    ```
- C++98 exception description, `unexpected_handler`, `set_excepted()` and other related features are deprecated and should use `noexcept`;
- `auto_ptr` is deprecated and `unique_ptr` should be used.
- `register` keyword is deprecated and can be used but no longer has any pratical meaning.
- The `++` opreation of the bool type is deprecated;
- If a class has a destructor, the properties for which is generates copy constructors and copy assignment operations are deprecated.
- C language style type conversion is deprecated (ie using (`conver_type`)) before variables, and `static_cast`, `reinterpret_cast`, `const_cast` should be used for type conversion.
- Is particular, some of the C standard libraries that can be used are deprecated in the latest C++17 standard, such as `<ccomplex>`, `<cstdalign>`, `<cstbool>`, and `<ctgmath>` etc.- and many more...

## 1.2 Compatibilities with C

When writing C++, yyou should pay attention of the use of `extern "C"`, separate the C langauge code form the C++ code, and then unify the link, for instance:

```cpp
// foo.h
#ifdef __cplusplus
extern "C" {
#endif

inf add(int x, int y);

#ifdef __cplusplus
}
#endif
```

```c
// foo.c
int add(int x, int y) {
    return x + y;
}
```

```cpp
// 1.1.cpp
#include "foo.h"
#include <iostream>
#include <functional>

int main() {
    [out = std::ref(std::cout << "Result from C code: " << add(1, 2))]() {
        out.get() << ".\n";
    }();
    return 0;
}
```

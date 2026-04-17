#include <iostream>
#include "stub.h"

// cpp-stub's arm64 trampoline is 16 bytes. Keep these two functions well
// bigger than that and noinline so the linker won't place one inside the
// other's patch range.
__attribute__((noinline)) int foo() {
    std::cout << "I am foo" << std::endl;
    return 1;
}

__attribute__((noinline)) int foo_stub() {
    std::cout << "I am foo_stub" << std::endl;
    return 42;
}

int main() {
    Stub s;
    s.set(foo, foo_stub);
    int v = foo();
    if (v != 42) {
        std::cerr << "FAIL: after set, foo()=" << v << ", expected 42\n";
        return 1;
    }
    s.reset(foo);
    v = foo();
    if (v != 1) {
        std::cerr << "FAIL: after reset, foo()=" << v << ", expected 1\n";
        return 1;
    }
    std::cout << "cmake_smoke: OK" << std::endl;
    return 0;
}

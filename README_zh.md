[中文](README_zh.md)|[English](README.md)

# 原理介绍

- 如何获取原函数的地址(**addr_pri.h**、**addr_any.h**)
- 如何用桩函数替换原函数(**stub.h**)

# 支持情况
- 支持的操作系统 :
  * [x] Windows
  * [x] Linux
  * [x] MacOS —— 同时支持 Intel (x86-64) 与 Apple Silicon (arm64)。由于 macOS
    对 `__TEXT` 段施加 W^X 限制，并且 `mprotect` 不能超过段的 `maxprot`，
    使用 cpp-stub 的每个可执行文件都需要在链接后做一次小处理（把 `__TEXT.maxprot`
    抬到 `rwx` 并 ad-hoc 重新签名，见 [issue #49](https://github.com/coolxv/cpp-stub/issues/49)）。
    本仓库提供了构建系统集成助手，下游通常**不需要手动调用**任何脚本。
    详见下文的 [macOS 集成说明](#macos-集成说明)。

- 支持的硬件平台 :
  * [x] x86
  * [x] x86-64
  * [x] arm32
  * [x] arm64
  * [x] arm thumb
  * [x] riscv32
  * [x] riscv64
  * [x] loongarch64
  * [x] mips64
  * [x] ppc64 (由chatgpt 生成，需要验证)
  * [x] s390x (由chatgpt 生成，需要验证)
  * [x] alpha (由chatgpt 生成，需要验证)
  * [x] sparc (由chatgpt 生成，需要验证)
  * [x] sw_64 (由chatgpt 生成，需要验证n)

- 支持的编译器 : 
  * [x] msvc
  * [x] gcc
  * [x] clang

- 支持函数类型：
  * [x] [常规函数](test/test_function.cpp)
  * [x] [可变参函数](test/test_variadic_function.cpp)
  * [x] [模板函数](test/test_template_function_linux.cpp)
  * [x] [重载函数](test/test_overload_function_linux.cpp)
  * [x] [lambda函数](test/test_addr_lambda_linux.cpp)
  * [x] [静态函数(使用 addr_any.h)](test/test_addr_any_linux.cpp)
  * [x] 内联函数(通过编译选项)
  * [x] [类的构造函数](test/test_constructor_function_linux.cpp)
  * [x] [类的析构函数](test/test_dtor_function_linux.cpp)
  * [x] [类的成员函数](test/test_object_member_function_linux.cpp)
  * [x] [类的静态成员函数](test/test_class_member_function.cpp)
  * [x] [类的成员虚函数(非纯虚函数)](test/test_virtual_function_linux.cpp)
  * [x] [类的虚函数并且重载](test/test_virtual_overload_function_linux.cpp)
  * [x] [仿函数](test/test_functor_linux.cpp)
  * [x] [类的私有成员函数(使用 addr_pri.h)](test/test_private_member_function_linux.cpp)
  * [x] [类的私有成员函数(cpp17)](test_cpp17/test_private_member.cpp)
  * [x] 动态库里的函数(使用 dlsym()获取)


# 单元测试相关说明
## 不能打桩
- 不能对 exit 函数打桩（部分系统调用）
- 不能对纯虚函数打桩, 纯虚函数没有地址
- 不能对 lambda 函数打桩, lambda 函数获取不到地址(但可以尝试使用 addr_any.h 接口获取地址)
- 不能对静态函数打桩, 静态函数地址不可见.(但可以尝试使用 addr_any.h 接口获取地址)


## 单元测试编译选项, linux g++可用的
- -fno-access-control
- -fno-inline
- -Wno-pmf-conversions
- -Wl,--allow-multiple-definition
- -no-pie -fno-stack-protector
- -fprofile-arcs
- -ftest-coverage

## macOS 集成说明

macOS 下每个使用 cpp-stub 的可执行文件都需要在链接完成后做一次小处理：
把 `__TEXT.maxprot` 抬到 `rwx`，并用 ad-hoc 重新签名。本仓库自带构建系统
助手帮你自动做这件事；Linux / Windows 下这些助手是空操作。

### CMake（零手动步骤）

```cmake
add_subdirectory(third_party/cpp-stub)
add_executable(my_test test.cpp)
target_link_libraries(my_test PRIVATE cpp-stub)
```

仓库根部的 `CMakeLists.txt` 会注册一个 deferred hook：在配置阶段结束时
遍历所有 executable，对每个直接链接 `cpp-stub` 的目标自动挂 POST_BUILD
步骤。只有当可执行文件通过中间静态库**间接**链接 cpp-stub 时，才需要
显式调用：

```cmake
cpp_stub_enable(my_test)
```

### Makefile

```make
CPP_STUB_DIR := third_party/cpp-stub
include $(CPP_STUB_DIR)/mk/cpp-stub.mk

my_test: my_test.cpp
	$(CXX) $(addprefix -I,$(CPP_STUB_INCLUDE)) ... -o $@ $<
	@$(CPP_STUB_POSTLINK)
```

`$(CPP_STUB_POSTLINK)` 在 macOS 上展开为 enable-stub 命令，
在 Linux/Windows 上展开为空操作 (`:`)，同一份 Makefile 跨平台可用。

### Xcode / Bazel / 其它

加一条 run-script / `genrule`，对每个可执行文件调一次：

```bash
third_party/cpp-stub/tool/macos_enable_stub.sh "$TARGET_BUILD_DIR/$EXECUTABLE_PATH"
```

### 为什么需要这一步

参见 [issue #49](https://github.com/coolxv/cpp-stub/issues/49) 以及
[src/stub.h](src/stub.h) 的 `__APPLE__` 分支：内核不允许
`mprotect`/`mach_vm_protect` 把 `__TEXT` 的权限提升到 Mach-O 里
`maxprot` 之上，而 Apple Silicon 还在页表层面强制 W^X。
`tool/macos_enable_stub.sh` 负责把 `maxprot` 抬到 `rwx`，这样
cpp-stub 运行时才能通过 `mach_vm_remap` 拿到 `__TEXT` 的可写别名。


## 代码覆盖率, linux g++使用方法
```
lcov -d build/ -z
lcov -d build/ -b ../../src1 --no-external -rc lcov_branch_coverage=1 -t ut -c -o ut_1.info
lcov -d build/ -b ../../src2 --no-external -rc lcov_branch_coverage=1 -t ut -c -o ut_2.info
lcov -a ut_1.info -a ut_2.info -o ut.info
genhtml -o report/ --prefix=`pwd` --branch-coverage --function-coverage ut.info
```
## 代码覆盖率, Windows使用方法
 [OpenCppCoverage](https://github.com/OpenCppCoverage/OpenCppCoverage)
```
OpenCppCoverage.exe --sources MySourcePath* -- YourProgram.exe arg1 arg2
```

# 接口介绍

## stub.h
```
Stub stub
stub.set(addr, addr_stub)
stub.reset(addr)
```

## addr_pri.h
```
Declaration:
    ACCESS_PRIVATE_FIELD(ClassName, TypeName, FieldName)
    ACCESS_PRIVATE_FUN(ClassName, TypeName, FunName)
    ACCESS_PRIVATE_STATIC_FIELD(ClassName, TypeName, FieldName)
    ACCESS_PRIVATE_STATIC_FUN(ClassName, TypeName, FunName)

Use:
    access_private_field::ClassNameFieldName(object);
    access_private_static_field::ClassName::ClassNameFieldName();
    call_private_fun::ClassNameFunName(object,parameters...);
    call_private_static_fun::ClassName::ClassNameFunName(parameters...);
    get_private_fun::ClassNameFunName();
    get_private_static_fun::ClassName::ClassNameFunName();
```

## addr_any.h(linux)
```
AddrAny any //for exe
AddrAny any(libname) //for lib

int get_local_func_addr_symtab(std::string func_name_regex_str, std::map<std::string,void*>& result)
int get_global_func_addr_symtab(std::string func_name_regex_str, std::map<std::string,void*>& result)
int get_weak_func_addr_symtab(std::string func_name_regex_str, std::map<std::string,void*>& result)

int get_global_func_addr_dynsym( std::string func_name_regex_str, std::map<std::string,void*>& result)
int get_weak_func_addr_dynsym(std::string func_name_regex_str, std::map<std::string,void*>& result)

```
## addr_any.h(windows)
```
AddrAny any //for all
int get_func_addr(std::string func_name, std::map<std::string,void*>& result)
```
## addr_any.h(darwin)
```
没实现
```



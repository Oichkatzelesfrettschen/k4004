# K4004 Emulator - Requirements and Build Instructions

**Version:** 2.0
**Last Updated:** 2025-11-19

---

## Table of Contents

1. [Overview](#overview)
2. [System Requirements](#system-requirements)
3. [Dependencies](#dependencies)
4. [Building from Source](#building-from-source)
5. [CMake Configuration Options](#cmake-configuration-options)
6. [Running Tests](#running-tests)
7. [Installation](#installation)
8. [Usage Examples](#usage-examples)
9. [Troubleshooting](#troubleshooting)

---

## 1. Overview

This document describes the requirements, dependencies, and build procedures for the K4004 Intel 4004/4040 emulator project.

---

## 2. System Requirements

### Supported Platforms

- **Linux** (Ubuntu 18.04+, Fedora 30+, Arch, etc.)
- **macOS** (10.14+)
- **Windows** (10/11 with Visual Studio 2019+ or MinGW)

### Minimum Hardware

- **CPU:** Any x86_64 or ARM64 processor
- **RAM:** 512 MB
- **Disk:** 100 MB for source + build artifacts

### Recommended Hardware

- **CPU:** Modern multi-core processor
- **RAM:** 2 GB
- **Disk:** 500 MB (includes test data and documentation)

---

## 3. Dependencies

### Required Dependencies

| Dependency | Minimum Version | Purpose |
|------------|----------------|---------|
| **CMake** | 3.10+ | Build system generator |
| **C++ Compiler** | C++17 support | Compilation (GCC 7+, Clang 6+, MSVC 2019+) |
| **Git** | 2.0+ | Version control and submodule management |

### Optional Dependencies

| Dependency | Minimum Version | Purpose |
|------------|----------------|---------|
| **Google Test** | 1.10.0+ | Unit testing (included as submodule) |
| **clang-tidy** | 10.0+ | Static analysis (optional) |
| **Doxygen** | 1.8.0+ | Documentation generation (future) |

### Installing Dependencies

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install -y build-essential cmake git clang-tidy
```

#### Fedora/RHEL/CentOS
```bash
sudo dnf install -y gcc-c++ cmake git clang-tools-extra
```

#### macOS (Homebrew)
```bash
brew install cmake git llvm
```

#### Windows (Visual Studio)
1. Install [Visual Studio 2019+](https://visualstudio.microsoft.com/) with C++ workload
2. Install [CMake](https://cmake.org/download/)
3. Install [Git for Windows](https://git-scm.com/download/win)

#### Windows (MinGW)
```bash
# Using MSYS2
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake git
```

---

## 4. Building from Source

### Step 1: Clone the Repository

```bash
git clone https://github.com/Oichkatzelesfrettschen/k4004.git
cd k4004
```

### Step 2: Initialize Submodules

Google Test is included as a Git submodule:

```bash
git submodule update --init --recursive
```

### Step 3: Create Build Directory

```bash
mkdir build
cd build
```

### Step 4: Configure with CMake

**Default configuration (with tests):**
```bash
cmake ..
```

**Release build (optimized):**
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
```

**Debug build (with symbols):**
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

**With static analysis:**
```bash
cmake -DSTATIC_ANALYSIS=ON ..
```

**Without tests:**
```bash
cmake -DBUILD_TESTS=OFF ..
```

### Step 5: Build

**Linux/macOS:**
```bash
make -j$(nproc)
```

**Windows (Visual Studio):**
```bash
cmake --build . --config Release
```

**Cross-platform CMake build:**
```bash
cmake --build . --parallel
```

### Build Output

After building, executables will be in:
- **Linux/macOS:** `build/output/bin/` or `build/`
- **Windows:** `build/output/Release/bin/` or `build/Release/`

**Generated binaries:**
- `assembler` - 4004/4040 assembler and disassembler
- `mcs4_emulator` - Main emulator application
- `emulator_tests` - Emulator test suite (if BUILD_TESTS=ON)
- `assembler_tests` - Assembler test suite (if BUILD_TESTS=ON)

---

## 5. CMake Configuration Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_TESTS` | `ON` | Build test suites (requires gtest submodule) |
| `STATIC_ANALYSIS` | `OFF` | Enable clang-tidy static analysis |
| `CMAKE_BUILD_TYPE` | `Debug` | Build type: `Debug`, `Release`, `RelWithDebInfo`, `MinSizeRel` |
| `CMAKE_INSTALL_PREFIX` | `/usr/local` | Installation directory prefix |

### Example: Custom Configuration

```bash
cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTS=ON \
  -DSTATIC_ANALYSIS=OFF \
  -DCMAKE_INSTALL_PREFIX=/opt/k4004 \
  ..
```

---

## 6. Running Tests

### Build and Run All Tests

```bash
cd build
cmake --build . --target emulator_tests assembler_tests
ctest --output-on-failure
```

### Run Specific Test Suite

**Emulator core tests:**
```bash
./emulator_tests
```

**Assembler tests:**
```bash
./assembler_tests
```

### Verbose Test Output

```bash
ctest -V
```

### Run Tests with Filtering (Google Test)

```bash
./emulator_tests --gtest_filter="*4004*"
./emulator_tests --gtest_filter="*JMS*"
```

---

## 7. Installation

### Install to System (Linux/macOS)

```bash
cd build
sudo make install
```

Or using CMake:
```bash
cd build
sudo cmake --install .
```

### Install to Custom Directory

```bash
cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local ..
make install
```

### Installation Structure

```
<prefix>/
├── bin/
│   ├── assembler
│   └── mcs4_emulator
├── lib/
│   └── (libraries, if any)
├── include/
│   └── k4004/
│       └── (headers, if installed)
└── share/
    └── k4004/
        ├── docs/
        └── examples/
```

---

## 8. Usage Examples

### Assemble a Program

```bash
./assembler input.asm -o output.bin
```

### Disassemble a Binary

```bash
./assembler -d input.bin
```

### Run the Emulator

```bash
./mcs4_emulator program.bin
```

### Run with Verbose Output

```bash
./mcs4_emulator -v program.bin
```

---

## 9. Troubleshooting

### Issue: CMake can't find compiler

**Solution:**
```bash
# Specify compiler explicitly
cmake -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ ..
```

### Issue: Submodule (gtest) not found

**Solution:**
```bash
git submodule update --init --recursive
```

### Issue: Build fails with C++17 errors

**Solution:**
Ensure your compiler supports C++17:
```bash
# Check GCC version (need 7+)
g++ --version

# Check Clang version (need 6+)
clang++ --version
```

### Issue: Tests fail to build

**Solution:**
Ensure submodules are initialized and BUILD_TESTS is ON:
```bash
git submodule update --init --recursive
cmake -DBUILD_TESTS=ON ..
```

### Issue: clang-tidy not found

**Solution:**
```bash
# Ubuntu/Debian
sudo apt install clang-tidy

# Or disable static analysis
cmake -DSTATIC_ANALYSIS=OFF ..
```

### Issue: Permission denied on install

**Solution:**
Use sudo or install to user directory:
```bash
cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local ..
make install
```

### Issue: Windows build fails with "cannot open file"

**Solution:**
Run as Administrator or check antivirus isn't blocking the build.

### Issue: Stack overflow during tests

**Solution:**
This is expected behavior for stack overflow tests - they validate proper bounds checking.

---

## 10. Advanced Build Configuration

### Cross-Compilation

```bash
# For ARM64 from x86_64
cmake -DCMAKE_TOOLCHAIN_FILE=arm64-toolchain.cmake ..
```

### Building with AddressSanitizer (ASan)

```bash
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer" \
      ..
```

### Building with Thread Sanitizer (TSan)

```bash
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_FLAGS="-fsanitize=thread" \
      ..
```

### Building with UndefinedBehaviorSanitizer (UBSan)

```bash
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_FLAGS="-fsanitize=undefined" \
      ..
```

### Generating Compilation Database (for IDEs)

```bash
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
```

---

## 11. Development Workflow

### Incremental Builds

After making changes:
```bash
cd build
make -j$(nproc)
ctest
```

### Clean Build

```bash
cd build
make clean
# Or full rebuild
rm -rf *
cmake ..
make -j$(nproc)
```

### Code Formatting (if clang-format is configured)

```bash
find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
```

---

## 12. Continuous Integration

This project uses GitHub Actions for CI. The workflow:
1. Builds on Linux (Ubuntu) and Windows
2. Runs all test suites
3. Validates code with clang-tidy (if enabled)

To run locally what CI runs:
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON ..
cmake --build . --parallel
ctest --output-on-failure
```

---

## 13. Performance Optimization

### Release Build with LTO (Link-Time Optimization)

```bash
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON \
      ..
```

### Profile-Guided Optimization (PGO)

1. Build with profiling:
```bash
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-fprofile-generate" \
      ..
make
```

2. Run representative workload:
```bash
./mcs4_emulator benchmark.bin
```

3. Rebuild with profile data:
```bash
cmake -DCMAKE_CXX_FLAGS="-fprofile-use" ..
make
```

---

## 14. Documentation Generation

### Generate Doxygen Documentation (future feature)

```bash
doxygen Doxyfile
```

Output will be in `docs/html/index.html`.

---

## 15. Platform-Specific Notes

### Linux
- Default compiler: GCC
- Recommended for development
- Full sanitizer support

### macOS
- Use Homebrew for dependencies
- May need to specify compiler: `CC=clang CXX=clang++`
- XCode Command Line Tools required

### Windows
- Visual Studio 2019+ recommended
- CMake GUI can simplify configuration
- Set correct generator: `-G "Visual Studio 16 2019"`

---

## 16. Support and Contact

- **Repository:** https://github.com/Oichkatzelesfrettschen/k4004
- **Issues:** https://github.com/Oichkatzelesfrettschen/k4004/issues
- **Documentation:** See `docs/` directory

---

**AD ASTRA PER MATHEMATICA ET SCIENTIAM**

*Last updated: 2025-11-19*

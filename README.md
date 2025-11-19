# K4004 - Intel 4004/4040 Emulator and Development Platform

[![Build Status](https://github.com/Oichkatzelesfrettschen/k4004/workflows/CI/badge.svg)](https://github.com/Oichkatzelesfrettschen/k4004/actions)

**A comprehensive, documentation-accurate emulator for the Intel 4004 (MCS-4) and Intel 4040 (MCS-40) microprocessors, with assembler, test suite, and extended capabilities.**

---

## Overview

K4004 is a complete development platform for Intel's pioneering 4-bit microprocessors. Originally conceived as an assembler and emulator for the Intel 4004, the project has evolved into a comprehensive system that:

- **Faithfully emulates** both the Intel 4004 and 4040 CPUs with documentation accuracy
- **Implements all 60 instructions** (46 from 4004 + 14 new 4040 instructions)
- **Supports advanced 4040 features** including interrupts, bank switching, and halt mode
- **Provides a complete toolchain** with assembler, disassembler, and emulator
- **"Cranks everything to eleven"** with enhanced configurations beyond original specs
- **Aims for ambitious goals** like a 4-bit BASIC interpreter and simple DOS/monitor

### Key Features

✅ **Accurate 4004 CPU Emulation**
- 3-level stack (corrected)
- 12-bit program counter (4KB ROM addressing)
- 16 × 4-bit registers (8 pairs)
- All 46 original instructions
- Cycle-accurate timing support

✅ **Complete 4040 CPU Emulation**
- 7-level stack (corrected)
- 13-bit addressing with ROM bank switching (8KB)
- 24 × 4-bit registers (2 banks of 12)
- All 60 instructions including 14 new ones:
  - Logical: OR4, OR5, AN6, AN7
  - Bank control: DB0, DB1, SB0, SB1
  - Interrupts: EIN, DIN, BBS
  - Special: HLT, LCR, RPM

✅ **MCS-4/MCS-40 Support Chips**
- 4001 ROM chips (256B + 4-bit I/O each)
- 4002 RAM chips (40B + 4-bit output each)
- Expandable I/O architecture

✅ **Development Tools**
- Full assembler with labels and directives
- Disassembler for reverse engineering
- Comprehensive test suite with Google Test

✅ **Documentation**
- Official Intel manuals (MCS-4 and MCS-40) included
- Technical audit and architecture documentation
- Detailed API and usage guides

---

## Project Goals

### Documentation Accuracy (Phase 1) ✅ COMPLETE

Build an emulator that faithfully implements Intel specifications:
- Correct stack depths (3 for 4004, 7 for 4040)
- Accurate instruction timing and behavior
- Proper memory addressing and banking
- Full interrupt and halt support for 4040

### Extended Capabilities (Phase 2) 🚧 IN PROGRESS

"Crank everything to eleven":
- Expanded ROM and RAM beyond original limits
- Additional I/O peripherals
- Enhanced debugging and inspection tools
- Performance optimizations while maintaining accuracy

### Software Ecosystem (Phase 3) 📋 PLANNED

The ultimate goal is to create something fun and useful:
- **4-bit BASIC interpreter** - A minimal BASIC for 4-bit architecture
- **4-bit DOS/Monitor** - Simple command interpreter and file system
- **Serial/TTY interface** - Interactive console I/O
- **Era-appropriate peripherals** - Hardware from the 1970-1980 era
- **Sample applications** - Games, calculators, utilities

---

## Quick Start

### Prerequisites

- CMake 3.10+
- C++17 compatible compiler (GCC 7+, Clang 6+, MSVC 2019+)
- Git (for cloning and submodules)

### Build

```bash
git clone https://github.com/Oichkatzelesfrettschen/k4004.git
cd k4004
git submodule update --init --recursive
mkdir build && cd build
cmake ..
make -j$(nproc)
```

For detailed build instructions, see [requirements.md](requirements.md).

### Run Tests

```bash
cd build
ctest --output-on-failure
```

### Assemble and Run a Program

```bash
# Assemble
./assembler examples/program.asm -o program.bin

# Run
./mcs4_emulator program.bin
```

---

## Architecture

The emulator is organized into modular components:

```
k4004/
├── assembler/          # Assembler and disassembler
├── emulator_core/      # CPU emulation (4004 and 4040)
├── emulator_apps/      # Emulator application
├── shared/             # Shared definitions
├── programs/           # Sample programs
├── docs/               # Documentation and Intel manuals
└── third_party/        # Dependencies (gtest)
```

For detailed architecture, see [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

---

## Documentation

- **[requirements.md](requirements.md)** - Build requirements and instructions
- **[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)** - System architecture overview
- **[docs/TECHNICAL_AUDIT.md](docs/TECHNICAL_AUDIT.md)** - Comprehensive technical analysis
- **[assembler/README.md](assembler/README.md)** - Assembler syntax and usage
- **Intel Manuals** (in `docs/`):
  - MCS-4 Users Manual (Feb 1973)
  - MCS-4 Assembly Programming Manual (Dec 1973)
  - MCS-40 Users Manual (Nov 1974)
  - MCS-40 Advance Specifications (Sep 1974)

---

## Recent Improvements (v2.0)

### Critical Bug Fixes
- ✅ Fixed 4004 stack size (4 → 3 levels)
- ✅ Fixed 4040 stack size (8 → 7 levels)
- ✅ Fixed PC addressing (10-bit → 12-bit for 4004, 13-bit for 4040)
- ✅ Added stack bounds checking (JMS, BBL)
- ✅ Fixed BBL instruction (now loads immediate data correctly)

### New Features
- ✅ Implemented all 14 missing 4040 instructions
- ✅ Added interrupt system (EIN, DIN, INT pin)
- ✅ Added halt mode (HLT instruction, halt state)
- ✅ Implemented register bank switching (SB0, SB1)
- ✅ Implemented ROM bank switching (DB0, DB1)
- ✅ Added command register for LCR instruction
- ✅ Added SRC backup for BBS (interrupt return)

### Documentation
- ✅ Created comprehensive technical audit
- ✅ Created detailed architecture documentation
- ✅ Created build requirements guide
- ✅ Updated README with expanded vision

---

## Instruction Set

### Intel 4004 (46 Instructions)

| Category | Instructions |
|----------|--------------|
| **Arithmetic** | ADD, SUB, INC, DAC, IAC, DAA |
| **Logical** | CMA, RAL, RAR, CLC, CMC, STC, TCC, TCS |
| **Transfer** | LD, XCH, LDM, BBL |
| **Branch** | JCN, JUN, JMS, JIN, ISZ |
| **Memory** | WRM, RDM, WR0-3, RD0-3, WMP, SBM, ADM, RDR, WRR |
| **Special** | NOP, KBP, DCL, SRC, FIM, FIN, WPM |

### Intel 4040 Additional Instructions (+14)

| Category | Instructions |
|----------|--------------|
| **Logical** | OR4, OR5, AN6, AN7 |
| **Bank Select** | DB0, DB1, SB0, SB1 |
| **Interrupt** | EIN, DIN, BBS |
| **Special** | HLT, LCR, RPM |

---

## CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_TESTS` | `ON` | Build test suites (requires gtest) |
| `STATIC_ANALYSIS` | `OFF` | Enable clang-tidy static analysis |

Example:
```bash
cmake -DCMAKE_BUILD_TYPE=Release -DSTATIC_ANALYSIS=ON ..
```

---

## Testing

The project includes comprehensive test suites:

- **Instruction tests** - Validate all 60 instructions
- **CPU tests** - Test state transitions and edge cases
- **Assembler tests** - Verify assembly and disassembly
- **Integration tests** - Test complete programs

Run all tests:
```bash
cd build
ctest -V
```

---

## Contributing

This project welcomes contributions! Areas of interest:

- Additional 4-bit programs and demos
- 4003 shift register implementation
- 4-bit BASIC interpreter design
- DOS/monitor system architecture
- Performance optimizations
- Additional test coverage
- Documentation improvements

---

## Historical Context

The Intel 4004, introduced in November 1971, was the world's first commercially available microprocessor. Designed for the Busicom calculator, it revolutionized computing by putting a complete CPU on a single chip.

The Intel 4040, released in 1974, enhanced the 4004 with:
- Larger stack (3 → 7 levels)
- More registers (16 → 24 via banking)
- Interrupt support
- Additional instructions
- Improved addressing (4KB → 8KB ROM)

This emulator preserves and celebrates this pioneering technology while exploring "what if" scenarios with enhanced configurations.

---

## License

See [LICENSE](LICENSE) file for details.

---

## Acknowledgments

- Intel Corporation for the original MCS-4 and MCS-40 architecture
- The retro computing community for preservation efforts
- All contributors to this project

---

## Future Roadmap

### Phase 3: Software Stack (Planned)
- [ ] 4-bit BASIC interpreter
  - Variable storage
  - Expression evaluation
  - Control flow (IF, GOTO, FOR)
  - I/O routines
- [ ] 4-bit DOS/Monitor
  - Command line interface
  - Memory inspection and editing
  - Simple file system
  - Device drivers
- [ ] Interactive development environment
  - Debugger with breakpoints
  - Register/memory viewer
  - Step-through execution
- [ ] Additional hardware
  - 4003 shift register
  - Serial interface
  - Display controller
  - Storage (tape/disk emulation)

### Phase 4: Advanced Features (Planned)
- [ ] GUI application with visual debugger
- [ ] Network support for multi-system emulation
- [ ] FPGA implementation for hardware acceleration
- [ ] Historical software preservation and porting

---

**Project Status:** Active Development
**Current Version:** 2.0 (Major accuracy and feature update)
**Next Milestone:** 4-bit BASIC interpreter (Phase 3)

---

**AD ASTRA PER MATHEMATICA ET SCIENTIAM**

*Building on the shoulders of giants - celebrating 50+ years of microprocessor history.*

# Bare Metal RISC-V C++ Hello World for Zisk VM

A minimal C++ application for bare metal RISC-V (rv64im) that runs on the Zisk zero-knowledge virtual machine.

## Features

- Bare metal C++ with newlib support
- C++ global constructors/destructors
- Custom linker script for Zisk VM memory layout
- Clean exit via syscall (no error messages)
- Integer-only (no floating-point)

## Prerequisites

- RISC-V GNU toolchain (`riscv64-unknown-elf-gcc`)
  - On macOS with Homebrew: `brew tap riscv-software-src/riscv; brew install riscv-gnu-toolchain`
- Rust toolchain (for building Zisk VM)
  - Install from: https://rustup.rs/
  - Or run: `curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh`
- curl or wget (for downloading newlib)
- Git
- Make

## Building

### First Time Setup

1. Build and install newlib:
```bash
./build-newlib.sh
```

This will:
- Download newlib 4.4.0.20231231
- Configure it for rv64im with medany code model
- Build with newlib-nano optimizations
- Install to `newlib-rv64im/`

2. Build Zisk VM emulator:
```bash
./build-zisk.sh
```

This will:
- Clone the Zisk VM repository from GitHub
- Build only the emulator (without proving support to avoid CUDA/libgmp dependencies)
- Install to `zisk/target/release/ziskemu`

**Note**: Building without proving support keeps dependencies simple and build times fast.

### Build the Application

```bash
make
```

This produces:
- `hello_world.elf` - ELF executable for Zisk VM
- `hello_world.bin` - Raw binary image

### Other Make Targets

- `make clean` - Remove build artifacts
- `make disasm` - Generate disassembly listing
- `make size` - Show detailed size information
- `make run` - Build and run the program with ziskemu

## Running

With the locally built ziskemu:
```bash
./zisk/target/release/ziskemu -e hello_world.elf
```

The program will output:
```
C++ Constructor called
Hello, World from bare metal RISC-V!
This is a C++ application with newlib support
Hello from C++ class!
C++ Destructor called
```

The program exits cleanly with no error messages.

## Project Structure

```
.
├── build-newlib.sh   # Script to fetch and build newlib
├── build-zisk.sh     # Script to fetch and build Zisk VM emulator
├── Makefile          # Build configuration
├── link.ld           # Linker script for Zisk VM memory layout
├── start.S           # Assembly startup code
├── syscalls.c        # Newlib syscall stubs with Zisk UART output
├── main.cpp          # C++ application code
├── newlib-rv64im/    # Newlib installation (created by build-newlib.sh)
└── zisk/             # Zisk VM repository (created by build-zisk.sh)
```

## Architecture Details

- **Target**: riscv64-unknown-elf
- **Architecture**: rv64im (64-bit RISC-V with Integer and Multiply extensions)
- **ABI**: lp64
- **Code Model**: medany (position-independent code for high memory addresses)
- **Newlib**: nano variant (integer-only printf/scanf, optimized for embedded)


# Zisk Specific Details

## Memory Layout

The linker script (`link.ld`) configures memory for the Zisk VM:

- **ROM**: 0x80000000 - 0x90000000
  - `.text` - Code
  - `.rodata` - Read-only data
  - Physical storage for initialized data

- **RAM**: 0xa0020000 - 0xc0000000
  - `.data` - Initialized data (copied from ROM at startup)
  - `.bss` - Uninitialized data
  - Heap
  - Stack (1MB)

### Exit Mechanism

The program uses Zisk VM syscall 93 to exit cleanly:
```asm
li a7, 93    # Exit syscall number
ecall        # Invoke syscall
```

### Data Section Layout

The linker script uses `>ram AT>rom` for the `.data` section, which tells the linker:
- Store initialized data in ROM in the ELF file (PhysAddr)
- Code expects to access it in RAM (VirtAddr)
- The Zisk VM loader copies data from ROM to RAM at startup

This is critical for C++ global constructors to work correctly.

## Customization

### Modifying the Application

Edit `main.cpp` to change the application behavior. The current example demonstrates:
- C++ classes with constructors/destructors
- Global object initialization
- Newlib printf support (note: output not visible without I/O implementation)

### Linker Script

The linker script (`link.ld`) is based on the Polygon Hermez Rust Zisk implementation. Modify it if you need different memory regions or section layouts.

### Console Output

The `syscalls.c` file implements console output using Zisk VM's memory-mapped UART at address `0xa0000200`. The `_write()` function writes each character to this address, which the Zisk VM emulator captures and displays on stdout. This allows printf and other newlib output functions to work correctly.

### Syscall Implementation Status

The `syscalls.c` file provides newlib syscall stubs required for linking. However, most are **minimal stub implementations** suitable only for this hello world example:

**Fully Implemented:**
- `_sbrk()` - Heap allocation (basic implementation)
- `_write()` - Console output via Zisk VM UART at 0xa0000200

**Stubbed (Not Implemented):**
- `_close()` - Always returns -1 (not supported)
- `_fstat()` - Returns character device mode, no actual file stats
- `_isatty()` - Always returns 1 (assumes all files are terminals)
- `_lseek()` - Always returns 0 (no seek support)
- `_read()` - Always returns 0 (no input support)
- `_exit()` - Infinite WFI loop (actual exit handled by syscall 93 in start.S)
- `_kill()` - Always fails with EINVAL
- `_getpid()` - Always returns 1
- `_fputwc_r()` - Wide character stub, always returns WEOF

**For Production Applications:**
You would need to implement these syscalls properly based on your application's requirements. The Zisk VM provides additional capabilities through ecall syscalls including Ethereum precompiles (SHA256, SHA3, RIPEMD160, Blake2f, elliptic curve operations, etc.). See the header comment in `syscalls.c` for details.

## References

- [Newlib Documentation](https://sourceware.org/newlib/)
- [RISC-V ISA Specification](https://riscv.org/technical/specifications/)
- [Zisk VM Repository](https://github.com/0xPolygonHermez/zisk)

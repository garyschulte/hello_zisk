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
  - On macOS with Homebrew: `brew install riscv-gnu-toolchain`
- curl or wget (for downloading newlib)
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

## Running

With ziskemu:
```bash
/path/to/ziskemu -e hello_world.elf
```

The program should exit cleanly with no error messages.

## Project Structure

```
.
├── build-newlib.sh   # Script to fetch and build newlib
├── Makefile          # Build configuration
├── link.ld           # Linker script for Zisk VM memory layout
├── start.S           # Assembly startup code
├── syscalls.c        # Newlib syscall stubs
├── main.cpp          # C++ application code
└── newlib-rv64im/    # Newlib installation (created by build-newlib.sh)
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

### Syscalls

The `syscalls.c` file provides minimal stubs for newlib. The `_write()` function currently returns success without actually outputting anything. To see printf output, you would need to implement the actual output mechanism for Zisk VM.

## References

- [Newlib Documentation](https://sourceware.org/newlib/)
- [RISC-V ISA Specification](https://riscv.org/technical/specifications/)
- [Zisk VM Repository](https://github.com/0xPolygonHermez/zisk)

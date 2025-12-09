# Bare Metal RISC-V 64-bit Hello World

This project implements a C++ hello world application for bare metal RISC-V 64-bit (rv64im) targets, using newlib for C standard library support.

## Architecture

- **Target**: rv64im (RISC-V 64-bit with Integer and Multiply extensions)
- **ABI**: lp64 (64-bit long and pointers, no floating point)
- **C Library**: newlib (with custom syscall stubs)

## Project Structure

- `start.S` - Startup assembly code that:
  - Sets up the stack pointer
  - Zeros the .bss section
  - Calls C++ global constructors
  - Calls main()
  - Calls C++ global destructors

- `syscalls.c` - Newlib syscall stubs including:
  - `_sbrk` - Heap memory allocation
  - `_write` - Output (needs hardware-specific implementation)
  - `_read`, `_close`, `_fstat`, etc. - File operations stubs
  - `_exit` - Program termination

- `main.cpp` - C++ hello world application demonstrating:
  - C-style printf
  - C++ iostream
  - Global constructors/destructors

- `link.ld` - Linker script defining:
  - Memory layout (128MB RAM @ 0x80000000)
  - Section placement
  - Stack location (64KB stack)
  - C++ init/fini array sections

## Prerequisites

You need the RISC-V GNU toolchain installed. Install it using:

### macOS (using Homebrew)
```bash
brew tap riscv/riscv
brew install riscv-tools
```

### Linux (Ubuntu/Debian)
```bash
sudo apt-get install gcc-riscv64-unknown-elf
```

### Building from source
```bash
git clone https://github.com/riscv/riscv-gnu-toolchain
cd riscv-gnu-toolchain
./configure --prefix=/opt/riscv --with-arch=rv64im --with-abi=lp64
make
```

## Building

Simply run:
```bash
make
```

This will produce:
- `hello_world.elf` - ELF executable
- `hello_world.bin` - Raw binary

## Additional Make Targets

- `make disasm` - Generate disassembly listing
- `make size` - Show section sizes
- `make clean` - Remove build artifacts

## Customizing Output

The `_write` syscall in `syscalls.c:22` is currently a stub. You need to implement actual output based on your target:

### For Spike Simulator (HTIF)
```c
// Use HTIF tohost register
#define HTIF_TOHOST 0x40008000
volatile uint64_t *tohost = (uint64_t *)HTIF_TOHOST;

int _write(int file, char *ptr, int len) {
    for (int i = 0; i < len; i++) {
        *tohost = 0x0101000000000000ULL | ptr[i];
    }
    return len;
}
```

### For UART Output
```c
#define UART_BASE 0x10000000
volatile uint8_t *uart = (uint8_t *)UART_BASE;

int _write(int file, char *ptr, int len) {
    for (int i = 0; i < len; i++) {
        uart[0] = ptr[i];
    }
    return len;
}
```

### For ZKVM
Implement using your ZKVM's specific syscall mechanism.

## Running

The execution method depends on your target:

### Spike Simulator
```bash
spike --isa=rv64im hello_world.elf
```

### QEMU
```bash
qemu-system-riscv64 -machine virt -bios none -kernel hello_world.elf -nographic
```

### Custom Hardware
Flash `hello_world.bin` to your device according to its programming method.

## Memory Map

```
0x80000000 - Code (.text)
           - Read-only data (.rodata)
           - Initialized data (.data)
           - Uninitialized data (.bss)
           - Heap (grows upward from __heap_start)
0x87FF0000 - Stack (64KB, grows downward from __stack_top)
```

## Notes

- The application uses `-fno-exceptions` and `-fno-rtti` to minimize code size
- C++ iostreams require more code space than printf
- Global constructors/destructors are fully supported
- No atomic or floating-point operations (not in rv64im)

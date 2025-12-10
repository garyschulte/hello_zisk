#include <sys/stat.h>
#include <sys/types.h>
#include <sys/reent.h>
#include <errno.h>
#include <wchar.h>

/*
 * Zisk VM Syscalls and Memory-Mapped I/O
 * =======================================
 *
 * This file implements newlib syscall stubs for the Zisk zero-knowledge VM.
 *
 * MEMORY-MAPPED I/O:
 * ------------------
 * - UART Output: 0xa0000200 (SYS_ADDR + 0x200)
 *   Writing a single byte to this address outputs to stdout
 *   Used by _write() for console output
 *
 * - Registers: 0xa0000000 - 0xa00000ff (SYS_ADDR)
 *   First 256 bytes store 32 8-byte RISC-V registers
 *
 * - Float Registers: 0xa0001000 (FREG_FIRST)
 * - CSR Registers: 0xa0008000 (CSR_ADDR)
 * - Output Data: 0xa0010000 (OUTPUT_ADDR)
 * - Input Data: 0x90000000 (INPUT_ADDR, read-only)
 *
 * SYSCALLS VIA ECALL:
 * -------------------
 * The Zisk VM supports syscalls via the RISC-V ecall instruction.
 * Syscall number in register a7, arguments in a0-a6.
 *
 * Implemented in this file:
 * - Syscall 93: exit - Clean program termination (see start.S)
 * - _write (via UART): Console output for stdout/stderr
 *
 * Available but NOT implemented in this hello world:
 * - Ethereum precompiles (via ecall with specific syscall numbers):
 *   * SHA256, SHA3 (Keccak256)
 *   * RIPEMD160
 *   * Blake2f
 *   * Modular exponentiation
 *   * Elliptic curve operations (ecrecover, ecadd, ecmul, ecpairing)
 *   * BN254/BLS12-381 pairing operations
 *
 * These precompiles are available through the Zisk VM's ecall interface
 * and can be accessed by setting up registers and invoking ecall.
 * See the Zisk VM documentation and lib-c examples for usage.
 *
 * For a production application, you would implement wrapper functions
 * that set up the proper register values and invoke ecall to access
 * these cryptographic precompiles.
 */

#undef errno
extern int errno;

/* Heap management */
extern char __heap_start;
static char *heap_end = 0;

void *_sbrk(int incr) {
    char *prev_heap_end;

    if (heap_end == 0) {
        heap_end = &__heap_start;
    }

    prev_heap_end = heap_end;
    heap_end += incr;

    return (void *)prev_heap_end;
}

/* Write to file descriptor
 * For Zisk VM, write to memory-mapped UART at 0xa0000200
 * Each byte write outputs one character to stdout
 */
int _write(int file, char *ptr, int len) {
    // Zisk VM UART address for console output
    volatile char *uart = (volatile char *)0xa0000200;

    // Only handle stdout (1) and stderr (2)
    if (file != 1 && file != 2) {
        errno = EBADF;
        return -1;
    }

    // Write each byte to the UART
    for (int i = 0; i < len; i++) {
        *uart = ptr[i];
    }

    return len;
}

int _close(int file) {
    (void)file;
    return -1;
}

int _fstat(int file, struct stat *st) {
    (void)file;
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file) {
    (void)file;
    return 1;
}

int _lseek(int file, int ptr, int dir) {
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}

int _read(int file, char *ptr, int len) {
    (void)file;
    (void)ptr;
    (void)len;
    return 0;
}

void _exit(int status) {
    (void)status;
    /* Hang forever */
    while(1) {
        __asm__ volatile ("wfi");
    }
}

int _kill(int pid, int sig) {
    (void)pid;
    (void)sig;
    errno = EINVAL;
    return -1;
}

int _getpid(void) {
    return 1;
}

/* Wide character stub for newlib-nano */
wint_t _fputwc_r(struct _reent *ptr, wchar_t wc, FILE *fp) {
    (void)ptr;
    (void)wc;
    (void)fp;
    return WEOF;
}

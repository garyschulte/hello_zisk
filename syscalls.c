#include <sys/stat.h>
#include <sys/types.h>
#include <sys/reent.h>
#include <errno.h>
#include <wchar.h>

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
 * For bare metal, fd 1 (stdout) and 2 (stderr) could output to UART or HTIF
 * This is a basic stub - you'll need to implement actual hardware output
 */
int _write(int file, char *ptr, int len) {
    /* TODO: Implement actual output mechanism
     * Options:
     * 1. UART output for physical hardware
     * 2. HTIF for simulators like Spike
     * 3. Custom syscall for ZKVM
     */

    // For now, just pretend we wrote everything
    return len;
}

int _close(int file) {
    return -1;
}

int _fstat(int file, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file) {
    return 1;
}

int _lseek(int file, int ptr, int dir) {
    return 0;
}

int _read(int file, char *ptr, int len) {
    return 0;
}

void _exit(int status) {
    /* Hang forever */
    while(1) {
        __asm__ volatile ("wfi");
    }
}

int _kill(int pid, int sig) {
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

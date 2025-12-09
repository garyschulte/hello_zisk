.section .text.init
.global _start

_start:
    # Set up stack pointer
    la sp, __stack_top

    # Clear bss section
    la a0, __bss_start
    la a1, __bss_end
    bgeu a0, a1, bss_done
bss_loop:
    sd zero, 0(a0)
    addi a0, a0, 8
    bltu a0, a1, bss_loop
bss_done:

    # Call global constructors
    call __libc_init_array

    # Call main
    call main

    # Call global destructors
    call __libc_fini_array

    # Exit loop (hang)
exit_loop:
    wfi
    j exit_loop

.size _start, .-_start

# Dhaka-FreeRTOS
Proposal of an implementation of SoftBound + CETS in Kage-FreeRTOS for memory safety at the source code level, to protect against data-only exploits.
Currently, the prototype demonstrates the one-line instrumentations on 32-bit x86 Linux, which should be easy to port to FreeRTOS, being written fully in C.

Read the paper at `Dhaka.pdf`!

## Build
`make all` - compiles the Linux prototype, then runs it in 32-bit mode, no ASLR/RELRO etc., 4 GB memory
`make dbg` - gdb!
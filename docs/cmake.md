# Build the benchmark with CMake

## Background

Our source tree is managed by `CMake` instead of a hand-written Makefile. CMake is widely used to build large codebases. To us, CMake eases building multiple versions of the same source code, each built with difference build flags. We will see the benefits in subsequent experiments. 

To use CMake, we list our build targets in a file called `CMakeLists.txt`. Then we invoke `cmake`, which will take `CMakeLists.txt` and  generate a `makefile` . `cmake` will then invoke make automatically. 

## **First time build**

```
git clone https://github.com/fxlin/p2-concurrency
# assuming we are building exp1
$ cd exp1
$ cmake .
```

This generates a Makefile from CMakeLists.txt. Then type:
```
$ make
```

**Troubleshooting** if CMakeCache.txt is complained to be out of date, simply delete or rename it and run `cmake` again. 

Peek at the generated Makefile to see whether you can understand it. 

## **Each time you modify source and need to rebuild**

Simply run: `make `

To build individual targets: 

```
$ make <tab> <tab> # this will list all targets
# examples: 
$ make counter
$ make clean
```

To see what commands are actually invoked by Make, do 

```$ make VERBOSE=1```

## Optional: cross-build for aarch64

Clean up all cmake intermediate files: 

```
$ rm -rf CMakeCache.txt  CMakeFiles  cmake_install.cmake
```

Then invoke cmake to regenerate for aarch64: 

```
$ cmake . -DCMAKE_ENV=aarch64
```
Build everything: 
```
$ make
```
Check the resultant binaries, which should show "ARM aarch64"
```
$ file counter
counter: ELF 64-bit LSB shared object, ARM aarch64, version 1 (SYSV), dynamically linked, interpreter /lib/ld-linux-aarch64.so.1, for GNU/Linux 3.7.0, BuildID[sha1]=f258ebadfacd9e41bc0425c6446ae419278fb50e, with debug_info, not stripped
```

Now you can disassemble the ARM64 program with `aarch64-linux-gnu-objdump`. e.g.

```
$ aarch64-linux-gnu-objdump -dS counter | more
counter:     file format elf64-littleaarch64

Disassembly of section .init:

0000000000000f90 <_init>:
 f90:   a9bf7bfd        stp     x29, x30, [sp, #-16]!
 f94:   910003fd        mov     x29, sp
 f98:   94000137        bl      1474 <call_weak_fn>
 f9c:   a8c17bfd        ldp     x29, x30, [sp], #16
 fa0:   d65f03c0        ret

Disassembly of section .plt:

0000000000000fb0 <.plt>:
     fb0:       a9bf7bf0        stp     x16, x30, [sp, #-16]!
     fb4:       b0000090        adrp    x16, 11000 <__FRAME_END__+0xf658>
     fb8:       f9478211        ldr     x17, [x16, #3840]
     fbc:       913c0210        add     x16, x16, #0xf00
     fc0:       d61f0220        br      x17
     fc4:       d503201f        nop
     fc8:       d503201f        nop
     fcc:       d503201f        nop
< ... >     
```




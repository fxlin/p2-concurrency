# Build the benchmark with CMake

The build is managed via CMake instead of hand-written Makefile. Using CMake eases building multiple targets with difference combinations of flags. We will see the benefits in subsequent experiments. 

To use CMake, we describe our build targets in a file CMakeLists.txt. Then we invoke `cmake` to generate a `makefile` automatically. 

## **First time build**

`$  cmake .`

This generates Makefile from CMakeLists.txt. You may take a look at Makefile to see whether it makes sense.  

## **Each time you modify source and rebuild**

Simply run: 

`$ make `

You may build individual targets: 

```
$ make counter
$ make clean
$ make <tab> <tab> # this will list all targets
```

To see what commands are actually invoked, do 

```$ make VERBOSE=1```

## Build for aarch64

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

Now you can disassemble the ARM64 program with `aarch64-linux-gnu-objdump`. 
#!/bin/bash
filename=$1
llc -o ${filename}.s $filename
as -o ${filename}.o ${filename}.s
ld \
    -dynamic-linker \
    /lib64/ld-linux-x86-64.so.2 \
    -o a.out \
    /usr/lib/crt1.o \
    /usr/lib/crti.o \
    /usr/lib/gcc/x86_64-pc-linux-gnu/7.2.0/crtbegin.o \
    ${filename}.o \
    -L /usr/lib/gcc/x86_64-pc-linux-gnu/7.2.0/ \
    -lstdc++ \
    -lc \
    /usr/lib/gcc/x86_64-pc-linux-gnu/7.2.0/crtend.o \
    /usr/lib/crtn.o


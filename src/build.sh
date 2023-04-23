#!/bin/sh
if [ -z $CC ]
then
	CC=cc
fi
$CC -g fuse-main.c `pkg-config --libs --cflags fuse` -o fuse-main.o
 

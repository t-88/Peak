#!/bin/sh
rm output/*
set -eux
gcc main.c -o main -lpthread `pkg-config --cflags gtk+-3.0  --libs gtk+-3.0 x11`
./main
#!/bin/sh

set -eux

gcc main.c -o main -lpthread `pkg-config --cflags gtk+-3.0 gdk-3.0  --libs gtk+-3.0 x11 gdk-3.0`
./main
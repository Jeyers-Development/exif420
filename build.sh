#!/bin/bash

set -e

mkdir -p out

g++ -c src/exif420.cpp -o out/exif420.o -Iinclude -w -fPIC

g++ -shared -o out/libexif420.so -Iinclude out/exif420.o -w

ar rcs out/libexif420.a out/exif420.o

rm out/exif420.o

echo "Build complete (Warnings suppressed)."
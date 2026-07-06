#!/bin/bash

set -e

mkdir -p out
cd out

g++ -c ../exif420.cpp -o exif420.o -w -fPIC

g++ -shared -o libexif420.so exif420.o -w

ar rcs libexif420_linux.a exif420.o

rm exif420.o

echo "Build complete. Created shared and static libraries in out/:"
echo "  - libexif420.so"
echo "  - libexif420_linux.a"
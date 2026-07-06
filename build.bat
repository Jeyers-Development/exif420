@echo off
setlocal enabledelayedexpansion

mkdir out
cd out

g++ -c ../exif420.cpp -o exif420.o -w

g++ -shared -o exif420.dll exif420.o -w -Wl,--out-implib,libexif420.a

rm exif420.o

echo Build complete (Warnings suppressed).
pause
@echo off
setlocal enabledelayedexpansion

if not exist out mkdir out

g++ -c src/exif420.cpp -o out/exif420.o -Iinclude -w 

g++ -shared -o out/exif420.dll -Iinclude out/exif420.o -w -Wl,--out-implib,out/libexif420.a

rm out/exif420.o

echo Build complete (Warnings suppressed).
pause
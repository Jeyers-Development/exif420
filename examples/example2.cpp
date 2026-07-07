#include "exif420.h"
#include <iostream>
#include <vector>
#include <fstream>

int main() {
    std::ifstream file("my_photo.jpg", std::ios::binary);
    if (!file) {
        std::cerr << "CRITICAL ERROR: Could not open file 'my_photo.jpg'" << std::endl;
        return 1;
    }

    std::vector<uint8_t> file_bytes((std::istreambuf_iterator<char>(file)), 
                                     std::istreambuf_iterator<char>());

    exif420::ExifData photo_data = exif420::ExtractExifData(file_bytes);

    std::cout << photo_data.dateTime;

    return 0;
}
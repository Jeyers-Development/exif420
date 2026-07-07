#ifndef EXIF420_H
#define EXIF420_H

#include <vector>
#include <string>
#include <cstdint>

#if defined(_WIN32) || defined(_WIN64)
    #ifdef EXIF420_STATIC
        #define EXIF_API
    #elif defined(EXIF_EXPORT)
        #define EXIF_API __declspec(dllexport)
    #else
        #define EXIF_API __declspec(dllimport)
    #endif
#else
    #if __GNUC__ >= 4
        #define EXIF_API __attribute__((visibility("default")))
    #else
        #define EXIF_API
    #endif
#endif

namespace exif420 {
    struct ExifData {
        std::string dateTime;
        std::string make;
        std::string model;
        std::string software;

        double exposureTime; 
        double fNumber;  
        int32_t iso;   
        double focalLength;   

        uint32_t width;
        uint32_t height;
        int16_t orientation;  
    };

    EXIF_API uint32_t ReadInt(const uint8_t* buf, size_t offset, bool intel, int bytes);
    
    EXIF_API ExifData ExtractExifData(const std::vector<uint8_t>& buf);

    EXIF_API void PrintExifData(const ExifData& data);
}

#endif
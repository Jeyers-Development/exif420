#ifndef EXIF420_H
#define EXIF420_H

#include <vector>
#include <string>
#include <cstdint>

// If EXIF420_EXPORTS is defined, we are building the DLL. 
// Otherwise, we assume we are importing from it.
#if defined(_WIN32) || defined(_WIN64)
    #ifdef EXIF_EXPORT
        #define EXIF_API __declspec(dllexport)
    #else
        #define EXIF_API __declspec(dllimport)
    #endif
#else
    // On Linux/macOS, we don't need __declspec
    #if __GNUC__ >= 4
        #define EXIF_API __attribute__((visibility("default")))
    #else
        #define EXIF_API
    #endif
#endif
namespace exif420 {
    EXIF_API uint32_t ReadInt(const uint8_t* buf, size_t offset, bool intel, int bytes);
    EXIF_API std::string ExtractNefDate(const std::vector<uint8_t>& buf);
}

#endif
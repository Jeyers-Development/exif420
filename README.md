
# JEY exif420 v1
The simple, lightweight, &amp; fast EXIF Reader Library for C++

### BIG NOTE: V1.0 ONLY SUPPORTS DATE & TIME INFORMATION!!!
### Another note: ExtractNefData() is for all file types supported not just NEF files.

Everything lives inside the namespace `exif420`

Make sure you have your header file (`exif420.h`) included at the top of your code:

```
#include "exif420.h"
#include <iostream>
#include <vector>
```

To get the date from an image, pass a vector of bytes (`std::vector<uint8_t>`) into `exif420::ExtractNefDate()`.

## Functions

#### ExtractNefDate

```
std::string ExtractNefDate(const std::vector<uint8_t>& buf);
```

-   The main function you call to find and return the photo's date.
    

#### TryParseIfd

```
bool TryParseIfd(const std::vector<uint8_t>& buf, size_t ifd_offset, size_t tiff_start, bool intel, std::string& out_date);
```

-   Searches through the the file's metadata looking for date tags.
    

#### ReadInt

```
uint32_t ReadInt(const uint8_t* buf, size_t offset, bool intel, int bytes);
```

-   Converts raw file bytes into readable numbers them.
    

#### ReadAsciiValue

```
std::string ReadAsciiValue(const std::vector<uint8_t>& buf, size_t tiff_start, size_t entry_offset, uint32_t count, uint32_t val_offset);
```

-   Gets the actual date text out of the file and cleans it up.
    

#### LooksLikeDateTime


```
bool LooksLikeDateTime(const std::string& value);
```

-   Checks the text to make sure it actually looks like a date (e.g., `2026:07:06`)

## Full Code Example

```
#include "exif420.h"
#include <iostream>
#include <vector>
#include <fstream>

int main() {
    // Open your image file (JPEG or RAW)
    std::ifstream file("my_photo.jpg", std::ios::binary);
    if (!file) {
        std::cerr << "Could not open file\n";
        return 1;
    }

    // Read the file into a vector of bytes
    std::vector<uint8_t> file_bytes((std::istreambuf_iterator<char>(file)), 
                                     std::istreambuf_iterator<char>());

    // Use function to get the date string
    std::string photo_date = exif420::ExtractNefDate(file_bytes);

    // Use the result
    if (photo_date.empty()) {
        std::cout << "No date found in the specified image\n";
    } else {
        std::cout << "The photo was taken on: " << photo_date << "\n";
        // Output format will look like: "2026:07:06 14:30:00"
    }

    return 0;
}
```



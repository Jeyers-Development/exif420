#include "exif420.h"
#include <cstring>
#include <iostream>

namespace exif420 {

    uint32_t ReadInt(const uint8_t* buf, size_t offset, bool intel, int bytes) {
        uint32_t val = 0;
        if (intel) {
            for (int i = 0; i < bytes; ++i) val |= (static_cast<uint32_t>(buf[offset + i]) << (8 * i));
        } else {
            for (int i = 0; i < bytes; ++i) val = (val << 8) | buf[offset + i];
        }
        return val;
    }

    bool LooksLikeDateTime(const std::string& value) {
        if (value.size() < 10) return false;
        if (value[4] != ':' || value[7] != ':') return false;
        for (size_t i = 0; i < value.size(); ++i) {
            if (std::isdigit(static_cast<unsigned char>(value[i])) || value[i] == ':' || value[i] == ' ' || value[i] == 'T' || value[i] == '.' || value[i] == '-' || value[i] == '_') {
                continue;
            }
            return false;
        }
        return true;
    }

    std::string ReadAsciiValue(const std::vector<uint8_t>& buf, size_t tiff_start, size_t entry_offset, uint32_t count, uint32_t val_offset) {
        size_t value_pos = count <= 4 ? entry_offset + 8 : tiff_start + val_offset;
        if (value_pos + count > buf.size()) {
            return "";
        }

        std::string value(reinterpret_cast<const char*>(&buf[value_pos]), count);
        while (!value.empty() && (value.back() == '\0' || value.back() == ' ')) {
            value.pop_back();
        }
        return value;
    }

    bool TryParseIfd(const std::vector<uint8_t>& buf, size_t ifd_offset, size_t tiff_start, bool intel, ExifData& out_data) {
        const size_t buf_size = buf.size();
        if (ifd_offset + 2 > buf_size) {
            return false;
        }

        size_t num_entries = ReadInt(buf.data(), ifd_offset, intel, 2);
        size_t entry_ptr = ifd_offset + 2;

        for (size_t i = 0; i < num_entries; ++i) {
            if (entry_ptr + 12 > buf_size) {
                break;
            }

            uint16_t tag = static_cast<uint16_t>(ReadInt(buf.data(), entry_ptr, intel, 2));
            uint16_t type = static_cast<uint16_t>(ReadInt(buf.data(), entry_ptr + 2, intel, 2));
            uint32_t count = ReadInt(buf.data(), entry_ptr + 4, intel, 4);
            uint32_t val_offset = ReadInt(buf.data(), entry_ptr + 8, intel, 4);

            size_t type_size = 1;
            if (type == 3) type_size = 2;
            else if (type == 4 || type == 9) type_size = 4;
            else if (type == 5 || type == 10) type_size = 8;

            size_t data_size = count * type_size;

            size_t data_pos;
            if (data_size > 4) {
                data_pos = tiff_start + val_offset;
            } else {
                data_pos = entry_ptr + 8;
            }

            if ((tag == 0x0132 || tag == 0x9003 || tag == 0x9004 || tag == 0x0133) && type == 2) {
                std::string date_value = ReadAsciiValue(buf, tiff_start, entry_ptr, count, val_offset);
                if (!date_value.empty() && LooksLikeDateTime(date_value)) {
                    out_data.dateTime = date_value;
                }
            }

            else if (tag == 0x010F && type == 2) {
                out_data.make = ReadAsciiValue(buf, tiff_start, entry_ptr, count, val_offset);
            }

            else if (tag == 0x0110 && type == 2) {
                out_data.model = ReadAsciiValue(buf, tiff_start, entry_ptr, count, val_offset);
            }

            else if (tag == 0x0131 && type == 2) {
                out_data.software = ReadAsciiValue(buf, tiff_start, entry_ptr, count, val_offset);
            }

            else if ((tag == 0x829A || tag == 0x829D || tag == 0x920A) && type == 5) {
                if (data_pos + 8 <= buf_size) {
                    uint32_t num = ReadInt(buf.data(), data_pos, intel, 4);
                    uint32_t den = ReadInt(buf.data(), data_pos + 4, intel, 4);
                    
                    if (den != 0) {
                        double val = static_cast<double>(num) / static_cast<double>(den);
                        if (tag == 0x829A) out_data.exposureTime = val;
                        else if (tag == 0x829D) out_data.fNumber = val;
                        else out_data.focalLength = val;
                    }
                }
            }

            else if (tag == 0x8827 && (type == 3 || type == 4)) {
                out_data.iso = static_cast<int32_t>(ReadInt(buf.data(), data_pos, intel, (type == 3 ? 2 : 4)));
            }

            else if (tag == 0xA002 && (type == 3 || type == 4)) {
                out_data.width = ReadInt(buf.data(), data_pos, intel, (type == 3 ? 2 : 4));
            }
            else if (tag == 0xA003 && (type == 3 || type == 4)) {
                out_data.height = ReadInt(buf.data(), data_pos, intel, (type == 3 ? 2 : 4));
            }

            else if (tag == 0x0112 && type == 3) {
                out_data.orientation = static_cast<int16_t>(ReadInt(buf.data(), data_pos, intel, 2));
            }

            else if ((tag == 0x8769 || tag == 0x014A) && type == 4) {
                size_t sub_ifd_offset = tiff_start + val_offset;
                if (sub_ifd_offset + 2 <= buf_size) {
                    TryParseIfd(buf, sub_ifd_offset, tiff_start, intel, out_data);
                }
            }

            entry_ptr += 12;
        }

        return true;
    }

    ExifData ExtractExifData(const std::vector<uint8_t>& buf) {
        const size_t buf_size = buf.size();
        
        if (buf_size < 12) return {}; 

        if (buf[0] == 0xFF && buf[1] == 0xD8) {
            for (size_t i = 2; i + 4 < buf_size; ) {
                if (buf[i] != 0xFF) break;

                uint8_t marker = buf[i + 1];
                if (marker == 0xD9 || marker == 0x01) break;
                if (marker == 0xD8) {
                    i += 2;
                    continue;
                }

                if (i + 2 + 4 > buf_size) break;
                uint16_t segment_length = static_cast<uint16_t>((buf[i + 2] << 8) | buf[i + 3]);
                if (i + 2 + segment_length > buf_size) break;

                if (marker == 0xE1 && segment_length >= 8 && std::memcmp(&buf[i + 4], "Exif\0\0", 6) == 0) {
                    size_t exif_start = i + 10;
                    if (exif_start + 8 <= buf_size) {
                        bool intel = false;
                        if (buf[exif_start] == 0x49 && buf[exif_start + 1] == 0x49 && buf[exif_start + 2] == 0x2A && buf[exif_start + 3] == 0x00) {
                            intel = true;
                        } else if (buf[exif_start] == 0x4D && buf[exif_start + 1] == 0x4D && buf[exif_start + 2] == 0x00 && buf[exif_start + 3] == 0x2A) {
                            intel = false;
                        } else {
                            i += 2 + segment_length;
                            continue;
                        }

                        ExifData jpeg_data;
                        size_t ifd_offset = exif_start + ReadInt(buf.data(), exif_start + 4, intel, 4);
                        if (TryParseIfd(buf, ifd_offset, exif_start, intel, jpeg_data)) {
                            return jpeg_data;
                        }
                    }
                }

                i += 2 + segment_length;
            }
        }

        if (buf_size < 12) return {};

        bool intel = false;
        if (buf[0] == 0x49 && buf[1] == 0x49 && buf[2] == 0x2A && buf[3] == 0x00) {
            intel = true;
        } else if (buf[0] == 0x4D && buf[1] == 0x4D && buf[2] == 0x00 && buf[3] == 0x2A) {
            intel = false;
        } else {
            return {};
        }

        ExifData result;
        size_t ifd_offset = 0;
        
        if (buf[0] == 0x49 && buf[1] == 0x49) {
            ifd_offset = ReadInt(buf.data(), 4, intel, 4);
        } else {
            ifd_offset = ReadInt(buf.data(), 4, intel, 4);
        }

        if (TryParseIfd(buf, ifd_offset, 0, intel, result)) {
            return result; 
        }

        return {};
    }

    void PrintExifData(const ExifData& data) {
        if (!data.make.empty())        std::cout << "Make:         " << data.make << "\n";
        if (!data.model.empty())       std::cout << "Model:        " << data.model << "\n";
        if (!data.software.empty())    std::cout << "Software:     " << data.software << "\n";
        if (!data.dateTime.empty())    std::cout << "Date/Time:    " << data.dateTime << "\n";
        
        if (data.iso > 0)              std::cout << "ISO:          " << data.iso << "\n";
        if (data.fNumber > 0)          std::cout << "F-Number:     f/" << data.fNumber << "\n";
        if (data.exposureTime > 0)     std::cout << "Exposure:     " << data.exposureTime << "s\n";
        if (data.focalLength > 0)      std::cout << "Focal Length: " << data.focalLength << "mm\n";
        
        if (data.width > 0 && data.height > 0) {
            std::cout << "Dimensions:   " << data.width << "x" << data.height << "\n";
        }
    }
}

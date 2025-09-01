#include <crc8.h>

uint8_t crc8(const uint8_t* data, std::size_t len) 
{
    uint8_t crc = 0x00;
    for (std::size_t i = 0; i < len; ++i) 
    {
        crc ^= data[i];
        // bit iteration over polynomial 0x07
        for (int b = 0; b < 8; ++b) 
        {
            if (crc & 0x80) crc = (uint8_t)((crc << 1) ^ 0x07);
            else crc <<= 1;
        }
    }
    return crc;
}

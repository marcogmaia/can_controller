#include <stdint.h>
#include <assert.h>
#include "can_utils.h"


/* support a max of 255 characters (buffer size of 256) */
void bitarray_to_str(char *dst, const uint8_t *src, const uint32_t size) {
    assert(size < 256);
    char buffer[256] = {0};
    for(uint32_t i = 0; i < size; ++i) {
        buffer[i] = (src[i] == 0) ? '0' : '1';
    }
    strncpy(dst, buffer, size);
}


uint32_t crc15(const uint8_t *bitarray, const uint32_t size) {
    // uint8_t res[15] = {0};
    uint8_t crc[15] = {0};

    for(uint32_t i = 0; i < size; ++i) {
        uint8_t invert = bitarray[i] ^ crc[14];
        crc[14]        = crc[13] ^ invert;
        crc[13]        = crc[12];
        crc[12]        = crc[11];
        crc[11]        = crc[10];
        crc[10]        = crc[9] ^ invert;
        crc[9]         = crc[8];
        crc[8]         = crc[7] ^ invert;
        crc[7]         = crc[6] ^ invert;
        crc[6]         = crc[5];
        crc[5]         = crc[4];
        crc[4]         = crc[3] ^ invert;
        crc[3]         = crc[2] ^ invert;
        crc[2]         = crc[1];
        crc[1]         = crc[0];
        crc[0]         = invert;
    }
    uint32_t res = 0x0;
    for(uint32_t i = 0; i < 15; ++i) {
        res |= crc[i] << i;
    }
    return res;
}

// void byte_to_binary(uint8_t *dst, uint8_t src) {
//     for(uint8_t i = 0; i < 8; ++i) {
//         dst[i] = src & (1 << i);
//     }
// }

// static void reverse_array(uint8_t *dst, uint8_t *src, uint32_t size) {
//     uint8_t r[256] = {0};
//     for(uint32_t i = 0; i < size; ++i) {
//         r[i] = src[size - i - 1];
//     }
//     memcpy(dst, src, size);
// }


uint32_t bitarray_to_int(const uint8_t *bitarr, uint8_t size) {
    // *num = 0;
    uint32_t num = 0;
    for(uint32_t i = 0; i < size; ++i) {
        num |= bitarr[i] << (size - 1 - i);
    }
    return num;
}

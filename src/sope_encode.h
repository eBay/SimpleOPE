/******************************************************************
Copyright 2019 eBay Inc.
Architect/Developer(s): Gene Zhang, Jung-Sang Ahn, Kun Ren

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
******************************************************************/
#pragma once

#include "endian_encode.h"

#include <cstdint>
#include <cstring>
#include <cassert>
#include <stdlib.h>

namespace sope {

// padding length for strings and bytes
#define STRING_PAD_LEN 2
#define BINARY_PAD_LEN 2

// order-preserving encoding with byte array comparison
//  for example, integer 0x12345678 is stored as:
//    high         low
//     12  34  56  78
// Byte comparison is from low to high, so 
// for little endian such as Intel CPU, type
// order needs to be swapped.

/** Ascending                   descending:
*   x80000000     x00000000    x80000000     xFFFFFFFF
*   xFFFFFFFF /=\ x7FFFFFFF    xFFFFFFFF /=\ x80000000
*   x00000000 \=/ x80000000    x00000000 \=/ x7FFFFFFF
*   x7FFFFFFF     xFFFFFFFF    x7FFFFFFF     x00000000
*/

inline uint32_t encode(int ii, bool asc = true) {
    uint32_t ui = asc ? ii ^ 0x80000000U : ii ^ 0x7FFFFFFFU;
    return _enc32(ui);
}

inline int decode_int(uint32_t ui, bool asc = true) {
    uint32_t nui = _dec32(ui);
    return asc ? nui ^ 0x80000000U : nui ^ 0x7FFFFFFFU;
}

inline int decode_int(const void * p, bool asc = true) {
    uint32_t ui = *(reinterpret_cast<const uint32_t*>(p));
    uint32_t nui = _dec32(ui);
    return asc ? nui ^ 0x80000000U : nui ^ 0x7FFFFFFFU;
}

// Date uses this for encode
inline uint64_t encode(long ll, bool asc = true) {
    uint64_t ul = asc ? ll ^ 0x8000000000000000ULL : ll ^ 0x7FFFFFFFFFFFFFFFULL;
    return _enc64(ul);
}

inline long decode_long(uint64_t ul, bool asc = true) {
    uint64_t nul = _dec64(ul);
    return asc ? nul ^ 0x8000000000000000ULL : nul ^ 0x7FFFFFFFFFFFFFFFULL;
}

inline long decode_long(const void* p, bool asc = true) {
    uint64_t ul = *(reinterpret_cast<const uint64_t*>(p));
    uint64_t nul = _dec64(ul);
    return asc ? nul ^ 0x8000000000000000ULL : nul ^ 0x7FFFFFFFFFFFFFFFULL;
}

#if defined(_SOPE_TYPES_DEFINED)
inline Date decode_date(uint64_t ul, bool asc = true) {
    uint64_t nul = _dec64(ul);
    return asc ? nul ^ 0x8000000000000000ULL : nul ^ 0x7FFFFFFFFFFFFFFFULL;
}

inline Date decode_date(const void* p, bool asc = true) {
    uint64_t ul = *(reinterpret_cast<const uint64_t*>(p));
    uint64_t nul = _dec64(ul);
    return asc ? nul ^ 0x8000000000000000ULL : nul ^ 0x7FFFFFFFFFFFFFFFULL;
}

// Timestamp is uint64_t
inline uint64_t encode(Timestamp ts, bool asc = true) {
    uint64_t nul = _enc64(ts);
    return asc ? nul : nul ^ 0xFFFFFFFFFFFFFFFFULL;
}

inline Timestamp decode_timestamp(const void* p, bool asc = true) {
    uint64_t ul = *(reinterpret_cast<const uint64_t*>(p));
    uint64_t nul = _dec64(ul);
    return asc ? nul : nul ^ 0xFFFFFFFFFFFFFFFFULL;
}
#endif

// IEEE double binary is represented as:
// (sign) exponent(1203-biased 11 bits) coefficient (implicit 1.xx...x, 52 bits)
// for negative numbers, we need to flip all bits as the larger the value, the smaller
// flipping all bits results in (0xFF...FF - the number treated as an integer).
// In descending order, for positive numbers, we need to flip all the bits except the
// sign bit, the larger the value, the smaller in descending order.
// flipping all bits except the sign bit results in
// (0x7F...FF - the number treated as an integer).
inline uint64_t encode(double dd, bool asc = true) {
    uint64_t ud = 0;
    memcpy(&ud, &dd, sizeof(dd));

    if (asc)
        ud = (ud & 0x8000000000000000ULL)
             ? (ud ^ 0xFFFFFFFFFFFFFFFFULL)
             : (ud ^ 0x8000000000000000ULL);
    else
        ud = (ud & 0x8000000000000000ULL) ? ud : (ud ^ 0x7FFFFFFFFFFFFFFFULL);
    return _enc64(ud);
}

inline double decode_double(uint64_t ul, bool asc = true) {
    uint64_t ud = _dec64(ul);
    if (asc)
        ud = (ud & 0x8000000000000000ULL)
             ? (ud ^ 0x8000000000000000ULL)
             : (ud ^ 0xFFFFFFFFFFFFFFFFULL);
    else
        ud = (ud & 0x8000000000000000ULL) ? ud : (ud ^ 0x7FFFFFFFFFFFFFFFULL);

    double ret = 0;
    memcpy(&ret, &ud, sizeof(ud));
    return ret;
}

inline double decode_double(const void * p, bool asc = true) {
    uint64_t ul = *(reinterpret_cast<const uint64_t*>(p));
    uint64_t ud = _dec64(ul);
    if (asc)
        ud = (ud & 0x8000000000000000ULL)
             ? (ud ^ 0x8000000000000000ULL)
             : (ud ^ 0xFFFFFFFFFFFFFFFFULL);
    else
        ud = (ud & 0x8000000000000000ULL) ? ud : (ud ^ 0x7FFFFFFFFFFFFFFFULL);

    double ret = 0;
    memcpy(&ret, &ud, sizeof(ud));
    return ret;
}

// Works for UTF-8 and UTF-16 encodings
// encode a string, a string does not have two consecutive 0 in the middle, end with x0000
// return the total length, flip bits for descending order
inline uint32_t encode(const char* ps, uint32_t len, void* pBuf, bool asc = true) {
    assert(STRING_PAD_LEN == 2);
    if (asc) {
        memcpy(pBuf, ps, (size_t)len);
        *(reinterpret_cast<char*>(pBuf)+len) = '\0';
        *(reinterpret_cast<char*>(pBuf)+len+1) = '\0';
    } else {
        for (uint32_t i = 0; i < len; i++) {
            *(reinterpret_cast<char*>(pBuf) + i) = *(ps + i) ^ 0xFF;
        }
        *(reinterpret_cast<char*>(pBuf)+len) = '\xFF';
        *(reinterpret_cast<char*>(pBuf)+len+1) = '\xFF';
    }
    return len + STRING_PAD_LEN;
}

// Get actual size of string
inline uint32_t get_string_len(const void* p, bool asc = true) {
    const char* ps = reinterpret_cast<const char*>(p);
    if (asc) {
        while (true) {
            if (*ps != 0) {
                ps++;
            } else if (*(ps+1) != 0) {
                ps++;
            } else {
                return (uint32_t) (ps - (reinterpret_cast<const char*>(p)));
            }
        }
    } else {
        while (true) {
            if (*ps != '\xFF') {
                ps++;
            } else if (*(ps+1) != '\xFF') {
                ps++;
            } else {
                return (uint32_t) (ps - (reinterpret_cast<const char*>(p)));
            }
        }
    }
}

// return the string len without trailing 00 or FF
inline uint32_t decode_string(const void * p, void* pBuf, bool asc = true) {
    const char* pfrom = reinterpret_cast<const char*>(p);
    char* pto = reinterpret_cast<char*>(pBuf);
    if (asc) {
        while (true) {
            if (*pfrom != 0) {
                *pto++ = *pfrom++;
            } else if (*(pfrom+1) != 0) {
                *pto++ = *pfrom++;
            } else {
                break;
            }
        }
    } else {
        while (true) {
            if (*pfrom != '\xFF') {
                *pto++ = *pfrom++ ^ 0xFF;
            } else if (*(pfrom+1) != '\xFF') {
                *pto++ = *pfrom++ ^ 0xFF;
            } else {
                break;
            }
        }
    }
    return pto - reinterpret_cast<char*>(pBuf);
}

// calculate encoding length for a binary string, which can contain 00 in the middle
// escaped with 0x00FF, end with 0x0000
inline uint32_t calc_binary_encoded_len(const void * pb, uint32_t len) {
    int zero_count = 0;
    for (uint32_t from = 0; from < len; from++) {
       if (*(reinterpret_cast<const uint8_t*>(pb) + from) == 0) {
           zero_count++;
       }
    }
    return len + BINARY_PAD_LEN + zero_count;
}

// calculate encoding length for a string, end with 0x00 or 0xFF
inline uint32_t calc_string_encoded_len(uint32_t len) {
    return len + STRING_PAD_LEN;
}

// encode a binary value, escape 0x00 with 0x00FF, end with 0x0000.
// The reason of escaping 0x00 value with 0x00FF is that we want it
// to be larger than the byte array ending at the 0x00 value position.
// return the total length, flip bits for descending order
// BINARY_PAD_LEN is 2, so append two extra byte
// In case we change BINARY_PAD_LEN in the future,
// make sure change here
inline uint32_t encode(const void * pb, uint32_t len, void* pBuf, bool asc = true) {
    assert(BINARY_PAD_LEN == 2);
    uint32_t to = 0;
    if (asc) {
        for (uint32_t from = 0; from < len; from++) {
           *(reinterpret_cast<uint8_t*>(pBuf) + to) =
                   *(reinterpret_cast<const uint8_t*>(pb) + from);
           if (*(reinterpret_cast<const uint8_t*>(pb) + from) == 0) {
               to++;
               *(reinterpret_cast<uint8_t*>(pBuf) + to) = 0xFF;
           }
           to++;
        }
        *(reinterpret_cast<uint8_t*>(pBuf)+to) = 0;
        *(reinterpret_cast<uint8_t*>(pBuf)+to+1) = 0;
    } else {
        for (uint32_t from = 0; from < len; from++) {
           *(reinterpret_cast<uint8_t*>(pBuf) + to) =
                   *(reinterpret_cast<const uint8_t*>(pb) + from) ^ 0xFF;
           if (*(reinterpret_cast<const uint8_t*>(pb) + from) == 0) {
               to++;
               *(reinterpret_cast<uint8_t*>(pBuf) + to) = 0;
           }
           to++;
        }
        *(reinterpret_cast<uint8_t*>(pBuf)+to) = 0xFF;
        *(reinterpret_cast<uint8_t*>(pBuf)+to+1) = 0xFF;
    }

    return to + BINARY_PAD_LEN;
}

// length after decode
inline uint32_t get_bytes_len(const void* p, bool asc = true) {
    uint32_t len = 0;
    const uint8_t* pb = reinterpret_cast<const uint8_t*>(p);
    if (asc) {
        while(true) {
            if (*pb != 0) {
                len++;
                pb++;
            } else if (*(pb+1) == 0) {
                break;
            } else if (*(pb+1) == 0xFF) {
                len++;
                pb += 2;
            } else {
                // If not, hanging happens.
                // We should abort the process to debug it.
                abort();
            }
        }
    } else {
        while(true) {
            if (*pb != 0xFF) {
                len++;
                pb++;
            } else if (*(pb+1) == 0xFF) {
                break;
            } else if (*(pb+1) == 0) {
                len++;
                pb += 2;
            } else {
                // If not, hanging happens.
                // We should abort the process to debug it.
                abort();
            }
        }
    }
    return len;
}

// return the bytes consumed during decoding, length after decode is in len
inline uint32_t decode_bytes(const void * p,
                             void* pBuf,
                             uint32_t& len,
                             bool asc = true) {
    const uint8_t* pfrom = reinterpret_cast<const uint8_t*>(p);
    uint8_t* pto = reinterpret_cast<uint8_t*>(pBuf);
    if (asc) {
        while (true) {
            if (*pfrom != 0) {
                *pto++ = *pfrom++;
            } else if (*(pfrom+1) == 0) {
                break;
            } else if (*(pfrom+1) == 0xFF) {
                *pto++ = 0;
                pfrom += 2;
            } else {
                // If not, hanging happens.
                // We should abort the process to debug it.
                abort();
            }
        }
    } else {
        while (true) {
            if (*pfrom != 0xFF) {
                *pto++ = *pfrom++ ^ 0xFF;
            } else if (*(pfrom+1) == 0xFF) {
                break;
            } else if (*(pfrom+1) == 0) {
                *pto++ = 0;
                pfrom += 2;
            } else {
                // If not, hanging happens.
                // We should abort the process to debug it.
                abort();
            }
        }
    }
    len = pto - reinterpret_cast<uint8_t*>(pBuf);
    return pfrom - reinterpret_cast<const uint8_t*>(p);
}

}

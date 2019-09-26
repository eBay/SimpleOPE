/******************************************************************
Copyright 2019 eBay Inc.
Architect/Developer(s): Jung-Sang Ahn, Gene Zhang, Kun Ren

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

#include "cast_helper.h"
#include "sope_types.h"
#include "sope_encode.h"

#include <string.h>

namespace sope {

/**
 * Every field is nullable except for primary key and meta fields
 *
 *   === NULL indicator for fields(when insert fields in record) ===
 * - For a non-null value, a not-null indicator byte is prepended to
 *   the front of the field value.
 * - For a null value, only a null indicator byte is stored
 *   without field value.
 * - The indicator byte depends on the asc/desc ordering.
 *   Below is the encoding method we are using for the indicator
 *   (We encode NULL as the smallest value):
 *
 *                       |       Asc     |       Desc
 * ----------------------------------------------------
 * Not NUll indicator:   |      0x0F     |       0xF0
 * ----------------------------------------------------
 * NULL     indicator:   |      0x07     |       0xF8
 * ----------------------------------------------------
 *
 * Note : if we change the NULL as the biggest value in future,
 * we can use below encoding method(Not use now):
 *
 *                       |       Asc     |       Desc
 * ----------------------------------------------------
 * Not NUll indicator:   |      0x0F     |       0xF0
 * ----------------------------------------------------
 * NULL     indicator:   |      0xF0     |       0x0F
 * ----------------------------------------------------
 *
 *   === NULL indicator for search conditions for scan ===
 * (1) If the search condition is NULL(And it is not the NULL point query),
 *     then the encoding method is below:
 *     Note that the encoding method for Asc or Desc is
 *     the same for NULL condition.
 *
 *                       |      startKey      |     endKey
 * ---------------------------------------------------------
 *  NUll condition :     |          0x00      |       0xFF
 * ---------------------------------------------------------
 *
 * (2) If the search condition is not NULL, then the encoding method is below:
 *     Note that the encoding method for startKey and endKey is the same
 *     for non-NULL condition.
 *
 *                           |       Asc      |    Desc
 * ---------------------------------------------------------
 *  non-Null condition :     |      0x0F      |    0xF0
 * ---------------------------------------------------------
 *
 *  (3) If the search condition is NULL point query, then encoding method
 *      is below:
 *
 *                             |      Asc      |     Desc
 * ---------------------------------------------------------
 *  NUll Point condition :     |      0x07     |     0xF8
 * ---------------------------------------------------------
 */

class EncodedRecord {

#define NOT_NULL_ASC   0x0F
#define NOT_NULL_DESC  0xF0
#define NULL_ASC   0x07
#define NULL_DESC  0xF8
#define NULL_COND_START  0x00
#define NULL_COND_END    0xFF
#define NOT_NULL_COND_ASC  0x0F
#define NOT_NULL_COND_DESC 0xF0
#define NULL_POINT_COND_ASC 0x07
#define NULL_POINT_COND_DESC 0xF8

public:
    // empty record for construction use
    EncodedRecord()
        : pData(nullptr)
        , curPos(0)
        , endPos(0)
        , curLen(0)
        , pWorkingBuf(nullptr)
        , lenWorkingBuf(0) { }

    EncodedRecord(size_t sz)
        : curPos(0)
        , endPos(0)
        , curLen(sz)
        , pWorkingBuf(nullptr)
        , lenWorkingBuf(0) {
        pData = (uint8_t *) malloc(sz);
    }

    // with record data, for field extraction use
    EncodedRecord(void* pdata, uint32_t len)
        : pData(_SCU(pdata))
        , curPos(0)
        , endPos(len)
        , curLen(len)
        , pWorkingBuf(nullptr)
        , lenWorkingBuf(0) {}

    ~EncodedRecord() {
        // Should not free pData in destructor,
        // as user might allocate it outside Record,
        // and assigned its pointer only via constructor.
        // User is responsible for calling freeInternals() function.
        if (pWorkingBuf) {
            ::free(pWorkingBuf);
            pWorkingBuf = nullptr;
            lenWorkingBuf = 0;
        }
    }

    void alloc(uint32_t sz) {
        if (pData != nullptr) ::free(pData);
        pData = (uint8_t *) malloc((size_t) sz);
        curPos = 0;
        curLen = sz;
    }

    void resize(size_t new_sz) {
        // Only for increasing.
        if (new_sz <= curLen) return;

        void* new_ptr = realloc(pData, new_sz);
        if (new_ptr) {
            pData = _SCU(new_ptr);
            curLen = new_sz;
        } // Otherwise, realloc failed.
    }

    void freeInternals() {
        if (pData) {
            ::free(pData);
            pData = nullptr;
        }
    }

    void resetPos() { curPos = 0; }
    void setPos(size_t new_pos) {
        assert(new_pos < curLen);
        curPos = new_pos;
    }

    void putNullFieldIndicator(bool asc = true) {
        *_RC(uint8_t*, pData + curPos) = asc ? NULL_ASC : NULL_DESC;
        curPos += LEN_NULL;
    }
    void putNotNullFieldIndicator(bool asc = true) {
        *_RC(uint8_t*, pData + curPos) = asc ? NOT_NULL_ASC : NOT_NULL_DESC;
        curPos += LEN_NULL;
    }
    void putNullConditionIndicator(bool start = true) {
        *_RC(uint8_t*, pData + curPos) = start ? NULL_COND_START : NULL_COND_END;
        curPos += LEN_NULL;
    }
    void putNotNullConditionIndicator(bool asc = true) {
        *_RC(uint8_t*, pData + curPos) = asc
                                         ? NOT_NULL_COND_ASC
                                         : NOT_NULL_COND_DESC;
        curPos += LEN_NULL;
    }
    void putNullPointConditionIndicator(bool asc = true) {
        *_RC(uint8_t*, pData + curPos) = asc
                                         ? NULL_POINT_COND_ASC
                                         : NULL_POINT_COND_DESC;
        curPos += LEN_NULL;
    }
    void put(int i, bool asc = true) {
        *_RC(uint32_t*, pData+curPos) = encode(i, asc);
        curPos += LEN_INT;
    }
    void put(long l, bool asc = true) {
        *_RC(uint64_t*, pData+curPos) = encode(l, asc);
        curPos += LEN_LONG;
    }

#ifdef __APPLE__
    // Mac reports ambiguity between long and Date.
    // Need to explicitly separate them.
    void put(Date l, bool asc = true) {
        *_RC(uint64_t*, pData+curPos) = encode((long)l, asc);
        curPos += LEN_LONG;
    }
#endif

    void put(double d, bool asc = true) {
        *_RC(uint64_t*, pData+curPos) = encode(d, asc);
        curPos += LEN_DOUBLE;
    }
    void put(bool b, bool asc = true) {
        *_RC(bool*, pData+curPos) = asc ? b : !b;
        curPos += LEN_BOOL;
    }
    void put(Timestamp ts, bool asc = true) {
        *_RC(Timestamp*, pData+curPos) = encode(ts, asc);
        curPos += LEN_TIMESTAMP;
    }
    void put(const char * p, uint32_t len, bool asc = true) {
        uint32_t enclen = encode(p, len, pData+curPos, asc);
        curPos += enclen;
    }
    void put(const void * p, uint32_t len, bool asc = true) {
        uint32_t enclen = encode(p, len, pData+curPos, asc);
        curPos += enclen;
    }

    bool checkNullFieldIndicator(bool asc = true) {
        bool is_null = (*_RC(uint8_t*, pData+curPos)
                        ==(asc ? NULL_ASC : NULL_DESC));
        curPos += LEN_NULL;
        return is_null;
    }

    int getInt(bool asc = true) {
        int i = decode_int(pData+curPos, asc);
        curPos += LEN_INT;
        return i;
    }

    long getLong(bool asc = true) {
        long l = decode_long(pData+curPos, asc);
        curPos += LEN_LONG;
        return l;
    }

    double getDouble(bool asc = true) {
        double d = decode_double(pData+curPos, asc);
        curPos += LEN_DOUBLE;
        return d;
    }

    bool getBool(bool asc = true) {
        bool b = *_RC(bool*, pData+curPos);
        curPos += LEN_BOOL;
        return b;
    }

    Date getDate(bool asc = true) {
        Date d = decode_date(pData+curPos, asc);
        curPos += LEN_DATE;
        return d;
    }

    Timestamp getTimestamp(bool asc = true) {
        Timestamp ts = decode_timestamp(pData+curPos, asc);
        curPos += LEN_TIMESTAMP;
        return ts;
    }

    const char* getString(uint32_t& len, bool asc = true) {
        len = get_string_len(pData+curPos, asc);
        getWorkingBuf(len);
        decode_string(pData+curPos, pWorkingBuf, asc);
        curPos += len + STRING_PAD_LEN;
        return _RC(char*, pWorkingBuf);
    }

    uint8_t* getBinary(uint32_t& len, bool asc = true) {
        len = get_bytes_len(pData+curPos, asc);
        getWorkingBuf(len);
        uint32_t len_before = decode_bytes(pData+curPos, pWorkingBuf, len, asc);
        curPos += len_before + BINARY_PAD_LEN; //bytes has 2 bytes trailer
        return pWorkingBuf;
    }

    void setEndPos() { endPos = curPos; }
    int  getEndPos() const { return endPos; }

    uint32_t getLen() const { return curLen; }
    void*    getData()      { return _SCV(pData); }
    const void*   getData() const   { return _SCV(pData); }
    int      getPos() const { return curPos; }
    void clear() {
        pData = nullptr;
        curPos = 0;
        endPos = 0;
        curLen = 0;
    }

private:
    void getWorkingBuf(uint32_t len) {
        if (pWorkingBuf != nullptr && lenWorkingBuf >= len) return;
        if (pWorkingBuf != nullptr && lenWorkingBuf < len) ::free(pWorkingBuf);
        pWorkingBuf = (uint8_t *) malloc((size_t) len);
        lenWorkingBuf = len;
    }

    uint8_t*  pData;
    int       curPos;
    int       endPos;
    uint32_t  curLen;
    uint8_t*  pWorkingBuf;
    uint32_t  lenWorkingBuf;
};

/* inline bool comp(const EncodedRecord& r1, const EncodedRecord& r2) {
    return ( 0 > memcmp(r1.getData(), r2.getData(), (r1.getEndPos() <= r2.getEndPos()) ? r1.getEndPos() : r2.getEndPos()) );
}
*/

inline bool comp(const EncodedRecord* r1, const EncodedRecord* r2) {
    return ( 0 > memcmp(r1->getData(), r2->getData(), (r1->getEndPos() <= r2->getEndPos()) ? r1->getEndPos() : r2->getEndPos()) );
}

}

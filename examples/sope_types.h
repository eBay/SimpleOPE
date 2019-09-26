/******************************************************************
Copyright 2019 eBay Inc.
Architect/Developer: Gene Zhang

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

#include <string>
#include <cstdint>

#define _SOPE_TYPES_DEFINED

namespace sope {

// Date contains milliseconds since epoch,
// can be negative
typedef int64_t Date;

// For system timestamp use, nonoseconds
typedef uint64_t Timestamp;

enum Type : int {
    TYPE_NULL       = 0,
    TYPE_INT        = 1, // for int32.
    TYPE_LONG       = 2, // for int64.
    TYPE_DOUBLE     = 3,
    TYPE_STRING     = 4,
    TYPE_BOOL       = 5,
    TYPE_DATE       = 6, // internally int64.
    TYPE_TIMESTAMP  = 7, // internally uint64.
    TYPE_BINARY     = 8, // any binary bytes.
    TYPE_OBJECT     = 9, // internally binary.
};

const uint32_t LEN_NULL = 1;
const uint32_t LEN_INT = 4;
const uint32_t LEN_LONG = 8;
const uint32_t LEN_DOUBLE = 8;
const uint32_t LEN_BOOL = 1;
const uint32_t LEN_DATE = 8;
const uint32_t LEN_TIMESTAMP = 8;

inline uint32_t Typelen(Type t) {
    static uint32_t TypeLen_[] = {1, 4, 8, 8, 0, 1, 8, 8, 0, 0};
    return TypeLen_[static_cast<int>(t)];
}
inline Type convert2Type(const std::string& s_type) {
    if (s_type == "NULL")        return TYPE_NULL;
    if (s_type == "INT")         return TYPE_INT;
    if (s_type == "LONG")        return TYPE_LONG;
    if (s_type == "DOUBLE")      return TYPE_DOUBLE;
    if (s_type == "STRING")      return TYPE_STRING;
    if (s_type == "BOOL")        return TYPE_BOOL;
    if (s_type == "DATE")        return TYPE_DATE;
    if (s_type == "TIMESTAMP")   return TYPE_TIMESTAMP;
    if (s_type == "BINARY")      return TYPE_BINARY;
    if (s_type == "OBJECT")      return TYPE_OBJECT;
    return TYPE_NULL;   // unknown type
}

std::string toString(Date d);

std::string toString(Timestamp ts);

std::string toHexString(const void* pd, uint32_t len);

}

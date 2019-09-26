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

#include "sope_encode.h"

#include <iostream>
#include <stdio.h>
#include <string.h>

using namespace sope;

// examples to show order-preserving encoding and their comparison results
// using tuples of three types: int, string, and double
// tuple1: (10, "This is a string", 1234.5678)
// tuple2: (-10, "This is a string", 12345.6789)
// tuple3: (100, "This is a string", 1234.5678)
// tuple4: (10, "This is a string1", 1234.5678)
// tuple5: (10, "This is a strin", 1234.5678)
// tuple6: (10, "This is a string", -1234.5678)
// tuple7: (10, "This is a string", 1234.5678)

struct EncodedTuple {
    size_t  len;
    uint8_t tuple[64];     // for simplicity

    EncodedTuple(int ii, const char* str, double dd) {
        *reinterpret_cast<uint32_t*>(tuple) = encode(ii);
        len = sizeof(uint32_t);
        size_t slen = strlen(str);
        uint32_t eslen = encode(str, slen, (void*)(tuple+len));
        len += eslen;
        *reinterpret_cast<uint64_t*>(tuple+len) = encode(dd);
        len += sizeof(uint64_t);
    }

    ~EncodedTuple() { }
};

#define MIN(x, y) ((x <= y) ? x : y)

// return -1, 0, 1 for t1 < t2, t1==t2, and t1 > t2, respectively
int compTuple(const EncodedTuple & t1, const EncodedTuple & t2) {
    int rc = memcmp(t1.tuple, t2.tuple, MIN(t1.len,t2.len));
    if (rc == 0) {
        rc = (t1.len < t2.len) ? -1 : ( (t1.len > t2.len) ? 1 : 0 );
    } else {
       rc = (rc < 0) ? -1 : 1;
    }
    return rc;
}

int main(int argc, char** argv)
{
    EncodedTuple tuple1(10, "This is a string", 1234.5678),
                tuple2(-10, "This is a string", 12345.6789),
                tuple3(100, "This is a string", 1234.5678),
                tuple4(10, "This is a string1", 1234.5678),
                tuple5(10, "This is a strin", 1234.5678),
                tuple6(10, "This is a string", -1234.5678),
                tuple7(10, "This is a string", 1234.5678);

    printf("Expected results:\n");
    printf("1 -1 -1 1 1 0\n");

    int rc = compTuple(tuple1, tuple2);
    printf("%d ", rc);

    rc = compTuple(tuple1, tuple3);
    printf("%d ", rc);

    rc = compTuple(tuple1, tuple4);
    printf("%d ", rc);

    rc = compTuple(tuple1, tuple5);
    printf("%d ", rc);

    rc = compTuple(tuple1, tuple6);
    printf("%d ", rc);

    rc = compTuple(tuple1, tuple7);
    printf("%d\n", rc);

    return 0;
}


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
#include "sope_types.h"

namespace sope {

std::string toString(Date d) {
    char buf[40];
    int64_t d_secs = d/1000;
    struct tm * ptm = gmtime((const time_t*)&d_secs);
    size_t sz = strftime(buf, sizeof(buf), "%FT%TZ", ptm);
    return std::string(buf, sz);
}

std::string toString(Timestamp ts) {
    time_t tsec = ts / 1000000000;
    uint32_t tnsec = ts % 1000000000;
    char  ctbuf[50];
    struct tm * ptm = gmtime(& tsec);
    size_t sz = strftime(ctbuf, sizeof(ctbuf), "%FT%T.", ptm);
    sprintf(ctbuf+sz, "%uZ", tnsec);
    return std::string(ctbuf);
}

std::string toHexString(const void* pd, uint32_t len) {
    char cbuf[2*len + 3];

    cbuf[0] = '0';
    cbuf[1] = 'X';
    for (int i = 0; i < len; i++) {
        sprintf(cbuf+2*(i+1), "%02X", *(reinterpret_cast<const uint8_t*>(pd) +i) );
    }
    cbuf[2*len+2] = '\0';
    return std::string(cbuf);
}

}

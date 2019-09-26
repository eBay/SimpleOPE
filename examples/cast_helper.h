/******************************************************************
Copyright 2019 eBay Inc.
Author/Developer: Jung-Sang Ahn

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

#define _RC(type, val)  reinterpret_cast<type>(val)
#define _SC(type, val)  static_cast<type>(val)

#define _SCU(val)       static_cast<uint8_t*>(val)
#define _SCV(val)       static_cast<void*>(val)
#define _SCC(val)       static_cast<char*>(val)
#define _SCCC(val)      static_cast<const char*>(val)
#define _SCCV(val)      static_cast<const void*>(val)
#define _SCI(val)       static_cast<int>(val)
#define _SCS(val)       static_cast<size_t>(val)
#define _SCD(val)       static_cast<double>(val)


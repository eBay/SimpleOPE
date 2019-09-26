
SOPE: Simple Order-Preserving Encoding
======================================
This code creates a Simple Order-Preserving Encoding. Written in C++, the code includes basic examples to illustrate how it can be used.

The code encodes a tuple or record of fields with various types into an array of bytes so that byte comparison on the encoded records returns the same results as comparing fields based on types. This can be used in many scenarios to avoid custom functions for comparison. It speeds up performance significantly. It works for both big endian and little endian machines.

Encoding/decoding code
----------------------
[`src/sope_encode.h`](src/sope_encode.h): Provides encoding and decoding functions for types: int, long, double, string, binary. Date and Timestamp are supported as long and unsigned long as examples illustrated in the encoded record example.

Examples
--------
 1. [`examples/sope_simple_test.cc`](examples/sope_simple_test.cc): illustrates a simple encoding use example.

 2. encoded record example: illustrates a little more sophisticated record encoding example
   - [`examples/sope_types.h`](examples/sope_types.h): defines types used in defining schema for records.
   - [`examples/sope_encoded_record.h`](examples/sope_encoded_record.h): supports encoding and decoding for records of fields.
   - [`examples/sope_record_test.cc`](examples/sope_record_test.cc): illustrates a record encoding example, including ascending or descending order, and support  for null values. Records or rows in a table are strongly typed by a schema. Every field is nullable. The main function is just to display the rows before and after sorting. (note that pretty formatting is not the goal.) Search can be done by providing start condition record (low key) and end condition record (high key), which is not included in the example. The record construction provides facility for it and since we use [low, high) convention in constructing the condition records, null encoding is different for record fields and conditions.

Notes
-----
* The example code is minimalistic, and for illustration purposes only.
* Further optimization can be done to reduce the encoded result size.

License
-------
Copyright 2018 eBay Inc.
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

3rd Party Code License
----------------------
This code base includes some third-party code, as detailed below:

URL: https://github.com/couchbase/forestdb <BR>
License: https://github.com/couchbase/forestdb/blob/master/LICENSE <BR>
Originally licensed under the Apache 2.0 license.

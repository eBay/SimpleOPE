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
#include "sope_encoded_record.h"

#include <iostream>
#include <stdio.h>
#include <vector>
#include <algorithm>

using namespace sope;

namespace sope_test {

/***************************************
Definition of FieldDef class
*****************************************/
struct FieldDef {
    Type     type;
    uint32_t len;
    bool     asc;

    FieldDef() : type(TYPE_NULL), len(0), asc(true) {}
    FieldDef(Type t, uint32_t l, bool asc_) : type(t), len(l), asc(asc_) {}
};

    
/***************************************
Definition of RecordDef class
*****************************************/
class RecordDef {
public:
    RecordDef(int n_fields)
        : fields(n_fields) {}

    void setFieldDef(int i, Type t, bool asc_) {
        fields[i] = FieldDef(t, Typelen(t), asc_);
    }

    Type getType(int i) const {
        return fields[i].type;
    }

    uint32_t getLen(int i) const {
        return fields[i].len;
    }

    bool isAsc(int i) const {
        return fields[i].asc;
    }

    int getNumFields() const {
        return fields.size();
    }

private:
    std::vector<FieldDef> fields;
};

/***************************************
Definition of Table class
*****************************************/
class Table {
public:
    Table(RecordDef* ps) : pSchema(ps) { }
    ~Table() { delete pSchema; clear(); }

    void clear() {
        for (int i = 0; i < table.size(); i++) {
            delete table[i];
        }
    }

    void addRecord(EncodedRecord* pr) {
        table.push_back(pr);
    }

    EncodedRecord* getRecord(int i) {
        return (i < table.size()) ? table[i] : nullptr;
    }

    int getNumRecords() {
        return table.size();
    }

    const RecordDef* getSchema() const {
       return pSchema;
    }

    void sort() {
        std::sort(table.begin(), table.end(), comp);
    }

private:
    RecordDef* pSchema;
    std::vector<EncodedRecord*> table;
};

void displayTable(Table* pt);

void display(EncodedRecord * pr, const RecordDef *ps);

RecordDef*  create_test_schema() {
    RecordDef* ps = new RecordDef(5);

    ps->setFieldDef(0, TYPE_INT, true);
    ps->setFieldDef(1, TYPE_LONG, true);
    ps->setFieldDef(2, TYPE_STRING, false);  // descending
    ps->setFieldDef(3, TYPE_BINARY, true);
    ps->setFieldDef(4, TYPE_DOUBLE, false);  // descending

    return ps;
}

// We will construct a table with the following records
//====================================================================================
//int       long          string(desc)            binary               double (desc)
//------------------------------------------------------------------------------------
//  10     1000000l  "This is a string"  "\x11\x22\x60\x70\x80\x90"   12345.6789
// 100     2000000l  "This is a s"       "\x11\x12\x00\x20"             234.567
//  10     2000000l  "This is"           "\x11\x10\x20"               12345.789
// 100     2000000l  "This is a s"       NULL                           234.567
// 100     2000000l  "This is a s"       "\x11\x12\x00\x20"             234.567
//  10     NULL      NULL                "\x11\x22\x60\x70\x80\x90"   12345.6789
//  10     1000000l  "This is a string"  "\x11\x22\x60\x70\x80\x90"  -12345.6789
// -20     -200000l  "This is a string"  "\x11\x22\x60\x70\x80\x90"     NULL
//  10     1000000l  "This is a string"  "\x11\x22\x60\x70\x80\x90"    2345.6789
// -20     -200000l  "This is a string"  "\x11\x22\x60\x70\x80\x90"     NULL
//  10     1000000l  NULL                "\x11\x22\x60\x70\x80\x90"   12345.6789
// -20     -200000l  "This is a string"  "\x11\x22\x60\x70\x80\x90"     123.456
//----------------------------------------------------------------------------------
// Then we will sort it and do some searh over the sorted table

Table*  buildRecords() {
    RecordDef* ps = create_test_schema();
    Table * pTable = new Table(ps);

    // record 1
    EncodedRecord * pr = new EncodedRecord();
    pr->alloc(100);    // large enough for simplicity
    pr->putNotNullFieldIndicator();
    pr->put(10);
    pr->putNotNullFieldIndicator();
    pr->put(1000000l);
    pr->putNotNullFieldIndicator(false);  // false: descending order
    pr->put("This is a string", 16, false);
    pr->putNotNullFieldIndicator();
    pr->put((void*) "\x11\x22\x60\x70\x80\x90", 6);
    pr->putNotNullFieldIndicator(false);
    pr->put((double) 12345.6789, false);
    pr->setEndPos();
    pr->resetPos();
    pTable->addRecord(pr);

    // record 2
    pr = new EncodedRecord();
    pr->alloc(100);
    pr->putNotNullFieldIndicator();
    pr->put(100);
    pr->putNotNullFieldIndicator();
    pr->put(2000000l);
    pr->putNotNullFieldIndicator(false);
    pr->put("This is a s", 11, false);
    pr->putNotNullFieldIndicator();
    pr->put((void*) "\x11\x12\x00\x20", 4);
    pr->putNotNullFieldIndicator(false);
    pr->put((double) 234.567, false);
    pr->setEndPos();
    pr->resetPos();
    pTable->addRecord(pr);

    // record 3
    pr = new EncodedRecord();
    pr->alloc(100);
    pr->putNotNullFieldIndicator();
    pr->put(10);
    pr->putNotNullFieldIndicator();
    pr->put(2000000l);
    pr->putNotNullFieldIndicator(false);
    pr->put("This is", 7, false);
    pr->putNotNullFieldIndicator();
    pr->put((void*) "\x11\x10\x20", 3);
    pr->putNotNullFieldIndicator(false);
    pr->put((double) 12345.789, false);
    pr->setEndPos();
    pr->resetPos();
    pTable->addRecord(pr);

    // record 4
    pr = new EncodedRecord();
    pr->alloc(100);
    pr->putNotNullFieldIndicator();
    pr->put(100);
    pr->putNotNullFieldIndicator();
    pr->put(2000000l);
    pr->putNotNullFieldIndicator(false);
    pr->put("This is a s", 11, false);
    pr->putNullFieldIndicator();
    pr->putNotNullFieldIndicator(false);
    pr->put((double) 234.567, false);
    pr->setEndPos();
    pr->resetPos();
    pTable->addRecord(pr);

    // record 5
    pr = new EncodedRecord();
    pr->alloc(100);
    pr->putNotNullFieldIndicator();
    pr->put(100);
    pr->putNotNullFieldIndicator();
    pr->put(2000000l);
    pr->putNotNullFieldIndicator(false);
    pr->put("This is a s", 11, false);
    pr->putNotNullFieldIndicator();
    pr->put((void*) "\x11\x12\x00\x20", 4);
    pr->putNotNullFieldIndicator(false);
    pr->put((double) 234.567, false);
    pr->setEndPos();
    pr->resetPos();
    pTable->addRecord(pr);

    // record 6
    pr = new EncodedRecord();
    pr->alloc(100);
    pr->putNullFieldIndicator();
    pr->putNullFieldIndicator();
    pr->putNotNullFieldIndicator(false);
    pr->put("This is a string", 16, false);
    pr->putNotNullFieldIndicator();
    pr->put((void*) "\x11\x22\x60\x70\x80\x90", 6);
    pr->putNotNullFieldIndicator(false);
    pr->put((double) 12345.6789, false);
    pr->setEndPos();
    pr->resetPos();
    pTable->addRecord(pr);

    // record 7
    pr = new EncodedRecord();
    pr->alloc(100);
    pr->putNotNullFieldIndicator();
    pr->put(10);
    pr->putNotNullFieldIndicator();
    pr->put(1000000l);
    pr->putNotNullFieldIndicator(false);
    pr->put("This is a string", 16, false);
    pr->putNotNullFieldIndicator();
    pr->put((void*) "\x11\x22\x60\x70\x80\x90", 6);
    pr->putNotNullFieldIndicator(false);
    pr->put((double) -12345.6789, false);
    pr->setEndPos();
    pr->resetPos();
    pTable->addRecord(pr);

    // record 8
    pr = new EncodedRecord();
    pr->alloc(100);
    pr->putNotNullFieldIndicator();
    pr->put(-20);
    pr->putNotNullFieldIndicator();
    pr->put(-2000000l);
    pr->putNotNullFieldIndicator(false);
    pr->put("This is a string", 16, false);
    pr->putNotNullFieldIndicator();
    pr->put((void*) "\x11\x22\x60\x70\x80\x90", 6);
    pr->putNullFieldIndicator(false);
    pr->setEndPos();
    pr->resetPos();
    pTable->addRecord(pr);

    // record 9
    pr = new EncodedRecord();
    pr->alloc(100);
    pr->putNotNullFieldIndicator();
    pr->put(10);
    pr->putNotNullFieldIndicator();
    pr->put(1000000l);
    pr->putNotNullFieldIndicator(false);
    pr->put("This is a string", 16, false);
    pr->putNotNullFieldIndicator();
    pr->put((void*) "\x11\x22\x60\x70\x80\x90", 6);
    pr->putNotNullFieldIndicator(false);
    pr->put((double) 2345.6789, false);
    pr->setEndPos();
    pr->resetPos();
    pTable->addRecord(pr);

    // record 10
    pr = new EncodedRecord();
    pr->alloc(100);
    pr->putNotNullFieldIndicator();
    pr->put(-20);
    pr->putNotNullFieldIndicator();
    pr->put(-2000000l);
    pr->putNotNullFieldIndicator(false);
    pr->put("This is a string", 16, false);
    pr->putNotNullFieldIndicator();
    pr->put((void*) "\x11\x22\x60\x70\x80\x90", 6);
    pr->putNullFieldIndicator(false);
    pr->setEndPos();
    pr->resetPos();
    pTable->addRecord(pr);

    // record 11
    pr = new EncodedRecord();
    pr->alloc(100);
    pr->putNotNullFieldIndicator();
    pr->put(10);
    pr->putNotNullFieldIndicator();
    pr->put(1000000l);
    pr->putNullFieldIndicator(false);
    pr->putNotNullFieldIndicator();
    pr->put((void*) "\x11\x22\x60\x70\x80\x90", 6);
    pr->putNotNullFieldIndicator(false);
    pr->put((double) 12345.6789, false);
    pr->setEndPos();
    pr->resetPos();
    pTable->addRecord(pr);

    // record 12
    pr = new EncodedRecord();
    pr->alloc(100);
    pr->putNotNullFieldIndicator();
    pr->put(-20);
    pr->putNotNullFieldIndicator();
    pr->put(-2000000l);
    pr->putNotNullFieldIndicator(false);
    pr->put("This is a string", 16, false);
    pr->putNotNullFieldIndicator();
    pr->put((void*) "\x11\x22\x60\x70\x80\x90", 6);
    pr->putNotNullFieldIndicator(false);
    pr->put((double) 123.456, false);
    pr->setEndPos();
    pr->resetPos();
    pTable->addRecord(pr);

    return pTable;
}

void display(EncodedRecord * pr, const RecordDef *ps) {
    int i;
    uint32_t len;
    for (i = 0; i < ps->getNumFields(); i++) {
        bool asc = ps->isAsc(i);
        if (pr->checkNullFieldIndicator(asc) ) {
            printf("NULL\t");
            continue;
        } 
        switch(ps->getType(i)) {
        case TYPE_INT:    printf("%d\t", pr->getInt(asc)); break;
        case TYPE_LONG:   printf("%ld\t", pr->getLong(asc)); break;
        case TYPE_DOUBLE: printf("%f\t", pr->getDouble(asc)); break;
        case TYPE_STRING: { const char* p = pr->getString(len, asc);
                            printf("%s\t", std::string(p, len).c_str()); break; }
        case TYPE_BOOL:   printf("%s\t", pr->getBool(asc) ? "true" : "false"); break;
        case TYPE_DATE:   printf("%s\t", toString(pr->getDate(asc)).c_str()); break;
        case TYPE_TIMESTAMP: printf("%s\t", toString(pr->getTimestamp(asc)).c_str()); break;
        case TYPE_BINARY:
        case TYPE_OBJECT: { uint8_t* p = pr->getBinary(len, asc);
                            printf("%s\t", toHexString((void*) p, len).c_str());
                            break; }
        case TYPE_NULL:  printf("NULL\t");
        }
    }
    printf("\n");
    pr->resetPos();
}

    
/**********************************
Displays all records of the table
**********************************/
void displayTable(Table* pt) {
    const RecordDef* ps = pt->getSchema();
    for (int i = 0; i < pt->getNumRecords(); i++) {
        display(pt->getRecord(i), ps);
    }
}

}
using namespace sope_test;

/************************************************
Example that:
a) Builds sample records into a table
b) Displays the table before sort
c) Sorts the table
d) Displays the table after sort
**************************************************/
int main(int argc, char** argv)
{
    Table* pt = buildRecords();
    std::cout << "Before sorting:" << std::endl;
    displayTable(pt);

    pt->sort();    

    std::cout << "After sorting:" << std::endl;
    std::cout << "==============================================================================" << std::endl;
    std::cout << "int       long         string(desc)          binary           double (desc) " << std::endl;
    std::cout << "------------------------------------------------------------------------------" << std::endl;
    displayTable(pt);

    //  printf("%s\n", toHexString((void*)pt->getRecord(0)->getData(), pt->getRecord(0)->getLen()).c_str()); 
    delete pt;

    return 0;
}


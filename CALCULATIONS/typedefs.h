#ifndef RDATA_H
#define RDATA_H

#include <stdint.h>

// for combined time and data value together
typedef struct RdataStruct {
    float time;
    uint32_t value;
} rData;

// for saving array data into a file
typedef struct dataStruct {
    int index_num;
    float time;
    uint32_t value;
} dataStruct;

#endif
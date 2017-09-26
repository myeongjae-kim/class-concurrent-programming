#pragma once

#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <pthread.h>

#define RESERVED_CAPACITY 1024
#define ALPHA_NUM 256

#define NUM_THREAD 48
// #define DBG

// class definition
class BMString : public std::string {
  public:
    BMString(const std::string& str);

    ~BMString() {
    }

    uint8_t badMatchTable[ALPHA_NUM];
    long patLength;

    char* BMH(const std::string& query);
};


std::string query(const std::string& query);

void* condSearchSubstr(void* arg);

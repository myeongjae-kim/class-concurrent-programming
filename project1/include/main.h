#pragma once

#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#include <set>
#include <unordered_set>
#include <pthread.h>

#include <boost/algorithm/searching/boyer_moore_horspool.hpp>

#define RESERVED_CAPACITY 1024
#define ALPHA_NUM 256

// #define DBG

using namespace boost::algorithm;

// class definition
class BMString : public std::string {
  public:
    BMString(const std::string& str);

    boyer_moore_horspool<std::string::const_iterator> search;

    ~BMString() {
    }

    uint8_t badMatchTable[ALPHA_NUM];
    long patLength;

    char* BMH(const std::string& query);
};


std::string query(const std::string& query);

#pragma once

#include <iostream>
#include <cstdlib>
#include <cassert>
#include <string>

#include <vector>
#include <boost/algorithm/searching/boyer_moore.hpp>

#include <set>

#define RESERVED_CAPACITY 1024

// #define DBG

using namespace boost::algorithm;

// class definition
class BMString : public std::string {
  public:
    BMString(const std::string str)
      : std::string(str), search(this->begin(), this->end()) {

      }
    boyer_moore<std::string::const_iterator> search;

    ~BMString() {
    }
};


std::string query(const std::string& query);

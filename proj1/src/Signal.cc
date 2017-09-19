#include <Signal.h>
#include <iostream>

Signal::Signal(const int n) {
  setNumberOfStrings(n);

  try {
    bf = new BloomFilter(n, 0.01);
  }catch(std::bad_alloc& e) {
    std::cerr << "(BloomFilter creation) " << e.what() << '\n'; 
    exit(-1);
  }
}

Signal::~Signal() {
  delete bf;
}

void Signal::setNumberOfStrings(const int n) {
  numberOfStrings = n;
}

int Signal::getNumberOfStrings() {
  return numberOfStrings;
}

std::string Signal::query(const std::string queryStr) {
  return "(query) " + queryStr + "\n(query) Not yet implemented";
}

void Signal::add(const std::string newStr) {
#ifdef DBG
  std::cout << "(add) " << newStr << std::endl;
#endif
  if (bf->lookup(newStr)) {
    // Do nothing. It is already exist
  } else {
    // insert
    bf->insert(newStr);

    // For finding substring in query,
    // We should remember lengths of strings in the bloom filter.
    int length = newStr.length();

    if (lengthsOfStrings.find(length) == lengthsOfStrings.end()) {
      // length is already exist
      __sync_fetch_and_add(&lengthsOfStrings[length], 1);
    } else {
      // length is not exist
      lengthsOfStrings[length] = 1;
    }

  }
}

void Signal::addParallel(const std::string newStr) {
  std::cout << "(addParallel) " << newStr << std::endl;
  std::cout << "(addParallel) Add a word to data structure parallel" << std::endl;
  std::cout << "(addParallel) Not yet implemented" << std::endl;
}


void Signal::del(const std::string toBeRemoved) {
#ifdef DBG
  std::cout << "(del) " << toBeRemoved << std::endl;
#endif

  if (bf->lookup(toBeRemoved)) {
    bf->remove(toBeRemoved);

    // For finding substring in query,
    // We should remember lengths of strings in the bloom filter.
    __sync_fetch_and_sub(&lengthsOfStrings[toBeRemoved.length()], 1);
  } else {
    // Do nothing. It is not exist
  }
}


bool Signal::isRegistered(const std::string str) {
  return bf->lookup(str) ? true : false;
}

void Signal::printLengthsOfStringsInBloomFilter() {
  for (auto i : lengthsOfStrings) {
    printf("length:%d, count:%d\n", i.first, i.second);
  }
}

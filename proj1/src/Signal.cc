#include <Signal.h>
#include <iostream>

Signal::Signal(const int n) {
  setNumberOfStrings(n);
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
  std::cout << "(add) " << newStr << std::endl;
  std::cout << "(add) Not yet implemented" << std::endl;
}

void Signal::addParallel(const std::string newStr) {
  std::cout << "(addParallel) " << newStr << std::endl;
  std::cout << "(addParallel) Add a word to data structure parallel" << std::endl;
  std::cout << "(addParallel) Not yet implemented" << std::endl;
}


void Signal::del(const std::string toBeRemoved) {
  std::cout << "(del) " << toBeRemoved << std::endl;
  std::cout << "(del) Not yet implemented" << std::endl;
}

Signal::~Signal() {
  // clear memory
}

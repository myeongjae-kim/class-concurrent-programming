#include <BloomFilter.h>
#include <cmath>
#include <iostream>
#include <main.h>

BloomFilter::BloomFilter (const uint64_t maxNumberOfElements, const double errProb) {
  this->maxNumberOfElements = maxNumberOfElements;

  // Below expression is from http://hur.st/bloomfilter
  filterSize = ceil((maxNumberOfElements * log(errProb)) / log(1.0 / (pow(2.0, log(2.0)))));
  numberOfHashFunctions = round(log(2.0) * filterSize / maxNumberOfElements);

  // TODO: remove below restriction
  // hash function should not be more than 10
  numberOfHashFunctions = numberOfHashFunctions > 10 ? 10 : numberOfHashFunctions;

  // TODO:new or malloc. which one is fast?
  try {
    filterArray = new uint8_t[filterSize];
  }catch(std::bad_alloc& e) {
    std::cerr << "(BloomFilter memory allocation is failed): " << e.what() << '\n'; 
    exit(-1);
  }

  // initialize
  for (uint64_t i = 0; i < filterSize; ++i) {
    filterArray[i] = 0;
  }

  // function pointer array create
  try {
    hashFuncs = new hashfunc_t[numberOfHashFunctions];
  }catch(std::bad_alloc& e) {
    std::cerr << "(Hash function array memory allocation is failed): " << e.what() << '\n'; 
    exit(-1);
  }

  // bind function pointers
  switch(numberOfHashFunctions)
  {
    default:
      ERROR_MSG("More hash functions are needed");
      std::cout << "Need: " << numberOfHashFunctions << ", Current have: " << 10 << std::endl;

    case 10:
      hashFuncs[9] = Hash10;
    case 9:
      hashFuncs[8] = Hash9;
    case 8:
      hashFuncs[7] = Hash8;
    case 7:
      hashFuncs[6] = Hash7;
    case 6:
      hashFuncs[5] = Hash6;
    case 5:
      hashFuncs[4] = Hash5;
    case 4:
      hashFuncs[3] = Hash4;
    case 3:
      hashFuncs[2] = Hash3;
    case 2:
      hashFuncs[1] = Hash2;
    case 1:
      hashFuncs[0] = Hash1;
      break;
  }
}

BloomFilter::~BloomFilter() {
  delete hashFuncs;
  delete filterArray;
}


uint64_t BloomFilter::getIndex(const int hashFuncReturn){
  return hashFuncReturn % filterSize;
}


void BloomFilter::insert(std::string str) {
  for (uint32_t i = 0; i < numberOfHashFunctions; ++i) {
    uint32_t idx = getIndex(hashFuncs[i](str));

    // Should it be parallelized?
    // make concurrent insertion!
    __sync_fetch_and_add(&filterArray[idx], 1);
  }
}

bool BloomFilter::lookup(std::string str) {
  for (uint32_t i = 0; i < numberOfHashFunctions; ++i) {
    uint32_t idx = getIndex(hashFuncs[i](str));
    if (filterArray[idx] == 0) {
      return false;
    }
  }
  return true;
}

void BloomFilter::remove(std::string str) {
  if (lookup(str) == false) {
    // do nothing
    return;
  }

  // exist!
  for (uint32_t i = 0; i < numberOfHashFunctions; ++i) {
    uint32_t idx = getIndex(hashFuncs[i](str));

    // Should it be parallelized?
    // make concurrent insertion!
    __sync_fetch_and_sub(&filterArray[idx], 1);
  }
}



/* hash functions */

uint64_t Hash1 (const std::string str)
{
    int HashVal=0;
    for(auto c: str)
        HashVal += c + 31;
    return HashVal;
}
uint64_t Hash2 (const std::string str)
{
    int HashVal=0;
    for(auto c: str)
        HashVal += c * 31;
    return HashVal;
}
uint64_t Hash3 (const std::string str)
{
    int HashVal=0;
    for(auto c: str)
        HashVal += (c + 31) *11;
    return HashVal;
}
uint64_t Hash4 (const std::string str)
{
    int HashVal=0;
    for(auto c: str)
        HashVal = (HashVal >> 5 ) + c++;
    return HashVal;
}
uint64_t Hash5 (const std::string str)
{
    int HashVal=0;
    for(auto c: str)
        HashVal = (HashVal << 5 ) + c++;
    return HashVal;
}
uint64_t Hash6 (const std::string str)
{
    int HashVal=0;
    for(auto c: str)
        HashVal = (HashVal << 5 ) + (HashVal >> 5 ) + c++;
    return HashVal;
}
uint64_t Hash7 (const std::string str)
{
    int HashVal=0;
    for(auto c: str)
        HashVal += (c * 11) +31;
    return HashVal;
}
uint64_t Hash8 (const std::string str)
{
    int HashVal=0;
    for(auto c: str)
        HashVal = ((HashVal << 5 ) + (HashVal >> 5 )) *11 + c++;
    return HashVal;
}
uint64_t Hash9 (const std::string str)
{
    int HashVal=0;
    for(auto c: str)
        HashVal = (HashVal >> 5 )*11 + c++;
    return HashVal;
}
uint64_t Hash10 (const std::string str)
{
    int HashVal=0;
    for(auto c: str)
        HashVal = (HashVal << 5 ) *11 + c++;
    return HashVal;
}


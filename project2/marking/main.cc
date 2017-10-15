#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include <cstdint>
#include <cstring>

#include "main.h"

int main(void)
{
  std::cout << std::endl << "** Marking program starts!! **" << std::endl;


  std::vector<std::string> file_data_per_line;

  uint64_t file_no = 0;
  while (1) {
    std::ifstream fin("thread" + std::to_string(++file_no) + ".txt");
    if ( ! fin.is_open() ) {
      --file_no;
      break;
    }
    
    std::string line_buffer;
    while( std::getline(fin, line_buffer) ) {
      file_data_per_line.push_back(line_buffer);
    }

    fin.close();
  }

  std::cout << "last file: thread" << std::to_string(file_no) << ".txt" << std::endl;

  uint64_t number_of_transactions = file_data_per_line.size();
  std::cout << "# of transactions: " << number_of_transactions << std::endl;

  std::vector<log_t> logs;

  for (auto &i : file_data_per_line) {
    /* std::cout << "before parsing data" << std::endl;
     * std::cout << i << std::endl; */


    // do parsing
    
    //[commit_id] [i] [j] [k] [value of record i] [value of record j] [value of record k]

    uint64_t token_start_position = 0;
    uint64_t token_size = 0;
    
    log_t log_buffer;

    auto c = i.begin();

    while (*c != ' ') {
      token_size++;
      c++;
    }
    log_buffer.commit_id = std::stol(i.substr(token_start_position, token_size));
    token_start_position += token_size + 1;
    token_size = 0;
    c++;

    while (*c != ' ') {
      token_size++;
      c++;
    }
    log_buffer.i = std::stol(i.substr(token_start_position, token_size));
    token_start_position += token_size + 1;
    token_size = 0;
    c++;

    while (*c != ' ') {
      token_size++;
      c++;
    }
    log_buffer.j = std::stol(i.substr(token_start_position, token_size));
    token_start_position += token_size + 1;
    token_size = 0;
    c++;

    while (*c != ' ') {
      token_size++;
      c++;
    }
    log_buffer.k = std::stol(i.substr(token_start_position, token_size));
    token_start_position += token_size + 1;
    token_size = 0;
    c++;

    while (*c != ' ') {
      token_size++;
      c++;
    }
    log_buffer.value_of_i = std::stol(i.substr(token_start_position, token_size));
    token_start_position += token_size + 1;
    token_size = 0;
    c++;

    while (*c != ' ') {
      token_size++;
      c++;
    }
    log_buffer.value_of_j = std::stol(i.substr(token_start_position, token_size));
    token_start_position += token_size + 1;
    token_size = 0;
    c++;

    while (*c != '\0') {
      token_size++;
      c++;
    }
    log_buffer.value_of_k = std::stol(i.substr(token_start_position, token_size));
    token_start_position += token_size + 1;
    token_size = 0;
    c++;


/*     std::cout << "parsed data" << std::endl;
 *
 *     std::cout << log_buffer.commit_id << " " << log_buffer.i << " " << log_buffer.j << " " << log_buffer.k << " "
 *       << log_buffer.value_of_i << " " << log_buffer.value_of_j << " " << log_buffer.value_of_k << std::endl; */

    logs.push_back(log_buffer);

    // std::cout << std::endl;
  }

  std::sort(logs.begin(), logs.end(), [](log_t lhs, log_t rhs){
        return lhs.commit_id < rhs.commit_id;
      });

  /* std::cout << "sorted parsed data" << std::endl;
   * for (auto log_buffer : logs) {
   *   std::cout << log_buffer.commit_id << " " << log_buffer.i << " " << log_buffer.j << " " << log_buffer.k << " "
   *     << log_buffer.value_of_i << " " << log_buffer.value_of_j << " " << log_buffer.value_of_k << std::endl;
   * } */

  bool wrong_found = false;
  uint64_t max_record_number = 0;
  for (auto i : logs) {
    max_record_number = std::max(max_record_number, i.i);
    max_record_number = std::max(max_record_number, i.j);
    max_record_number = std::max(max_record_number, i.k);
  }
  max_record_number++;
  std::cout << "max_record_number: " << max_record_number << std::endl;

  int64_t *records = (int64_t*)malloc(max_record_number * sizeof(*records));
  for (uint64_t i = 0; i < max_record_number; ++i) {
    records[i] = 100;
  }

  for (auto log : logs) {
    // do each transaction and comparison log
    log_t log_again;
    log_again.commit_id = log.commit_id;
    log_again.i = log.i;
    log_again.j = log.j;
    log_again.k = log.k;

    log_again.value_of_i = records[log_again.i];

    records[log_again.j] += log_again.value_of_i + 1;
    log_again.value_of_j = records[log_again.j];

    records[log_again.k] -= log_again.value_of_i;
    log_again.value_of_k = records[log_again.k];

    if (memcmp(&log_again, &log, sizeof(log_t)) != 0) {
      wrong_found = true;
      std::cout << "Commit_id = " << log.commit_id << ", different value is found" << std::endl;

      std::cout << "desired: ";
    std::cout << log_again.commit_id << " " << log_again.i << " " << log_again.j << " " << log_again.k << " "
      << log_again.value_of_i << " " << log_again.value_of_j << " " << log_again.value_of_k << std::endl;

      std::cout << "   real: ";
    std::cout << log.commit_id << " " << log.i << " " << log.j << " " << log.k << " "
      << log.value_of_i << " " << log.value_of_j << " " << log.value_of_k << std::endl;

    } else {
      // std::cout << "Commit_id = " << log.commit_id << ", correct!"<< std::endl;
    }
  }

  if (wrong_found == false) {
    std::cout << "Congratulations! All commits are correct!" << std::endl;
  }

  // uint64_t max_commit_id;
  // uint64_t max_record_number;

  free(records);
  return 0;
}

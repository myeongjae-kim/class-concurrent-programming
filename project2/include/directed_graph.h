/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : graph.h
 * Due date             : 2017-10-22
 * Compilation Standard : c++11 */

#ifndef __DIRECTED_GRAPH_H__
#define __DIRECTED_GRAPH_H__

#include <vector>
#include <unordered_set>
#include <iostream>
#include <queue>
#include <vector>

#include <cstdint>
#include <cassert>

// #define GRAPH_DBG
typedef struct _dest_and_count {
  uint64_t dest;
  int64_t count;

  bool operator==(struct _dest_and_count &rhs) {
    return dest == rhs.dest;
  }
  bool operator!=(struct _dest_and_count &rhs) {
    return dest != rhs.dest;
  }
  bool operator<=(struct _dest_and_count &rhs) {
    return dest <= rhs.dest;
  }
  bool operator>=(struct _dest_and_count &rhs) {
    return dest >= rhs.dest;
  }
  bool operator<(struct _dest_and_count &rhs) {
    return dest < rhs.dest;
  }
  bool operator>(struct _dest_and_count &rhs) {
    return dest > rhs.dest;
  }
}dest_and_count_t;

class directed_graph
{
public:
  directed_graph (uint64_t number_of_nodes);
  virtual ~directed_graph ();

  bool add_edge(uint64_t from, uint64_t to);
  bool remove_edge(uint64_t from, uint64_t to);

  std::vector<uint64_t> get_cycle(uint64_t from);

  void show_all_edges();
  void print_cycle(std::vector<uint64_t> &search_result);


private:
  uint64_t number_of_nodes;
  std::vector<dest_and_count_t> *nodes;

  bool is_node_exist(uint64_t node_number);

  bool get_cycle_recur(uint64_t from, uint64_t current_node,
      std::vector<uint64_t> &stack, std::unordered_set<uint64_t> &visited);

};

#endif

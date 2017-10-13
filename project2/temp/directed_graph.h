/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : graph.h
 * Due date             : 2017-10-22
 * Compilation Standard : c++11 */

#ifndef DIRECTED_GRAPH_H
#define DIRECTED_GRAPH_H

#include <unordered_set>
#include <iostream>
#include <queue>
#include <vector>

#include <cstdint>
#include <cassert>

#define GRAPH_DBG

class directed_graph
{
public:
  directed_graph (uint64_t number_of_nodes);
  virtual ~directed_graph ();

  bool add_edge(uint64_t from, uint64_t to);
  bool remove_edge(uint64_t from, uint64_t to);

  bool has_cycle_start_from(uint64_t node_number);

  
  std::vector<uint64_t> get_cycle(uint64_t from);

  void show_all_edges();

private:
  uint64_t number_of_nodes;
  std::unordered_set<uint64_t> *nodes;

  bool is_node_exist(uint64_t node_number);


  bool get_cycle_recur(uint64_t from, std::vector<uint64_t> &stack, std::unordered_set<uint64_t> &visited);

};

#endif

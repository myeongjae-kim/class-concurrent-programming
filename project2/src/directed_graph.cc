/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : directed_graph.cc
 * Due date             : 2017-10-22
 * Compilation Standard : c++11 */

#include "directed_graph.h"

// constructor
directed_graph::directed_graph (uint64_t number_of_nodes) {
  this->number_of_nodes = number_of_nodes;

  // create nodes
  nodes = new std::unordered_set<uint64_t>[this->number_of_nodes];
}

directed_graph::~directed_graph () {
  // destroy nodes
  delete[] nodes;
}

bool directed_graph::is_node_exist(uint64_t node_number) {
  // check whether node_number is exist
  if(node_number >= number_of_nodes) {

    std::cout << "(directed_graph::check_node_number) Failed. node_number "
      << node_number
      << " is same or bigger than number_of_nodes "
      << number_of_nodes << std::endl;

    return false;
  }

  return true;
}

bool directed_graph::add_edge(uint64_t from, uint64_t to) {
  // check whether node is exist
  if ( ! (is_node_exist(from) && is_node_exist(to)) ) {
    return false;
  }

  // add an edge
  nodes[from].insert(to);
  return true;
}

bool directed_graph::remove_edge(uint64_t from, uint64_t to) {
  // check whether node is exist
  if ( ! (is_node_exist(from) && is_node_exist(to)) ) {
    return false;
  }

  // remove an edge
  nodes[from].erase(to);
  return true;
}

void directed_graph::show_all_edges() {
  uint64_t edge_count = 0;

  // node is start from one.
  std::cout << "** Printing All Edges **" << std::endl;

  // iterate all nodes
  for (uint64_t from = 0; from < number_of_nodes; ++from) {

    // iterate all edges in each node
    for (auto to : nodes[from]) {
      std::cout << "Edge from "<< from << "\tto " << to << std::endl;
      edge_count++;
    }
  }

  std::cout << "The number of edges: " << edge_count << std::endl;
  std::cout << "**  Printing is end   **" << std::endl;
}


// This function finds a cycle that includes 'from' node.
// If cycle is found, the return vector has member nodes of cycle.
// If not found, return vector is empty.
std::vector<uint64_t> directed_graph::get_cycle(uint64_t from) {
  // initialize data structures.
  
  // The cycle_member_nodes is used to save cycle members.
  std::vector<uint64_t> cycle_member_nodes;

  // The visited set has nodes that have been visited.
  std::unordered_set<uint64_t> visited;

  // DFS
  for (auto to : nodes[from]) {
    // push destination to stack

#ifdef GRAPH_DBG
    std::cout << "(get_cycle) ** Start finding a cycle that includes " << from << " **" << std::endl;
    std::cout << "(get_cycle)       from " << from <<" to " << to << std::endl;
#endif

    // Call recursive function
    if (get_cycle_recur(from, to, cycle_member_nodes, visited)) {
      cycle_member_nodes.push_back(to);

#ifdef GRAPH_DBG
      std::cout << "(get_cycle) cycle found" << std::endl;
      std::cout << "(get_cycle) " << cycle_member_nodes.back();

      
      for (uint64_t i = cycle_member_nodes.size() - 2; i < cycle_member_nodes.size(); --i) {
        std::cout << " -> " << cycle_member_nodes[i];
      }
      std::cout << std::endl;

      std::cout << "(get_cycle) **  End finding a cycle that includes " << from << "  **" << std::endl;
#endif

      return cycle_member_nodes;
    }

  }

#ifdef GRAPH_DBG
    std::cout << "(get_cycle) cycle not found" << std::endl;
    std::cout << "(get_cycle) **  End finding a cycle that includes " << from << "  **" << std::endl;
#endif


  return cycle_member_nodes;
}


// This is a function used to find cycle in get_cycle.
// If cycle is found, it reutrns ture.
// If not, it returns false.
bool directed_graph::get_cycle_recur(uint64_t from,
                                uint64_t current_node,
                                std::vector<uint64_t> &cycle_member_nodes,
                                std::unordered_set<uint64_t> &visited){
  // Base case
  visited.insert(current_node);

  if (nodes[current_node].empty()) {
    // There is no cycle.
    return false;
  } else if (current_node == from) {
    // cycle is found
    return true;
  }

  // Recursion process
  for (auto to : nodes[current_node]) {
    // do not visit node again
    if (visited.find(to) == visited.end()) {

#ifdef GRAPH_DBG
      std::cout << "(get_cycle_recur) from " << current_node <<" to " << to << std::endl;
#endif

      if (get_cycle_recur(from, to, cycle_member_nodes, visited)) {
        cycle_member_nodes.push_back(to);
        return true;
      }

    }
  }

  return false;
}


void directed_graph::print_cycle(std::vector<uint64_t> &search_result) {
  if (search_result.size() == 0) {
    std::cout << "(print_cycle) ** No cycle found **" << std::endl;
    return;
  }

  std::cout << "(print_cycle) ** Cycle found **" << std::endl;
  std::cout << "(print_cycle)    "<< search_result[0];

  uint64_t result_size = search_result.size();
  for (uint64_t i = 1; i < result_size; ++i) {
    std::cout << " -> " << search_result[i];
  }
  std::cout << std::endl;
  std::cout << "(print_cycle) **     End     **" << std::endl;
}


#include "directed_graph.h"

#include <iostream>

void temp1(directed_graph &G, std::vector<uint64_t>& cycle_member) {
  cycle_member = G.get_cycle(3);
}

int main(void)
{
  directed_graph G(11);

  G.add_edge(1, 2);
  G.add_edge(2, 3);
  G.add_edge(3, 4);
  G.add_edge(4, 5);
  G.add_edge(5, 6);
  G.add_edge(6, 7);
  G.add_edge(7, 8);
  G.add_edge(8, 9);
  G.add_edge(9, 10);

  G.add_edge(10, 5);
  G.add_edge(4, 2);

  G.show_all_edges();


  std::vector<uint64_t> && find_result = G.get_cycle(7);
  G.print_cycle(find_result);

  temp1(G, find_result);
  G.print_cycle(find_result);

  find_result = G.get_cycle(7);
  G.print_cycle(find_result);

  find_result = G.get_cycle(3);
  G.print_cycle(find_result);


  find_result = G.get_cycle(1);
  G.print_cycle(find_result);

}

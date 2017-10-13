#include "directed_graph.h"

#include <iostream>

void directed_graph_test()
{
  directed_graph G(10);

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

  G.has_cycle_start_from(1);

  std::cout << std::endl;

  G.has_cycle_start_from(7);

  G.get_cycle(7);

  G.get_cycle(3);

  G.get_cycle(1);
}

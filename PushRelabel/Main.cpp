#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include "graph.hpp"

int main() {
	graph g = graph();
	std::unordered_map<int, node *> node_map;

	string line;
	std::ifstream file;
	file.open("graph_generator.txt");

	if (file.is_open()) {
		while (getline(file, line)) {
			std::stringstream s(line);
			string s_u_id, s_v_id, s_capacity;

			s >> s_u_id >> s_v_id >> s_capacity;
			int u_id = stoi(s_u_id), v_id = stoi(s_v_id);

			if (node_map.find(u_id) == node_map.end()) {
				node* u = new node(u_id);
				g.add_node(u);
				node_map.insert({u_id, u});
			}

			if (node_map.find(v_id) == node_map.end()) {
				node *v = new node(v_id);
				g.add_node(v);
				node_map.insert({v_id, v});
			}

			g.add_edge(*node_map[u_id], *node_map[v_id], stoi(s_capacity));

		}

		int flux = g.get_max_flow(*node_map[1], *node_map[4]);
		cout << "max flux: " << flux;

		return 0;
	}
}


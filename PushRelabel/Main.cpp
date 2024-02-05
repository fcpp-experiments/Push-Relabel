#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <map>
#include <algorithm>
#include "graph.hpp"
#include "tests.hpp"

#include <chrono>

void create_graph_file();
graph get_graph();
void create_test_aggregate(string);

std::unordered_map<int, node*> node_map;
std::map<int, std::vector<pair<int, string>>> string_map;

int main() {
	//create_graph_file();
	//create_test_aggregate("test1");

	graph g = get_graph();

	auto start = chrono::high_resolution_clock::now();

	long long flux = g.get_max_flow(*node_map[1], *node_map[126]);

	auto stop = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<chrono::microseconds> (stop - start);

	cout << "max flux: " << flux << " - time: " << duration.count() << "\n";

	tests::start_tests();

	return 0;
}

graph get_graph() {
	graph g = graph();
	string line;
	std::ifstream file;
	file.open("graph4.txt");

	if (file.is_open()) {
		while (getline(file, line)) {
			std::stringstream s(line);
			string s_u_id, s_v_id, s_capacity;

			s >> s_u_id >> s_v_id >> s_capacity;
			int u_id = stoi(s_u_id), v_id = stoi(s_v_id);

			if (node_map.find(u_id) == node_map.end()) {
				node* u = new node(u_id);
				g.add_node(u);
				node_map.insert({ u_id, u });
			}

			if (node_map.find(v_id) == node_map.end()) {
				node* v = new node(v_id);
				g.add_node(v);
				node_map.insert({ v_id, v });
			}

			g.add_edge(*node_map[u_id], *node_map[v_id], stoll(s_capacity));

		}
	}

	return g;
}

// create a random graph and saves it into a file ("graph1.txt")
void create_graph_file() {
	int n_nodes = 500; // number of nodes of the random graph

	vector<int> node_neighbors;
	ofstream myfile;
	myfile.open("graph1.txt");

	srand((unsigned)time(NULL));

	for (int i = 1; i <= n_nodes; i++) {

		int n = 10 + rand() % 20; // number of children

		for (int k = 0; k < n; k++) {
			string new_line = "";

			int n2 = 1 + rand() % n_nodes;
			while (n2 == i || find(node_neighbors.begin(), node_neighbors.end(), n2) != node_neighbors.end()) {
				n2 = 1 + rand() % n_nodes;
			}

			node_neighbors.push_back(n2);
			int n3 = 5 + rand() % 20; // capacity of new edge

			new_line += to_string(i) + " " + to_string(n2) + " " + to_string(n3);
			new_line += "\n";

			myfile << new_line;
		}
		node_neighbors = vector<int>();
	}

	myfile.close();
}

void create_test_aggregate(string file_name) {
	graph g = graph();
	string line, s_arcs = "", s_nodes= "";
	std::ifstream file;
	std::ofstream nodes, arcs;
	file.open("test_files\\" + file_name + ".txt");
	nodes.open("input\\" + file_name + ".nodes");
	arcs.open("input\\" + file_name  + ".arcs");

	if (file.is_open()) {
		while (getline(file, line)) {
			std::stringstream s(line);
			string s_u_id, s_v_id, s_capacity;

			s >> s_u_id >> s_v_id >> s_capacity;
			int u_id = stoi(s_u_id), v_id = stoi(s_v_id);

			s_arcs += s_u_id + "\t" + s_v_id + "\n";
			if(string_map.find(stoi(s_v_id)) == string_map.end()){
				string_map[stoi(s_v_id)];
			}
			string_map[stoi(s_u_id)].push_back({ stoi(s_v_id), s_capacity});
		}
	}

	arcs << s_arcs;
	srand(time(NULL));
	
	for (auto& line : string_map) {
		sort(line.second.begin(), line.second.end());
		int n1 = rand() % 500, n2 = rand() % 500;
		string capacities = "{";
		for (pair<int, string> p : line.second) {
			capacities += to_string(p.first) + ": " + p.second + ", ";
		}
		capacities += "*: 0}\n";

		s_nodes += to_string(line.first) + " \t[" + to_string(n1) + ", " + to_string(n2) + "]\t " + capacities;
	}

	nodes << s_nodes;

	file.close();
	nodes.close();
	arcs.close();
}

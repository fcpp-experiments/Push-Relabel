#include <algorithm>
#include <chrono>
#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>

#include "../lib/openmp.hpp"


void create_graph_file();
graph get_graph(string);
void create_test_aggregate(string);

std::unordered_map<int, node*> node_map;
std::map<int, std::vector<pair<int, string>>> string_map;

std::map<pair<int, int>, long long> string_map2;

int main() {

	// create_graph_file();
	//create_test_aggregate("test21");

	// graph g = get_graph("../test_files/test1.txt");
	// auto start = chrono::high_resolution_clock::now();

	// long long flow = g.get_max_flow(*node_map[1], *node_map[4]);

	// auto stop = chrono::high_resolution_clock::now();
	// auto duration = chrono::duration_cast<chrono::microseconds> (stop - start);

	// cout << "max flux: " << flow << " - time: " << duration.count() << "\n";

	tests::start_tests();

	return 0;
}

graph get_graph(string file_name) {
	graph g = graph();
	string line;
	std::ifstream file;
	file.open(file_name);

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
		file.close();
	}

	return g;
}

// create a random graph and saves it into a file ("graph1.txt")
void create_graph_file() {
	int n_nodes = 500; // number of nodes of the random graph

	vector<int> node_neighbors;
	ofstream myfile;
	myfile.open("../test_files/graph1.txt");

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
	file.open("..\\test_files\\" + file_name + ".txt");
	nodes.open("..\\new_inputs\\" + file_name + ".nodes");
	arcs.open("..\\new_inputs\\" + file_name  + ".arcs");

	if (file.is_open()) {
		while (getline(file, line)) {
			std::stringstream s(line);
			string s_u_id, s_v_id, s_capacity;

			s >> s_u_id >> s_v_id >> s_capacity;
			int u_id = stoi(s_u_id), v_id = stoi(s_v_id);

			if(string_map2.find({u_id, v_id}) == string_map2.end()){
				string_map2[{ u_id, v_id }] = stoll(s_capacity);
				string_map2[{ v_id, -1 }] = 0ll;
			}else{
				string_map2[{ u_id, v_id }] += stoll(s_capacity);
			}
		}
	}

	srand(time(NULL));
	int last_node_u = -1;
	string capacities = "";
	for (auto& line : string_map2) {
		if(get<1>(line.first) != -1){
			s_arcs += to_string(get<0>(line.first)) + "\t" + to_string(get<1>(line.first)) + "\n";
		}

		if(get<0>(line.first) != last_node_u){
			last_node_u = get<0>(line.first);
			s_nodes += capacities + "" + to_string(last_node_u) + "\t";
			capacities = "{*: 0}\n";
		}
		if(get<1>(line.first) != -1){
			capacities.insert(capacities.size() - 6, to_string(get<1>(line.first)) + ": " + to_string(line.second) + ", ");
		}
	}

	s_nodes += capacities;

	arcs << s_arcs;
	nodes << s_nodes;

	file.close();
	nodes.close();
	arcs.close();
}

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <cassert>

#include "graph.hpp"


namespace tests {

	inline graph get_graph_from_file(string, std::unordered_map<int, node*>&);
	inline void test(string, int, long long);

	inline void start_tests() {
		std::cout << "TEST START:\n";
		test("test_files\\test1.txt", 4, 3);
		std::cout << "TEST 1 OK\n";
		test("test_files\\test2.txt", 4, 2);
		std::cout << "TEST 2 OK\n";
		test("test_files\\test3.txt", 4, 2000000000);
		std::cout << "TEST 3 OK\n";
		test("test_files\\test4.txt", 2, 0);
		std::cout << "TEST 4 OK\n";
		test("test_files\\test5.txt", 2, 1000000000000);
		std::cout << "TEST 5 OK\n";
		test("test_files\\test6.txt", 500, 60);
		std::cout << "TEST 6 OK\n";
		test("test_files\\test7.txt", 500, 1093765123);
		std::cout << "TEST 7 OK\n";
		test("test_files\\test8.txt", 2, 1);
		std::cout << "TEST 8 OK\n";
		test("test_files\\test9.txt", 4, 6);
		std::cout << "TEST 9 OK\n";
		test("test_files\\test10.txt", 4, 3);
		std::cout << "TEST 10 OK\n";
		test("test_files\\test11.txt", 10, 111000000000);
		std::cout << "TEST 11 OK\n";
		test("test_files\\test12.txt", 7, 2);
		std::cout << "TEST 12 OK\n";
		test("test_files\\test13.txt", 498, 248);
		std::cout << "TEST 13 OK\n";
		test("test_files\\test14.txt", 8, 1);
		std::cout << "TEST 14 OK\n";
		test("test_files\\test15.txt", 122, 1);
		std::cout << "TEST 15 OK\n";
		test("test_files\\test16.txt", 4, 150);
		std::cout << "TEST 16 OK\n";
		test("test_files\\test17.txt", 499, 510000000);
		std::cout << "TEST 17 OK\n";
		test("test_files\\test18.txt", 334, 130988929122);
		std::cout << "TEST 18 OK\n";
		test("test_files\\test19.txt", 442, 32724);
		std::cout << "TEST 19 OK\n";
		test("test_files\\test20.txt", 126, 2147483648);
		std::cout << "TEST 20 OK\n";
		test("test_files\\test21.txt", 5, 3);
		std::cout << "TEST 21 OK\n";
	}

	void test(string file_name, int last_node, long long result) {
		std::unordered_map<int, node*> node_map;
		graph g = get_graph_from_file(file_name, node_map);
		assert(g.get_max_flow(*node_map[1], *node_map[last_node]) == result);
	}

	graph get_graph_from_file(string file_name, unordered_map<int, node*>& node_map) {
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
		}

		return g;
	}
}

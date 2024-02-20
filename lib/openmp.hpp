#ifndef PUSH_RELABEL_H
#define PUSH_RELABEL_H

#include <cassert>
#include <climits>

#if defined(_OPENMP)
#include <omp.h>
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

struct edge;

struct node {
	int id;
	int height;
	long long e_flow; // excess flow
	vector<edge*> neighbors;

	node(int id) {
		this->id = id;
		this->height = 0;
		this->e_flow = 0;
	}

	node(int id, int height, long long int e_flow) {
		this->id = id;
		this->height = height;
		this->e_flow = e_flow;
	}
};

struct edge {
	node* u;
	node* v;
	long long int capacity;
	long long flow = 0, d_flow_u = 0, d_flow_v = 0;

	edge(node* u, node* v, long long int capacity, long long int flow = 0) {
		this->capacity = capacity;
		this->flow = flow;
		this->u = u;
		this->v = v;
		u->neighbors.push_back(this);
	}
};

class graph {
	vector<node*> nodes;
	vector<edge*> edges;

public:
	void add_node(node* new_node) {
		nodes.push_back(new_node);
	}

	void add_edge(node& u, node& v, long long int capacity) {
		bool found = false;
		for (edge* e : u.neighbors) {
			if (&v == e->v && e->capacity == 0) {
				found = true;
				e->capacity = capacity;
				break;
			}
		}

		if (!found) {
			edges.push_back(new edge(&u, &v, capacity));
		}

		for (edge* e : v.neighbors) {
			if (&u == e->v) {
				return;
			}
		}

		// reverse edge does not exist: add it
		edges.push_back(new edge(&v, &u, 0, 0));
	}

	long long int get_max_flow(node& source, node& t) {
		int remaining = 1;

		preflow(source);

		while (remaining > 0) {
			remaining = 0;

			#pragma omp parallel for
			for (int i = 0; i < nodes.size(); i++) {
				if (&source != nodes.at(i) && nodes.at(i) != &t && nodes.at(i)->e_flow > 0) {
					#pragma omp atomic
					remaining++;

					discharge(*nodes.at(i));
				}
			}
			normalize_edges_flow();
		}

		return t.e_flow;
	}

private:
	void preflow(node& source) {
		source.height = static_cast<int>(nodes.size());
		for (edge* e : source.neighbors) {
			e->flow = e->capacity;
			e->v->e_flow += e->flow;

			edges.push_back(new edge(e->v, e->u, 0, -e->flow));
		}
	}
	
	void normalize_edges_flow() {
		for (edge* e : edges) {
			e->flow += (e->d_flow_u + e->d_flow_v);
			e->u->e_flow -= e->d_flow_u;
			e->v->e_flow += e->d_flow_u;

			e->d_flow_u = 0;
			e->d_flow_v = 0;
		}
	}

	void reverse_edge_flow(edge& e_param, long long int flow) {
		for (edge* e : e_param.v->neighbors) {
			if (e_param.u == e->v) {
				e->d_flow_v -= flow;
				return;
			}
		}
	}

	void push(edge& e) {
		long long int flow = min(e.capacity - e.flow, e.u->e_flow);

		e.d_flow_u += flow;

		reverse_edge_flow(e, flow);
	}

	void discharge(node& u) {
		int curr_edge = 0, min_height = INT_MAX;
		edge* min_edge = u.neighbors.at(0);

		for (edge* e : u.neighbors) { // min_hood
			if (e->capacity - e->flow > 0 && e->v->height < min_height) { // mux
				min_height = e->v->height;
				min_edge = e;
			}
		}

		if (u.height > min_height) {
			push(*min_edge);
		}
		else {
			u.height = min_height + 1; // relabel
		}

		min_height = INT_MAX;
	}
};


namespace tests {

    inline graph get_graph_from_file(string, std::unordered_map<int, node*>&);
    inline void test(string, int, long long);
	long long get_flow_multiple(graph, vector<int>, vector<int>, std::unordered_map<int, node*>&);

    inline void start_tests() {
        std::cout << "TEST START:\n";
        test("..\\test_files\\test1.txt", 4, 3);
        std::cout << "TEST 1 OK\n";
        test("..\\test_files\\test2.txt", 4, 2);
        std::cout << "TEST 2 OK\n";
        test("..\\test_files\\test3.txt", 4, 2000000000);
        std::cout << "TEST 3 OK\n";
        test("..\\test_files\\test4.txt", 500, 60);
        std::cout << "TEST 4 OK\n";
        test("..\\test_files\\test5.txt", 500, 1093765123);
        std::cout << "TEST 5 OK\n";
        test("..\\test_files\\test6.txt", 4, 6);
        std::cout << "TEST 6 OK\n";
        test("..\\test_files\\test7.txt", 4, 3);
        std::cout << "TEST 7 OK\n";
        test("..\\test_files\\test8.txt", 10, 111000000000);
        std::cout << "TEST 8 OK\n";
        test("..\\test_files\\test9.txt", 7, 2);
        std::cout << "TEST 9 OK\n";
        test("..\\test_files\\test10.txt", 498, 248);
        std::cout << "TEST 10 OK\n";
        test("..\\test_files\\test11.txt", 8, 1);
        std::cout << "TEST 11 OK\n";
        test("..\\test_files\\test12.txt", 122, 1);
        std::cout << "TEST 12 OK\n";
        test("..\\test_files\\test13.txt", 4, 150);
        std::cout << "TEST 13 OK\n";
        test("..\\test_files\\test14.txt", 499, 510000000);
        std::cout << "TEST 14 OK\n";
        test("..\\test_files\\test15.txt", 334, 130988929122);
        std::cout << "TEST 15 OK\n";
        test("..\\test_files\\test16.txt", 442, 32724);
        std::cout << "TEST 16 OK\n";
        test("..\\test_files\\test17.txt", 126, 2147483648);
        std::cout << "TEST 17 OK\n";
        test("..\\test_files\\test18.txt", 5, 3);
        std::cout << "TEST 18 OK\n";
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

	inline vector<long long> get_flows(string file_name, int n_nodes){
		vector<long long> results;

		std::unordered_map<int, node*> node_map;
		graph g = get_graph_from_file(file_name, node_map);
		results.push_back(get_flow_multiple(g, vector<int>{1}, vector<int>{n_nodes}, node_map));
		
		node_map.clear();
		g = get_graph_from_file(file_name, node_map);
		results.push_back(get_flow_multiple(g, vector<int>{1, 2}, vector<int>{n_nodes}, node_map));

		node_map.clear();
		g = get_graph_from_file(file_name, node_map);
		results.push_back(get_flow_multiple(g, vector<int>{1, 2}, vector<int>{n_nodes, n_nodes - 1}, node_map));

		node_map.clear();
		g = get_graph_from_file(file_name, node_map);
		results.push_back(get_flow_multiple(g, vector<int>{2}, vector<int>{n_nodes, n_nodes - 1}, node_map));

		node_map.clear();
		g = get_graph_from_file(file_name, node_map);
		int last_test = get_flow_multiple(g, vector<int>{2}, vector<int>{n_nodes - 1}, node_map);
		results.push_back(last_test);
		results.push_back(last_test);

		return results;
	}

	long long get_flow_multiple(graph g, vector<int> sources, vector<int> sinks, unordered_map<int, node*>& node_map){
		// Add virtual source
		node* v_source = new node(0);
		g.add_node(v_source);

		for(int s : sources){
			g.add_edge(*v_source, *node_map[s], LLONG_MAX);
		}

		// Add virtual sink
		node* v_sink = new node(999);
		g.add_node(v_sink);

		for(int s : sinks){
			g.add_edge(*node_map[s], *v_sink, LLONG_MAX);
		}

		return g.get_max_flow(*v_source, *v_sink);
	}
}

#endif

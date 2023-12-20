#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <omp.h>
#include <iostream>

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

#endif
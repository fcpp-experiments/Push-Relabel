#include <cassert>
#include <vector>

using namespace std;

struct node {
	int id;
	int height;
	int e_flow; //excess flow

	node() : id(0), height(0), e_flow(0){}

	node(int id) {
		this->id = id;
		this->height = 0;
		this->e_flow = 0;
	}

	node(int id, int height, int e_flow) {
		this->id = id;
		this->height = height;
		this->e_flow = e_flow;
	}
};

struct edge {
	node *u;
	node *v;
	int capacity;
	int flow;

	edge(node *u, node *v, int capacity, int flow = 0) {
		this->capacity = capacity;
		this->flow = flow;
		this->u = u;
		this->v = v;
	}
};

class graph {
	vector<node *> nodes;
	vector<edge> edges;

public:
	void add_node(node *new_node) {
		nodes.push_back(new_node);
	}

	void add_edge(node& u, node& v, int capacity) {
		edge new_edge = edge(&u, &v, capacity);
		edges.push_back(new_edge);
	}


	int get_max_flow(node& s, node& t) {
		preflow(s);

		node* over_flow_node;
		/*while ((over_flow_node = get_over_flow_node(s, t)) != nullptr) {
			if (!push(*over_flow_node)) {
				relabel(*over_flow_node);
			}
		}*/


		while ((over_flow_node = get_over_flow_node(s, t)) != nullptr) {
			if (over_flow_node != &s && over_flow_node != &t) {
				discharge(*over_flow_node);
			}
		}


		return t.e_flow;
	}

private:
	void preflow(node& source) {
		source.height = static_cast<int>(nodes.size());

		for (edge& e : edges) {
			if (e.u == &source) {
				e.flow = e.capacity;
				e.v->e_flow += e.flow;
				edges.push_back(edge(e.v, e.u, 0, -e.flow));
			}
		}
	}

	node* get_over_flow_node(node& source, node& t) {
		for (node* n : nodes) {
			if (&source != n && n != &t && n->e_flow > 0) {
				return n;
			}
		}
		return nullptr;
	}

	void reverse_edge_flow(edge& e_param, int flow) {
		for (edge& e : edges) {
			if (e_param.u == e.v && e_param.v == e.u) {
				e.flow -= flow;
				return;
			}
		}

		// reverse edge does not exist: add it
		edge a = edge(e_param.v, e_param.u, 0, -flow);
		edges.push_back(a);
	}

	bool push(node& u) {
		for (edge& e : edges) {
			if (e.u == &u) {
				if ((u.height > e.v->height) && (e.flow != e.capacity)) {
					int flow = min(e.capacity - e.flow, u.e_flow);

					u.e_flow -= flow;
					e.v->e_flow += flow;
					e.flow += flow;

					reverse_edge_flow(e, flow);

					return true;
				}
			}
		}
		return false;
	}

	void relabel(node& u) {
		int max_height = INT_MAX;
		for (edge& e : edges) {
			if (e.u == &u) {
				if (e.flow == e.capacity) {
					continue;
				}
				
				if (e.v->height < max_height) {
					max_height = e.v->height;
					u.height = max_height + 1;
				}
			}
		}
	}

	void push2(edge& e) {
		int flow = min(e.capacity - e.flow, e.u->e_flow);

		e.u->e_flow -= flow;
		e.v->e_flow += flow;
		e.flow += flow;

		reverse_edge_flow(e, flow);
	}

	void discharge(node& u) {
		int curr_edge = 0;
		while (u.e_flow > 0) {
			if (curr_edge >= edges.size()) {
				relabel(u);
				curr_edge = 0;
			}
			else {
				if (edges.at(curr_edge).u->height == edges.at(curr_edge).v->height + 1) {
					push2(edges.at(curr_edge));
				}
				curr_edge++;
			}
		}
	}
};

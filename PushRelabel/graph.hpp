#include <vector>
#include <omp.h>

using namespace std;

struct edge;

struct node {
	int id;
	int height;
	int e_flow; //excess flow
	vector<edge *> neighbors;
	omp_lock_t writelock;

	node() : id(0), height(0), e_flow(0){
		omp_init_lock(&writelock);
	}

	node(int id) {
		omp_init_lock(&writelock);
		this->id = id;
		this->height = 0;
		this->e_flow = 0;
	}

	node(int id, int height, int e_flow) {
		omp_init_lock(&writelock);
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
	omp_lock_t writelock;

	edge(node *u, node *v, int capacity, int flow = 0) {
		this->capacity = capacity;
		this->flow = flow;
		this->u = u;
		this->v = v;
		omp_set_lock(&u->writelock);
		u->neighbors.push_back(this);
		omp_unset_lock(&u->writelock);
	}
};

class graph {
	vector<node *> nodes;
	vector<edge *> edges;
	omp_lock_t g_writelock;

public:

	graph() {
		omp_init_lock(&g_writelock);
	}

	void add_node(node *new_node) {
		nodes.push_back(new_node);
	}

	void add_edge(node& u, node& v, int capacity) {
		edges.push_back(new edge(&u, &v, capacity));
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


	int get_max_flow_parallel(node& s, node& t) {;
		preflow(s);
		node* over_flow_node;

		#pragma omp parallel
		{
			#pragma omp single
			{
				vector<node *> over_nodes = get_over_flow_nodes(s, t);
				while (over_nodes.size() > 0) {
					for (node *n : over_nodes) {
						#pragma omp task
						discharge(*n);
					}
					#pragma omp taskwait
					over_nodes = get_over_flow_nodes(s, t);
				}
			}
		}

		return t.e_flow;
	}

private:
	void preflow(node& source) {
		vector<edge *> new_edges;

		source.height = static_cast<int>(nodes.size());
		for (edge *e : edges) {
			if (e->u == &source) {
				e->flow = e->capacity;
				e->v->e_flow += e->flow;

				new_edges.push_back(new edge(e->v, e->u, 0, -e->flow));
			}
		}
		edges.insert(edges.end(), new_edges.begin(), new_edges.end());
	}

	//return first node that has excess flow
	node* get_over_flow_node(node& source, node& t) {
		for (node* n : nodes) {
			if (&source != n && n != &t && n->e_flow > 0) {
				return n;
			}
		}
		return nullptr;
	}

	//return vector containing all nodes that have excess flow
	vector<node *> get_over_flow_nodes(node& source, node& t) {
		vector<node *> over_nodes;
		for (node* n : nodes) {
			if (&source != n && n != &t && n->e_flow > 0) {
				over_nodes.push_back(n);
			}
		}
		return over_nodes;
	}

	void reverse_edge_flow(edge& e_param, int flow) {
		omp_set_lock(&e_param.v->writelock);
		for (edge *e : e_param.v->neighbors) {
			if (e_param.u == e->v) {
				e->flow -= flow;
				omp_unset_lock(&e_param.v->writelock);
				return;
			}
		}
		omp_unset_lock(&e_param.v->writelock);

		// reverse edge does not exist: add it
		omp_set_lock(&g_writelock);
		edges.push_back(new edge(e_param.v, e_param.u, 0, -flow));
		omp_unset_lock(&g_writelock);
	}

	bool push(node& u) {
		for (edge *e : edges) {
			if (e->u == &u) {
				if ((u.height > e->v->height) && (e->flow != e->capacity)) {
					int flow = min(e->capacity - e->flow, u.e_flow);

					u.e_flow -= flow;
					e->v->e_flow += flow;
					e->flow += flow;

					reverse_edge_flow(*e, flow);

					return true;
				}
			}
		}
		return false;
	}

	void relabel(node& u) {
		int max_height = INT_MAX;

		for (edge *e : u.neighbors) {
			node* test = e->u;
			if (e->u == &u) {
				if (e->flow == e->capacity) {
					continue;
				}

				if (e->v->height < max_height) {
					max_height = e->v->height;
					u.height = max_height + 1;
				}
			}
		}
	}

	void push2(edge& e) {
		int flow = min(e.capacity - e.flow, e.u->e_flow);

		e.u->e_flow -= flow;
		omp_unset_lock(&e.u->writelock);
		
		omp_set_lock(&e.v->writelock);
		e.v->e_flow += flow;
		omp_unset_lock(&e.v->writelock);

		e.flow += flow;

		reverse_edge_flow(e, flow);
		omp_set_lock(&e.u->writelock);
	}

	void discharge(node& u) {
		omp_set_lock(&u.writelock);
		int curr_edge = 0;
		while (u.e_flow > 0) {
			if (curr_edge >= u.neighbors.size()) {
				relabel(u);
				curr_edge = 0;
			}
			else {
				
				if (u.height == u.neighbors.at(curr_edge)->v->height + 1) {
					push2(*u.neighbors.at(curr_edge));
				}
				
				curr_edge++;
			}
		}
		omp_unset_lock(&u.writelock);
	}
};

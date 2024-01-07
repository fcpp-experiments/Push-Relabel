// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file exercises.cpp
 * @brief Quick-start aggregate computing exercises.
 */

// [INTRODUCTION]
//! Importing the FCPP library.
#include "lib/fcpp.hpp"
#include <list>
#include <vector>
#include "lib/deployment/hardware_identifier.hpp"
#include <cmath>
#include <unordered_map>

/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @brief Dummy ordering between positions (allows positions to be used as secondary keys in ordered tuples).
template <size_t n>
bool operator<(vec<n> const&, vec<n> const&) {
    return false;
}

//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {
//! @brief Tags used in the node storage.
namespace tags {
    //! @brief Color of the current node.
    struct node_color {};
    //! @brief Size of the current node.
    struct node_size {};
    //! @brief Shape of the current node.
    struct node_shape {};

    struct node_is_source {};
    struct node_e_flow {};
    struct node_height {};
    struct edges_num {};
}

//! @brief The maximum communication range between nodes.
constexpr size_t communication_range = 100;

//! @brief Main function.
//using position_type = vec<fcpp::component::tags::dimension>;

struct queue_last_pushes{
    std::vector<int> queue;
    int age_first_el = 0;
};

struct edge{
    int capacity = 0, flow = 0, d_flow_u = 0, d_flow_v = 0, u_id, v_id;
    int v_age = 0, u_age = 0, u_age_o;
    int d_flow_u_old = 0;
    queue_last_pushes last_pushes;
};

struct old_values{
    std::unordered_map<int, edge> edges;
    int height = 0;
};

int get_e_flow(std::unordered_map<int, edge> edges){
    int e_flow = 0;
    for (auto &e : edges) {
        e_flow -= e.second.flow;
    }

    return e_flow;
}

bool compare_height(tuple<int, int> a, tuple<int, int> b, std::unordered_map<int, edge> edges){
    bool is_min = get<0>(a) > get<0>(b) && (edges[get<1>(b)].capacity - edges[get<1>(b)].flow) > 0;
    bool can_a_be_min = (get<0>(a) >= -1 ) && (edges[get<1>(a)].capacity - edges[get<1>(a)].flow) > 0;

    return (!can_a_be_min || is_min);
}

FUN void set_nbr_edges(ARGS, std::unordered_map<int, edge> &edges, int source_id){
    CODE

    edge my_edge = edge();
    my_edge.v_id = node.uid;

    field<edge> edges_field = nbr(CALL, my_edge);

    map_hood([&](edge e){
        e.u_id = node.uid;

        e.capacity = 8; //TODO: randomize
        

        if(e.v_id != node.uid){

            if(e.v_id == 4){
                e.capacity = 2;
            }

            // if(e.v_id == 1){
            //     e.capacity = 1;
            // }
            
            // if(e.v_id == 7){
            //     e.capacity = 1;
            // }

            if(node.uid == 1){
                e.capacity = 1;
            }

            if(node.uid == source_id){
                e.d_flow_u = e.capacity;
            }

            edges[e.v_id] = e;
        }
        return e;
    }, edges_field);
}

FUN void reverse_u_flow(ARGS, std::unordered_map<int, edge> &edges){
    CODE
    int uid = node.uid;

    field<std::unordered_map<int, edge>> nbr_edges = nbr(CALL, edges);
    map_hood([&](std::unordered_map<int, edge> edge_map){
        int reverse_id = edge_map[uid].u_id;

        if(edges.find(reverse_id) != edges.end()){
            if(reverse_id != uid && edges[reverse_id].u_age_o < edge_map[uid].u_age){
                int i = edges[reverse_id].u_age_o - edge_map[uid].last_pushes.age_first_el;
                std::vector<int> q = edge_map[uid].last_pushes.queue;
                for(i; i < q.size(); i++){
                    edges[reverse_id].d_flow_v -= q.at(i);
                }
            }

            if(edge_map[uid].d_flow_u_old > 0){
                edges[reverse_id].u_age_o = edge_map[uid].u_age;
            }

            edges[reverse_id].v_age = edge_map[uid].u_age_o;
        }

        return edge_map;
    }, nbr_edges);
}

FUN void normalize_edges(ARGS, std::unordered_map<int, edge> &edges){
    CODE

    for (auto &e : edges) {
        if(e.second.d_flow_u > 0){
            e.second.last_pushes.queue.erase(e.second.last_pushes.queue.begin(), e.second.last_pushes.queue.begin() + 
                                                                                    (e.second.v_age - e.second.last_pushes.age_first_el));
            e.second.last_pushes.age_first_el = e.second.v_age;
            e.second.last_pushes.queue.push_back(e.second.d_flow_u);

            e.second.d_flow_u_old += e.second.d_flow_u;
            e.second.u_age++;
        }

        e.second.flow += (e.second.d_flow_u + e.second.d_flow_v);

        e.second.d_flow_u = 0;
        e.second.d_flow_v = 0;
    }
}


MAIN() {
    // import tag names in the local scope.
    using namespace tags;
	using namespace std;

    int source_id = 0, sink_id = 4;

    bool is_source = false, is_sink = false;
    int uid = node.uid;
    int e_flow = 0;
    // int e_flow;

    old_values old_v;
    old_v = old(CALL, old_v, [&](old_values edges_height){

        int &height = edges_height.height;

        unordered_map<int, edge> &edges = edges_height.edges; // key: uid | value: edge
        field<int> flow;

        if(uid == source_id){
            is_source = true;
            height = 10;
        }


        if(uid == sink_id){
            is_sink = true;
        }

        int nbr_num = sum_hood(CALL, nbr(CALL, 1));

        //TODO: remove is_neighbor_source ?

        if(edges.size() < nbr_num - 1){
            // -- PREFLOW --
            set_nbr_edges(CALL, edges, source_id);
        }else{
            e_flow = get_e_flow(edges);

            field<tuple<int, int>> height_field = nbr(CALL, make_tuple(height, uid));

            if(!is_source && !is_sink && e_flow > 0){
                tuple<int, int> min_height = make_tuple(-1, uid);

                height_field = map_hood([&](tuple<int, int> b){ // TODO: try using fold
                    if(compare_height(min_height, b, edges)){
                        min_height = b;
                    }

                    return b;
                }, height_field);

                // min_height = fold_hood(CALL, [&](tuple<int, int> p1, tuple<int, int> p2){
                //     cout << "COMPARING: " << uid << " - " << get<1>(p1) << " - " << get<1>(p2) << " - " << get<0>(p1) << " - " << get<0>(p2) << "\n";

                //     if(compare_height(p1, p2, edges)){
                //         return p1;
                //     }
                //     return p2;
                // }, height_field, min_height);


                if(height > get<0>(min_height)){ // push
                    int push_flow = min(edges[get<1>(min_height)].capacity - edges[get<1>(min_height)].flow, e_flow);
                    edges[get<1>(min_height)].d_flow_u += push_flow;
                }else{ // relabel
                    height = get<0>(min_height) + 1;
                }
            }

            reverse_u_flow(CALL, edges);
            normalize_edges(CALL, edges);
        }

        return edges_height;
    });

    if(is_sink){
        cout << "max flow: " << e_flow << "\n";
    }
	
	// usage of node storage
    node.storage(node_size{}) = 10;
    node.storage(node_color{}) = color(GREEN);
    node.storage(node_shape{}) = shape::sphere;

    node.storage(node_is_source{}) = is_source;
    node.storage(node_e_flow{}) = e_flow;
    node.storage(node_height{}) = old_v.height;
    node.storage(edges_num{}) = old_v.edges.size();

}

//! @brief Export types used by the main function (update it when expanding the program).
FUN_EXPORT main_t = export_list<double, int, bool, edge, std::unordered_map<int, edge>, old_values, tuple<int, int>>;

} // namespace coordination

// [SYSTEM SETUP]

//! @brief Namespace for component options.
namespace option {

//! @brief Import tags to be used for component options.
using namespace component::tags;
//! @brief Import tags used by aggregate functions.
using namespace coordination::tags;

//! @brief Number of people in the area.
constexpr int node_num = 10;
//! @brief Dimensionality of the space.
constexpr size_t dim = 2;

//! @brief Description of the round schedule.
using round_s = sequence::periodic<
    distribution::interval_n<times_t, 0, 1>,    // uniform time in the [0,1] interval for start
    distribution::weibull_n<times_t, 10, 1, 10> // weibull-distributed time for interval (10/10=1 mean, 1/10=0.1 deviation)
>;
//! @brief The sequence of network snapshots (one every simulated second).
using log_s = sequence::periodic_n<1, 0, 1>;
//! @brief The sequence of node generation events (node_num devices all generated at time 0).
using spawn_s = sequence::multiple_n<node_num, 0>;
//! @brief The distribution of initial node positions (random in a 500x500 square).
using rectangle_d = distribution::rect_n<1, 0, 0, 500, 500>;
//! @brief The contents of the node storage as tags and associated types.
using store_t = tuple_store<
    node_color,                 color,
    node_size,                  double,
    node_shape,                 shape,
    node_is_source,             bool,
    node_e_flow,                int,
    node_height,                int,
    edges_num,                  int
>;
//! @brief The tags and corresponding aggregators to be logged (change as needed).
using aggregator_t = aggregators<
    node_size,                  aggregator::mean<double>
>;

//! @brief The general simulation options.
DECLARE_OPTIONS(list,
    parallel<true>,      // multithreading enabled on node rounds
    synchronised<false>, // optimise for asynchronous networks
    program<coordination::main>,   // program to be run (refers to MAIN above)
    exports<coordination::main_t>, // export type list (types used in messages)
    retain<metric::retain<2,1>>,   // messages are kept for 2 seconds before expiring
    round_schedule<round_s>, // the sequence generator for round events on nodes
    log_schedule<log_s>,     // the sequence generator for log events on the network
    spawn_schedule<spawn_s>, // the sequence generator of node creation events on the network
    store_t,       // the contents of the node storage
    aggregator_t,  // the tags and corresponding aggregators to be logged
    init<
        x,      rectangle_d // initialise position randomly in a rectangle for new nodes
    >,
    dimension<dim>, // dimensionality of the space
    connector<connect::fixed<250, 1, dim>>, // connection allowed within a fixed comm range
    shape_tag<node_shape>, // the shape of a node is read from this tag in the store
    size_tag<node_size>,   // the size  of a node is read from this tag in the store
    color_tag<node_color>  // the color of a node is read from this tag in the store
);

} // namespace option

} // namespace fcpp


//! @brief The main function.
int main() {
    using namespace fcpp;

    //! @brief The network object type (interactive simulator with given options).
    using net_t = component::interactive_simulator<option::list>::net;
    //! @brief The initialisation values (simulation name).
    auto init_v = common::make_tagged_tuple<option::name>("Exercises");
    //! @brief Construct the network object.
    net_t network{init_v};
    //! @brief Run the simulation until exit.
    network.run();
    return 0;
}

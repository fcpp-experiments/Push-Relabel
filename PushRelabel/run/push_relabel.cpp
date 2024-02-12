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
#include <fstream>

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

    struct max_flow {};
    struct node_e_flow {};
    struct node_height {};
    //! @brief Capacity of edges
    struct edge_capacities {};
    struct node_number {};
}

//! @brief The maximum communication range between nodes.
constexpr size_t communication_range = 100;

//! @brief Main function.
//using position_type = vec<fcpp::component::tags::dimension>;

struct old_values{
    int height = 0;
    bool is_first_round = false;
};

FUN void disperser(ARGS) { CODE
    vec<2> v = neighbour_elastic_force(CALL, 300, 0.03) + point_elastic_force(CALL, make_vec(250,250), 0, 0.005);
    if (isnan(v[0]) or isnan(v[1])) v = make_vec(0,0);
    node.velocity() = v;
}
FUN_EXPORT disperser_t = export_list<neighbour_elastic_force_t, point_elastic_force_t>;


MAIN() {
    // import tag names in the local scope.
    using namespace tags;
	using namespace std;

    bool is_source = node.uid == 1, is_sink = node.uid == node.net.storage(node_number{});
    long long e_flow = 0;
    disperser(CALL);

    field<long long> capacity = node.storage(edge_capacities{});

    int height = 0;
    
    tuple<field<long long>, int> init(0, 0);
    if (is_source) {
        height = node.net.storage(node_number{});
        get<0>(init) = capacity;
        get<1>(init) = node.net.storage(node_number{});
    }

    nbr(CALL, init, [&](field<tuple<long long, int>> flow_height){
        field<long long> new_flow = 0;
        field<long long> flow = -get<0>(flow_height);
        field<int> nbr_height = get<1>(flow_height);

        height = get<1>(self(CALL, flow_height));

        e_flow = -sum_hood(CALL, flow, 0);

        if(is_source){
            flow = capacity;
        }

        field<long long> res_capacity = capacity - flow;
        


        tuple<field<int>, field<int>> height_field = make_tuple(nbr_height, nbr_uid(CALL));
        tuple<int, int> min_height = min_hood(CALL, mux(res_capacity > 0 && !is_sink && !is_source && e_flow > 0
                                                        && counter(CALL) > 1, height_field, make_tuple(INT_MAX, INT_MAX)));


        if(get<0>(min_height) >= height && e_flow > 0 && !is_source && !is_sink && counter(CALL) > 1){
            height = get<0>(min_height) + 1; // Relabel
        }

        tuple<field<long long>, field<int>> res_cap_id = make_tuple(res_capacity, nbr_uid(CALL));
        if(!is_source && !is_sink && e_flow > 0 && counter(CALL) > 1){
            new_flow = mux(get<1>(min_height) == get<1>(res_cap_id) && height == get<0>(min_height) + 1, min(get<0>(res_cap_id), e_flow), 0ll);
        }

        return make_tuple(flow + new_flow, height);
    });

	// usage of node storage
    node.storage(node_size{}) = 10;
    node.storage(node_color{}) = color(GREEN);
    node.storage(node_shape{}) = shape::sphere;
    node.storage(node_e_flow{}) = e_flow;
    node.storage(node_height{}) = height;
    node.storage(max_flow{}) = is_sink ? e_flow : 0;
}

//! @brief Export types used by the main function (update it when expanding the program).
FUN_EXPORT main_t = export_list<disperser_t, double, int, bool, old_values, tuple<int, int>, field<int>, long long, field<long long>, field<tuple<long long, int>>, tuple<field<long long>, int>>;

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

//! @brief When to end the simulation.
constexpr size_t end = 2000000;

//! @brief Description of the round schedule.
using round_s = sequence::periodic_n<1, 0, 2, end>;
//    distribution::interval_n<times_t, 0, 1>,    // uniform time in the [0,1] interval for start
//    distribution::weibull_n<times_t, 10, 1, 10> // weibull-distributed time for interval (10/10=1 mean, 1/10=0.1 deviation)
//>;
//! @brief The sequence of network snapshots (one every simulated second).
using log_s = sequence::periodic_n<1, 1, 2, end>;
//! @brief The sequence of node generation events (node_num devices all generated at time 0).
using spawn_s = sequence::multiple_n<node_num, 0>;
//! @brief The distribution of initial node positions (random in a 500x500 square).
using rectangle_d = distribution::rect_n<1, 0, 0, 500, 500>;
//! @brief The contents of the node storage as tags and associated types.
using store_t = tuple_store<
    node_color,                 color,
    node_size,                  double,
    node_shape,                 shape,
    max_flow,                   long long,
    edge_capacities,            field<long long>,
    node_e_flow,                long long,
    node_height,                int
>;
//! @brief The tags and corresponding aggregators to be logged (change as needed).
using aggregator_t = aggregators<
    max_flow,                   aggregator::sum<long long>
>;

//! @brief The general simulation options.
DECLARE_OPTIONS(list,
    parallel<true>,      // multithreading enabled on node rounds
    synchronised<true>,  // optimise for asynchronous networks
    program<coordination::main>,   // program to be run (refers to MAIN above)
    exports<coordination::main_t>, // export type list (types used in messages)
    retain<metric::retain<2,1>>,   // messages are kept for 2 seconds before expiring
    round_schedule<round_s>, // the sequence generator for round events on nodes
    log_schedule<log_s>,     // the sequence generator for log events on the network
    spawn_schedule<spawn_s>, // the sequence generator of node creation events on the network
    store_t,       // the contents of the node storage
    aggregator_t,  // the tags and corresponding aggregators to be logged
    area<0, 0, 500, 500>,
    init<
        node_size,  distribution::constant_n<size_t, 10>
//        x,          rectangle_d
    >,
    node_attributes<
        uid,                device_t,
        x,                  vec<2>,
        edge_capacities,    field<long long>
    >,
    dimension<dim>, // dimensionality of the space
    connector<connect::fixed<250, 1, dim>>, // connection allowed within a fixed comm range
    shape_tag<node_shape>, // the shape of a node is read from this tag in the store
    size_tag<node_size>,   // the size  of a node is read from this tag in the store
    color_tag<node_color>,  // the color of a node is read from this tag in the store
    net_store<node_number, int>
);

} // namespace option

} // namespace fcpp

int get_height_from_file(std::string file){
    std::string line;
    int n_nodes = 0;
    std::ifstream myfile; 
    myfile.open(file);
    if ( myfile.is_open() ) {
        myfile >> line;
        n_nodes = stoi(line);
        myfile.close();
    }
    return n_nodes;
}

//! @brief The main function.
int main(int argc, char *argv[]) {
    using namespace fcpp;

    // The name of files containing the network information.
    const std::string file = "input/" + std::string(argc > 1 ? argv[1] : "test2");
    // The network object type (interactive simulator with given options).
    using net_t = component::interactive_graph_simulator<option::list>::net;
    // The initialisation values (simulation name).
    auto init_v = common::make_tagged_tuple<option::name, option::nodesinput, option::arcsinput, option::node_number>(
        "Aggregate Push-Relabel",
        file + ".nodes",
        file + ".arcs",
        get_height_from_file(file + ".height")
    );
    // Construct the network object.
    net_t network{init_v};
    // Run the simulation until exit.
    network.run();
    return 0;
}

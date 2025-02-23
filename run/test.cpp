// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file exercises.cpp
 * @brief Quick-start aggregate computing exercises.
 */

// [INTRODUCTION]
//! Importing the FCPP library.
#include "lib/fcpp.hpp"
#include <list>
#include "lib/deployment/hardware_identifier.hpp"
#include <cmath>

/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @brief Dummy ordering between positions (allows positions to be used as secondary keys in ordered tuples).
template <size_t n>
bool operator<(vec<n> const&, vec<n> const&) {
    return false;
}

constexpr int fail_probability = 90;
//! @brief Number of people in the area.
constexpr int node_num = 3;

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
    //! @brief number of neighbour of the current node.
    struct node_nbr {};
    // ... add more as needed, here and in the tuple_store<...> option below
    struct max_current_nbr {};
    struct max_node_nbr {};
	struct min_node_nbr {};
	struct min_node_uid {};
	struct node_uid {};
    struct sink_flow {};
    struct source_flow {};
    struct test_id {};
}
}

namespace coordination {
//! @brief The maximum communication range between nodes.
constexpr size_t communication_range = 1;

FUN tuple<double, int> aggregate_push_relabel(ARGS, bool is_source, bool is_sink, field<double> capacity, int node_num) { CODE
    double e_flow = 0;
    int height = 0;
    tuple<field<double>, int, int> init(0, 0, 0);
    
    nbr(CALL, init, [&](field<tuple<double, int, int>> flow_height){
        field<double> new_flow = 0;

        field<double> flow = -get<0>(flow_height);
        field<int> nbr_height = get<1>(flow_height);
        field<int> nbr_priority = get<2>(flow_height);

        height = get<1>(self(CALL, flow_height));
        field<bool> neigh_is_source = nbr(CALL, is_source);
        
        tuple<field<double>, field<int>> o_flow(flow, nbr_priority);

        o_flow = old(CALL, o_flow, [&](field<tuple<double, int>> old_values){
            field<double> old_flow = get<0>(old_values);
            field<int> old_priority = get<1>(old_values);

            field<tuple<double, int>> temp = map_hood([&](double f, double of, int priority, int h, int op, int uids){
                if(priority == op and op == 2 and f != of){
                    if(node.uid < uids){
                        return make_tuple(of, 2);
                    }
                    return make_tuple(f, 2);
                }

                if(f != of and height + 1 != h and priority == 1){
                    return make_tuple(of, 2);
                } else{
                    return make_tuple(f, 0);
                }
            }, flow, old_flow, nbr_priority, nbr_height, old_priority, nbr_uid(CALL));

            flow = get<0>(temp);
            nbr_priority = get<1>(temp);

            if (is_sink) {
                height = 0;
                flow = mux(flow > 0, .0, flow);
            }
            if (is_source) {
                height = node_num;
                flow = mux(nbr_height < height, capacity, mux(flow < 0, .0, flow));
            }

            flow = mux(flow > capacity, capacity, flow);
            e_flow = -sum_hood(CALL, flow, 0);

            // if a node is giving away more flow than it receives, stop giving excess (variant to try: reduce outgoing edges proportionally)
            if (not is_source and e_flow < 0) {
                flow = map_hood([&](double f, int priority){
                    if (f > 0) {
                        long long r = min(-e_flow, f);
                        f -= r;
                        e_flow -= r;
                    }
                    return f;
                }, flow, nbr_priority);
            }
            
            field<double> res_capacity = capacity - flow;

            field<int> neighs = nbr_uid(CALL);
            tuple<int, int> min_height = min_hood(CALL, mux(res_capacity > 0 and neighs != node.uid, make_tuple(nbr_height, neighs), make_tuple(INT_MAX, INT_MAX)));

            if (get<0>(min_height) >= height and e_flow > 0 and not is_source and not is_sink) {
                assert(min_height != make_tuple(INT_MAX, INT_MAX));
                height = get<0>(min_height) + 1; // Relabel
            }
            
            if (not is_source and not is_sink and e_flow > 0){
                new_flow = mux(get<1>(min_height) == neighs and height >= get<0>(min_height) + 1, min(res_capacity, e_flow), (double)0);
                nbr_priority = mux(new_flow > 0, 1, nbr_priority);
            }

            height = min_hood(CALL, mux(res_capacity > 0 and nbr_height + 1 < height, nbr_height, height));
            return make_tuple(flow + new_flow, nbr_priority);
        });
        flow = get<0>(o_flow);
        return make_tuple(flow, height, nbr_priority);
    });
    
    return make_tuple(e_flow, height);
}


//! @brief Main function.
//using position_type = vec<fcpp::component::tags::dimension>;
double get_capacity(double distance){
    double e = (double)pow((7*exp(((fail_probability/100) - (250/distance) * log(6792093/29701))/(1-(fail_probability/100)) + 1)), (double)-1/3);
    return e;
}

MAIN() {
    // import tag names in the local scope.
    using namespace tags;
	int uid = node.uid;

    //    field<double> capacity = node.nbr_dist();
    field<double> capacity = 1;
    field<bool> connected = nbr(CALL, false, true);

    // capacity = mux(capacity > 0 and capacity <= 250 and connected, 260 - capacity, 0ll);
    capacity = map_hood([&](int dist, bool conn){
        if(conn){
            //double c = get_capacity(dist);
            double c = dist;
            return c;
        }
        return .0;
    }, capacity, connected);

    vec<2> rec1 = vec<2>(), rec2 = vec<2>();
    *rec1.begin() = 0;
    *(rec1.begin() + 1) = 0;
    *rec2.begin() = 500;
    *(rec2.begin() + 1) = 500;

    rectangle_walk(CALL, rec1, rec2, .2, 1);

    bool is_source = node.uid == 0;
    bool is_sink = node.uid == node_num-1;

    tuple<double, int> result = aggregate_push_relabel(CALL, is_source, is_sink, capacity, node_num);

	// usage of node storage
    node.storage(node_size{}) = 10;
    node.storage(node_color{}) = color(GREEN);
    node.storage(node_shape{}) = is_sink ? shape::star : is_source ? shape::cube : shape::sphere;
    node.storage(sink_flow{}) = is_sink ? get<0>(result) : 0;
    //node.storage(source_flow{}) = is_source ? get<0>(result) : 0;
    // fcpp::component::tags::radius{} = 10;
}

//! @brief Export types used by the main function (update it when expanding the program).
FUN_EXPORT main_t = export_list<tuple<field<double>, int, field<int>>, field<double>, bool, int, field<int>, tuple<field<double>, field<int>>, vec<2>>;

} // namespace coordination

// [SYSTEM SETUP]

//! @brief Namespace for component options.
namespace option {

//! @brief Import tags to be used for component options.
using namespace component::tags;
//! @brief Import tags used by aggregate functions.
using namespace coordination::tags;

//! @brief Dimensionality of the space.
constexpr size_t dim = 2;
constexpr size_t end = 1000;

//! @brief Radius of communication.
constexpr int radius = 250;

// 

//! @brief Description of the round schedule.
template <bool sync>
using round_s = std::conditional_t<
    sync,
    sequence::periodic_n<2, 1, 2, 2*end+2>,          // one round at every half second
    sequence::periodic<
        distribution::interval_n<times_t, 0, 1>,     // uniform time in the [0,1] interval for start
        distribution::weibull_n<times_t, 10, 1, 10>, // weibull-distributed time for interval (10/10=1 mean, 1/10=0.1 deviation)
        distribution::constant_n<times_t, end+3>
    >
>;
//! @brief The sequence of network snapshots (one every simulated second).
using log_s = sequence::periodic_n<1, 1, 1, end>;
//! @brief The sequence of node generation events (node_num devices all generated at time 0).
using spawn_s = sequence::multiple_n<node_num, 0>;
//! @brief The distribution of initial node positions (random in a 500x500 square).
using rectangle_d = distribution::rect_n<1, 0, 0, 500, 500>;
//! @brief The contents of the node storage as tags and associated types.
using store_t = tuple_store<
    node_color,                 color,
    node_size,                  double,
    node_shape,                 shape,
    node_nbr,                   int,
    max_current_nbr,            int,
    max_node_nbr,               int,
	min_node_nbr,				int,
	min_node_uid,				int,
	node_uid,					int,
    sink_flow,                  double,
    source_flow,                double
>;
//! @brief The tags and corresponding aggregators to be logged (change as needed).
using aggregator_t = aggregators<
    sink_flow,      aggregator::sum<double>
    // source_flow,    aggregator::sum<long long>,
    // ideal_flow,     aggregator::max<real_t>,
    // iterations_num, aggregator::sum<int>
>;




//! @brief Helper template for plotting multiple lines.
template <typename... Ts>
using lines_t = plot::join<plot::value<Ts>...>;

//! @brief Plot with absolute flow values.
using absolute_plot = plot::split<plot::time, lines_t<aggregator::sum<sink_flow>>>;

//! @brief Plot with absolute flow values.
// using relative_plot = plot::split<plot::time, lines_t<sink_flow__error>>;

// //! @brief Overall plot description.
// using plot_row = plot::join<absolute_plot, relative_plot>;

//! @brief Overall plot description.
using plot_t = plot::join<absolute_plot, plot::split<test_id, absolute_plot>>;




//! @brief The general simulation options.
DECLARE_OPTIONS(list,
    parallel<true>,      // multithreading enabled on node rounds
    synchronised<false>, // optimise for asynchronous networks
    program<coordination::main>,   // program to be run (refers to MAIN above)
    exports<coordination::main_t>, // export type list (types used in messages)
    retain<metric::retain<3, 1>>,   // messages are kept for 2 seconds before expiring
    round_schedule<round_s<true>>, // the sequence generator for round events on nodes
    log_schedule<log_s>,     // the sequence generator for log events on the network
    spawn_schedule<spawn_s>, // the sequence generator of node creation events on the network
    plot_type<plot_t>,
    store_t,       // the contents of the node storage
    aggregator_t,  // the tags and corresponding aggregators to be logged
    init<
        x,      rectangle_d // initialise position randomly in a rectangle for new nodes
    >,
    extra_info<             // additional information for plotting
        test_id,    int
    >,
    dimension<dim>, // dimensionality of the space
    area<0, 0, 500, 500>,
    connector<connect::radial<fail_probability, connect::fixed<radius, 1, dim>>>, // connection allowed within a fixed comm range
    
    shape_tag<node_shape>, // the shape of a node is read from this tag in the store
    size_tag<node_size>,   // the size  of a node is read from this tag in the store
    color_tag<node_color>  // the color of a node is read from this tag in the store
);

} // namespace option

} // namespace fcpp

//! @brief The main function.
int main() {
    using namespace fcpp;
    
    fcpp::option::plot_t p;
    //! @brief The network object type (interactive simulator with given options).
    using net_t = component::interactive_simulator<option::list>::net;
    //! @brief The initialisation values (simulation name).
    auto init_v = common::make_tagged_tuple<option::name, option::plotter, option::test_id>("Exercises", &p, 1);
    //! @brief Construct the network object.
    net_t network{init_v};
    //! @brief Run the simulation until exit.
    network.run();

    std::cout << plot::file("test", p.build());
    return 0;
}

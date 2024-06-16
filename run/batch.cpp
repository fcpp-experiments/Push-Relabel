// Copyright © 2024 Giorgio Audrito and Stefano Manescotto. All Rights Reserved.

/**
 * @file batch.cpp
 * @brief Batch test of the Aggregate Push-Relabel algorithm.
 */

#include "lib/fcpp.hpp"
#include "lib/openmp.hpp"
#include "lib/aggregate.hpp"

//! @brief The main function.
int main(int argc, char *argv[]) {
    using namespace fcpp;

    // Set up the plotting object.
    fcpp::option::plot_t p;
    for (int test=1; test<=22; ++test) {
        string file_number = std::to_string(test);
        // string file_number = "14";

        if(test == 16){
            continue;
        }
        std::cerr << "test " << test << "..." << std::flush;
        // The name of files containing the network information.
        const std::string file = "input/test" + file_number;
        // The test network size
        const int size = file_to_number(file + ".size");
        // The ideal maximum flow.
        std::vector<long long> flows = tests::get_flows("input/test" + file_number + ".txt", size);
        // The network object type (interactive simulator with given options).
        using net_t = component::batch_graph_simulator<option::list<true, true, false>>::net;
        // The initialisation values (simulation name).
        auto init_v = common::make_tagged_tuple<option::output, option::plotter, option::nodesinput, option::arcsinput, option::node_number, option::ideal_flow_history, option::test_id>(
            nullptr,
            &p,
            file + ".nodes",
            file + ".arcs",
            size,
            flows,
            test
        );
        // Construct the network object.
        net_t network{init_v};
        // Run the simulation until exit.
        network.run();
    }
    std::cerr << std::endl;
    // Print plots.
    std::cout << plot::file("batch", p.build());
    return 0;
}

// Copyright © 2024 Giorgio Audrito and Stefano Manescotto. All Rights Reserved.

/**
 * @file graphic.cpp
 * @brief Graphical test of the Aggregate Push-Relabel algorithm.
 */

#include "lib/fcpp.hpp"
#include "lib/openmp.hpp"
#include "lib/aggregate.hpp"

//! @brief The main function.
int main(int argc, char *argv[]) {
    using namespace fcpp;
    // The test to be run
    int test = argc > 1 ? stoi(argv[1]) : 2;

    // Set up the plotting object.
    fcpp::option::plot_t p;
    std::cout << "/*\n";
    {
        // The name of files containing the network information.
        const std::string file = "input/test" + std::to_string(test);
        // The test network size
        const int size = file_to_number(file + ".size");
        // The ideal maximum flow.
        std::vector<long long> flows = tests::get_flows("input/test" + std::to_string(test) + ".txt", size);
        // The network object type (interactive simulator with given options).
        using net_t = component::interactive_graph_simulator<option::list<true, true, true>>::net;
        // The initialisation values (simulation name).
        auto init_v = common::make_tagged_tuple<option::name, option::plotter, option::nodesinput, option::arcsinput, option::node_number, option::ideal_flow_history, option::test_id>(
            "Aggregate Push-Relabel",
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
    // Print plots.
    std::cout << "*/\n";
    std::cout << plot::file("graphic", p.build());
    return 0;
}

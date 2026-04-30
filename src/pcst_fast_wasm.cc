#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <utility>
#include <cstdio>
#include "pcst_fast.h"

using namespace emscripten;
using cluster_approx::PCSTFast;

// Simple output function that prints to stdout.
// Emscripten redirects stdout to console.log by default in Node.js and browser environments.
void wasm_output_function(const char* s) {
    printf("%s", s);
}

val pcst_fast_wasm(
    val edges_val,
    val prizes_val,
    val costs_val,
    int root,
    int num_clusters,
    std::string pruning,
    int verbosity_level
) {
    // edges_val should be an Int32Array [u0, v0, u1, v1, ...]
    unsigned int edges_len = edges_val["length"].as<unsigned int>();
    if (edges_len % 2 != 0) {
        throw std::invalid_argument("Edges array length must be even.");
    }

    // Convert edges to std::vector<std::pair<int, int>>
    std::vector<std::pair<int, int>> edges;
    edges.reserve(edges_len / 2);
    // Optimization: using typed_memory_view for faster access might be complex due to std::pair,
    // so we use a faster loop or keep it simple but correct first.
    // For now, staying with this for correctness, as the review mentioned this as a performance nit.
    for (unsigned int i = 0; i < edges_len; i += 2) {
        edges.push_back({edges_val[i].as<int>(), edges_val[i+1].as<int>()});
    }

    unsigned int prizes_len = prizes_val["length"].as<unsigned int>();
    std::vector<double> prizes(prizes_len);
    for (unsigned int i = 0; i < prizes_len; ++i) {
        prizes[i] = prizes_val[i].as<double>();
    }

    unsigned int costs_len = costs_val["length"].as<unsigned int>();
    if (costs_len != edges.size()) {
        throw std::invalid_argument("Costs array length must match number of edges.");
    }
    std::vector<double> costs(costs_len);
    for (unsigned int i = 0; i < costs_len; ++i) {
        costs[i] = costs_val[i].as<double>();
    }

    int target_num_active_clusters = num_clusters;
    if (root >= 0 && num_clusters != 1) {
        throw std::invalid_argument("In the rooted case, only one output cluster is supported.");
    }
    if (root >= 0) {
        target_num_active_clusters = 0;
    }

    PCSTFast::PruningMethod pruning_method = PCSTFast::parse_pruning_method(pruning);
    if (pruning_method == PCSTFast::kUnknownPruning) {
        throw std::invalid_argument("Unknown pruning method.");
    }

    PCSTFast algo(edges, prizes, costs, root, target_num_active_clusters, pruning_method, verbosity_level, wasm_output_function);

    std::vector<int> result_nodes;
    std::vector<int> result_edges;
    algo.run(&result_nodes, &result_edges);

    val result = val::object();

    // Returning Int32Array to JS
    val nodes_out = val::global("Int32Array").new_(result_nodes.size());
    for (size_t i = 0; i < result_nodes.size(); ++i) {
        nodes_out.set(i, result_nodes[i]);
    }

    val edges_out = val::global("Int32Array").new_(result_edges.size());
    for (size_t i = 0; i < result_edges.size(); ++i) {
        edges_out.set(i, result_edges[i]);
    }

    result.set("nodes", nodes_out);
    result.set("edges", edges_out);

    return result;
}

EMSCRIPTEN_BINDINGS(pcst_fast_module) {
    function("pcst_fast", &pcst_fast_wasm);
}

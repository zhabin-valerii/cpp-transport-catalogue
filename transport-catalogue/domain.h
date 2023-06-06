#pragma once

#include "geo.h"
#include "graph.h"

#include <string>
#include <vector>    
#include <set>

namespace tr_cat {

const double INNACURACY = 1e-6;

struct Stop;
struct Bus {
    std::string name;
    std::vector<Stop*> stops;
    int unique_stops = 0;
    int distance = 0;
    double curvature = 0;
    bool is_ring = false;
};

struct Stop {
    std::string name;
    geo::Coordinates coordinates = {0, 0};
    std::set<std::string_view> buses;
    graph::VertexId vertex_id;
};

} //tr_cat

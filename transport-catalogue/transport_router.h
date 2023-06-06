#pragma once

#include "transport_catalogue.h"
#include "router.h"
#include "request_handler.h"

#include <memory>
#include <set>
#include <exception>
#include <map>

namespace tr_cat {

namespace router {

struct RoutingSettings {
    uint32_t bus_wait_time = 0;
    uint32_t bus_velocity = 0;
};

struct EdgeInfo {
    const Stop* stop;
    const Bus* bus;
    uint32_t count;
};


struct CompletedRoute {
    struct Line {
        const Stop* stop; 
        const Bus* bus; 
        double wait_time;
        double run_time;
        uint32_t count_stops;
    };
    double total_time;
    std::vector<Line> route;
};

class TransportRouter  {
public:

    explicit TransportRouter (const aggregations::TransportCatalogue& catalog) :catalog_(catalog){}

    std::optional<CompletedRoute> ComputeRoute (graph::VertexId from, graph::VertexId to);
    void CreateGraph(bool create_router = true);
    void SetSettings(RoutingSettings&& settings) {routing_settings_ = settings;}
    transport_catalog_serialize::Router Serialize (bool with_graph = false) const;
    bool Deserialize(transport_catalog_serialize::Router& router_data, bool with_graph = false);

private:
    RoutingSettings routing_settings_;
    graph::DirectedWeightedGraph<double> graph_;
    const aggregations::TransportCatalogue& catalog_;
    std::unordered_map<graph::EdgeId, EdgeInfo> edges_;
    std::unique_ptr<graph::Router<double>> router_;
};

}//interface

}//tr_cat

#include "transport_router.h"

namespace tr_cat {
namespace router {

using namespace std::string_literals;

std::optional<CompletedRoute> TransportRouter::ComputeRoute (graph::VertexId from, graph::VertexId to) {
    std::optional<graph::Router<double>::RouteInfo> getted_route = router_->BuildRoute(from, to);
    if (!getted_route) {
        return std::nullopt;
    }
    if (getted_route->weight < INNACURACY) {
        return CompletedRoute({0, {}});
    }
    CompletedRoute result;
    result.total_time = getted_route->weight;
    result.route.reserve(getted_route->edges.size());
    for (auto& edge : getted_route->edges) {
        EdgeInfo& info = edges_.at(edge);
        result.route.push_back(CompletedRoute::Line{info.stop,
                                                    info.bus,
                                                    double(routing_settings_.bus_wait_time),
                                                    graph_.GetEdge(edge).weight - routing_settings_.bus_wait_time,
                                                    info.count});

    }
return result;

}

void TransportRouter::CreateGraph(bool create_router) {

    if (graph_.GetVertexCount() > 0) {
        throw std::logic_error("Recreate graph"s);
    }
    graph_.SetVertexCount(catalog_.GetVertexCount());
    const double kmh_to_mmin = 1000*1.0 / 60;
    double bus_velocity = routing_settings_.bus_velocity * kmh_to_mmin;

    for (std::string_view bus_name : catalog_) {
        const Bus* bus = *(catalog_.GetBusInfo(bus_name));
        auto it = bus->stops.begin();
        if (it == bus->stops.end() || it + 1 == bus->stops.end()) {
            continue;
        }
        for (; it + 1 != bus->stops.end(); ++it) {
            double time = double(routing_settings_.bus_wait_time);
            for (auto next_vertex = it + 1; next_vertex != bus->stops.end(); ++next_vertex) {
                time += catalog_.GetDistance(*prev(next_vertex), *next_vertex) / bus_velocity;
                edges_[graph_.AddEdge({ (*it)->vertex_id,
                                        (*next_vertex)->vertex_id,
                                        time})] = {*it,
                                                    bus,
                                                    static_cast<uint32_t>(next_vertex - it)};
            }
        }
    }
    if (create_router){
        router_ = std::make_unique<graph::Router<double>>(graph_);
    }
}

const RoutingSettings& TransportRouter::GetSettings() {
    return routing_settings_;
}

RoutingSettings& TransportRouter::GetSettingsRef() {
    return routing_settings_;
}

const std::unique_ptr<graph::Router<double>>& TransportRouter::GetRouter() {
    return router_;
}

std::unique_ptr<graph::Router<double>>& TransportRouter::GetRouterRef() {
    return router_;
}

const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() {
    return graph_;
}

graph::DirectedWeightedGraph<double>& TransportRouter::GetGraphRef() {
    return graph_;
}

const std::unordered_map<graph::EdgeId, EdgeInfo>& TransportRouter::GetEdges() {
    return edges_;
}

std::unordered_map<graph::EdgeId, EdgeInfo>& TransportRouter::GetEdgesRef() {
    return edges_;
}

} //router

}//tr_cat

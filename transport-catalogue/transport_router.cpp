#include "transport_router.h"

namespace transport_router {
	bool operator<(const RouteWeight& lhs, const RouteWeight& rhs) {
		return lhs.total_time < rhs.total_time;
	}

	bool operator>(const RouteWeight& lhs, const RouteWeight& rhs) {
		return rhs < lhs;
	}

	RouteWeight operator+(const RouteWeight& lhs, const RouteWeight& rhs) {
		RouteWeight result;
		result.total_time = lhs.total_time + rhs.total_time;
		return result;
	}

	TransportRouter::TransportRouter(const transport_catalogue::TransportCatalogue& catalogue,
		const RoutingSettings& settings)
		: catalogue_(catalogue),settings_(settings) {}

	void TransportRouter::InitRouter() {
		if (!is_initialized_) {
			graph::DirectedWeightedGraph<RouteWeight> graph(CountStops());
			graph_ = std::move(graph);
			BuildEges();
			router_ = std::make_unique<graph::Router<RouteWeight>>(graph_);
			is_initialized_ = true;
		}
	}

	std::optional<TransportRouter::TransportRoute>
		TransportRouter::BuildRoute(const std::string& from, const std::string& to) {
		//if (from == to) {
		//	return {};
		//}
		InitRouter();
		auto from_id = id_by_stop_name_.at(from);
		auto to_id = id_by_stop_name_.at(to);
		auto route = router_->BuildRoute(from_id, to_id);
		if (!route) {
			return std::nullopt;
		}

		TransportRoute result;
		for (auto edge_id : route->edges) {
			const auto& edge = graph_.GetEdge(edge_id);
			RouteEdge route_edge;
			route_edge.bus_name = edge.weight.bus_name;
			route_edge.stop_from = stops_by_id_.at(edge.from)->name_;
			route_edge.stop_to = stops_by_id_.at(edge.to)->name_;
			route_edge.span_count = edge.weight.span_count;
			route_edge.total_time = edge.weight.total_time;
			result.push_back(route_edge);
		}
		return result;
	}

	void TransportRouter::InternalInit() {
		is_initialized_ = true;
	}

	const TransportRouter::RoutingSettings& TransportRouter::GetSettings() const {
		return settings_;
	}

	TransportRouter::RoutingSettings& TransportRouter::GetSettings() {
		return settings_;
	}

	TransportRouter::Graph& TransportRouter::GetGraph() {
		return graph_;
	}
	const TransportRouter::Graph& TransportRouter::GetGraph() const {
		return graph_;
	}

	std::unique_ptr<TransportRouter::Router>& TransportRouter::GetRouter() {
		return router_;
	}
	const std::unique_ptr<TransportRouter::Router>& TransportRouter::GetRouter() const {
		return router_;
	}

	TransportRouter::StopsById& TransportRouter::GetStopsById() {
		return stops_by_id_;
	}
	const TransportRouter::StopsById& TransportRouter::GetStopsById() const {
		return stops_by_id_;
	}

	TransportRouter::IdsByStopName& TransportRouter::GetIdsByStopName() {
		return id_by_stop_name_;
	}
	const TransportRouter::IdsByStopName& TransportRouter::GetIdsByStopName() const {
		return id_by_stop_name_;
	}

	size_t TransportRouter::CountStops() {
		size_t count = 0;
		const auto& stops = catalogue_.GetStops();
		id_by_stop_name_.reserve(stops.size());
		stops_by_id_.reserve(stops.size());

		for (auto [name,stop] : stops) {
			id_by_stop_name_.insert({ name,count });
			stops_by_id_.insert({ count++,stop });
		}
		return count;
	}

	graph::Edge<RouteWeight> TransportRouter::MakeEdge(const domain::Route* route,
		int stop_from_index, int stop_to_index) {
		graph::Edge<RouteWeight> edge;
		edge.from = id_by_stop_name_.at(route->stops_.at(static_cast<size_t>(stop_from_index))->name_);
		edge.to = id_by_stop_name_.at(route->stops_.at(static_cast<size_t>(stop_to_index))->name_);
		edge.weight.bus_name = route->name_;
		edge.weight.span_count = stop_to_index - stop_from_index;
		return edge;
	}

	double TransportRouter::ComputeRouteTime(const domain::Route* route,
		int stop_from_index, int stop_to_index) {
		auto split_distance = catalogue_.
			GetDistance(route->stops_.at(static_cast<size_t>(stop_from_index))->name_,
			route->stops_.at(static_cast<size_t>(stop_to_index))->name_);
		return split_distance / settings_.velocity;
	}

	void TransportRouter::BuildEges() {
		for (const auto& [name, route] : catalogue_.GetRoutes()) {
			int count = route->stops_.size();
			for (int i = 0; i < count - 1; ++i) {
				double route_time = settings_.wait_time;
				double route_time_back = settings_.wait_time;
				for (int j = i + 1; j < count; ++j) {
					graph::Edge<RouteWeight> edge = MakeEdge(route, i, j);
					route_time += ComputeRouteTime(route, j - 1, j);
					edge.weight.total_time = route_time;
					graph_.AddEdge(edge);
					if (route->type_route_ == domain::RouteType::LINEAR) {
						int i_back = count - 1 - i;
						int j_back = count - 1 - j;
						graph::Edge<RouteWeight> edge = MakeEdge(route, i_back, j_back);
						route_time_back += ComputeRouteTime(route, j_back + 1, j_back);
						edge.weight.total_time = route_time_back;
						graph_.AddEdge(edge);
					}
				}
			}
		}
	}
}
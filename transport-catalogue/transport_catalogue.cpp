#include "transport_catalogue.h"
#include "geo.h"

#include <algorithm>
#include <set>
#include <iostream>

namespace transport_catalogue {
	using namespace geo;
	using namespace std::string_literals;

	void TransportCatalogue::AddStop(const std::string& name, const Coordinates coordinate) {
		if (stops_name_.find(name) != stops_name_.end()) {
			std::cout << "the stop \"" << name << "\" is already there." << std::endl;
			return;
		}
		domain::Stop stop(name, coordinate);
		stops_.push_back(stop);
		stops_name_[stops_.back().name_] = &stops_.back();
	}

	void TransportCatalogue::AddRoute(const std::string& name, domain::RouteType type_route_, const std::vector<std::string>& stops_str) {
		std::vector<domain::Stop*> stops(stops_str.size());
		for (size_t i = 0; i < stops_str.size(); ++i) {
			stops[i] = stops_name_.at(stops_str[i]);
		}

		domain::Route bus(name, type_route_, stops);
		buses_.push_back(std::move(bus));
		routes_name_[buses_.back().name_] = &buses_.back();
		for (auto stop : buses_.back().stops_) {
			buses_on_stops_[stop->name_].insert(buses_.back().name_);
		}
	}

	domain::Stop* TransportCatalogue::FindStop(const std::string& name) const {
		if (stops_name_.count(name) == 0) {
			throw std::out_of_range("Stop "s + name + " does not exist in catalogue"s);
		}
		return stops_name_.at(name);
	}

	domain::Route* TransportCatalogue::FindRoute(const std::string& name) const {
		if (routes_name_.count(name) == 0) {
			throw std::out_of_range("Route "s + name + " does not exist in catalogue"s);
		}
		return routes_name_.at(name);
	}

	double TransportCatalogue::CalculateRouteLength(const domain::Route* route) const noexcept {
		double result = 0.0;
		if (route != nullptr) {
			for (auto iter1 = route->stops_.begin(), iter2 = iter1 + 1;
				iter2 < route->stops_.end();
				++iter1, ++iter2) {
				result += ComputeDistance((*iter1)->coordinates_, (*iter2)->coordinates_);
			}
			if (route->type_route_ == domain::RouteType::LINEAR) {
				result *= 2;
			}
		}
		return result;
	}

	domain::RouteInfo TransportCatalogue::GetRouteInfo(const std::string& name) const {
		domain::RouteInfo info;
		auto* bus = routes_name_.at(name);
		info.name = bus->name_;
		info.type_route = bus->type_route_;
		info.num_of_stops = static_cast<int>(bus->stops_.size());
		std::unordered_set<std::string> temp;
		temp.insert(bus->stops_[0]->name_);
		for (size_t i = 1; i < bus->stops_.size(); ++i) {
			temp.insert(bus->stops_[i]->name_);
		}
		info.route_length = CalculationGivenDistance(bus);
		if (bus->type_route_ == domain::RouteType::LINEAR) {
			info.num_of_stops = info.num_of_stops * 2 - 1;
		}
		info.num_of_unique_stops = static_cast<int>(temp.size());
		info.curvature = info.route_length / CalculateRouteLength(bus);
		return info;
	}

	std::optional<std::reference_wrapper<const std::set<std::string_view>>>
		TransportCatalogue::GetBusesOnStop(const std::string& stop_name) const {
		const auto pos = buses_on_stops_.find(FindStop(stop_name)->name_);
		if (pos != buses_on_stops_.end()) {
			return std::cref(pos->second);
		}
		else {
			return std::nullopt;
		}
		//Здесь я и так возвращаю nullptr. optional<> это же просто обёртка над указателем.!!!!!!!!!!!!!!!
	}

	const std::unordered_map<std::string_view, std::set<std::string_view>>&
		TransportCatalogue::GetBusesOnStops() const {
		return buses_on_stops_;
	}

	void TransportCatalogue::SetDistance(const std::string& from, const std::string& to, int distance) {
		auto from_ = FindStop(from);
		auto to_ = FindStop(to);
		std::pair pair(from_->name_, to_->name_);
		distances_[pair] = distance;
	}

	int TransportCatalogue::GetForwardDistance(const std::string& stop_from,
		const std::string& stop_to) const {
		std::pair pair_stops(stop_from, stop_to);
		if (distances_.count(pair_stops) != 0) {
			return distances_.at(pair_stops);
		}
		return 0;
	}

	int TransportCatalogue::GetDistance(const std::string& stop_from, const std::string& stop_to) const {
		int result = 0;
		result = GetForwardDistance(stop_from, stop_to);
		if (result == 0) {
			result = GetForwardDistance(stop_to, stop_from);
		}
		return result;
	}

	int TransportCatalogue::CalculationGivenDistance(const domain::Route* route) const {
		int result = 0;
		if (route != nullptr) {
			for (auto iter1 = route->stops_.begin(), iter2 = iter1 + 1;
				iter2 < route->stops_.end(); ++iter1, ++iter2) {
				result += GetDistance((*iter1)->name_, (*iter2)->name_);
			}
			if (route->type_route_ == domain::RouteType::LINEAR) {
				for (auto iter1 = route->stops_.rbegin(), iter2 = iter1 + 1;
					iter2 < route->stops_.rend(); ++iter1, ++iter2) {
					result += GetDistance((*iter1)->name_, (*iter2)->name_);
				}
			}
		}
		return result;
	}

	const std::unordered_map<std::string_view, domain::Stop*>& TransportCatalogue::GetStops() const {
		return stops_name_;
	}

	const std::unordered_map<std::string_view, domain::Route*>& TransportCatalogue::GetRoutes() const {
		return routes_name_;
	}
}//transport_catalogue

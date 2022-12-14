#include "transport_catalogue.h"
#include "geo.h"

#include <algorithm>
#include <set>
#include <iostream>

namespace transport_catalogue {
	using namespace std::string_literals;

	size_t HasherCatalogue::operator()(const std::string_view name) const {
		return hasher_(name);
	}

	TransportCatalogue::Stop::Stop(const std::string& name, Coordinates coordinates) :
		name_(name),
		coordinates_(coordinates) {}

	TransportCatalogue::Route::Route(const std::string& name, RouteType type_route, std::vector<Stop*>& route) :
		name_(name),
		type_route_(type_route),
		stops_(route) {}

	bool TransportCatalogue::Stop::operator== (const Stop& rhs) {
		return rhs.name_ == this->name_;
	}

	bool TransportCatalogue::Route::operator== (const Route& rhs) {
		return rhs.name_ == this->name_;
	}

	void TransportCatalogue::AddStop(const std::string& name, const Coordinates& coordinate) {
		if (stops_name_.find(name) != stops_name_.end()) {
			std::cout << "the stop \"" << name << "\" is already there." << std::endl;
			return;
		}
		Stop stop(name, coordinate);
		stops_.push_back(stop);
		stops_name_[stops_.back().name_] = &stops_.back();
	}

	void TransportCatalogue::AddRoute(const std::string& name, RouteType type_route_, const std::vector<std::string>& stops_str) {
		std::vector<Stop*> stops(stops_str.size());
		for (int i = 0; i < stops_str.size(); ++i) {
			stops[i] = stops_name_.at(stops_str[i]);
		}

		Route bus(name, type_route_, stops);
		buses_.push_back(std::move(bus));
		routes_name_[buses_.back().name_] = &buses_.back();
		for (auto stop : buses_.back().stops_) {
			buses_on_stops_[stop->name_].insert(buses_.back().name_);
		}
	}

	TransportCatalogue::Stop* TransportCatalogue::FindStop(const std::string& name) const {
		if (stops_name_.count(name) == 0) {
			throw std::out_of_range("Stop "s + name + " does not exist in catalogue"s);
		}
		return stops_name_.at(name);
	}

	TransportCatalogue::Route* TransportCatalogue::FindRoute(const std::string& name) const {
		if (routes_name_.count(name) == 0) {
			throw std::out_of_range("Route "s + name + " does not exist in catalogue"s);
		}
		return routes_name_.at(name);
	}

	double TransportCatalogue::CalculateRouteLength(const Route* route) noexcept {
		double result = 0.0;
		if (route != nullptr) {
			for (auto iter1 = route->stops_.begin(), iter2 = iter1 + 1;
				iter2 < route->stops_.end();
				++iter1, ++iter2) {
				result += ComputeDistance((*iter1)->coordinates_, (*iter2)->coordinates_);
			}
			if (route->type_route_ == RouteType::LINEAR) {
				result *= 2;
			}
		}
		return result;
	}

	RouteInfo TransportCatalogue::GetBusInfo(std::string& name) {
		RouteInfo info;
		auto* bus = routes_name_.at(name);
		info.name = bus->name_;
		info.type_route = bus->type_route_;
		info.num_of_stops = static_cast<int>(bus->stops_.size());
		std::unordered_set<std::string> temp;
		temp.insert(bus->stops_[0]->name_);
		for (int i = 1; i < bus->stops_.size(); ++i) {
			temp.insert(bus->stops_[i]->name_);
		}
		info.route_length = CalculationGivenDistance(bus);
		if (bus->type_route_ == RouteType::LINEAR) {
			info.num_of_stops = info.num_of_stops * 2 - 1;
		}
		info.num_of_unique_stops = static_cast<int>(temp.size());
		info.curvature = info.route_length / CalculateRouteLength(bus);
		return info;
	}

	const std::unordered_map<std::string_view, std::set<std::string_view>>&
		TransportCatalogue::GetBusesOnStops() const {
		return buses_on_stops_;
	}

	void TransportCatalogue::SetDistance(const std::string& from, const std::string& to, int distance) {
		auto from_ = FindStop(from);
		auto to_ = FindStop(to);
		distances_[from_->name_][to_->name_] = distance;
	}

	int TransportCatalogue::GetForwardDistance(const std::string& stop_from,
		const std::string& stop_to) const {
		if (distances_.count(stop_from) == 0 || distances_.at(stop_from).count(stop_to) == 0) {
			throw std::out_of_range("No information about distance from "s
				+ stop_from + " to "s + stop_to);
		}
		return distances_.at(stop_from).at(stop_to);
	}

	int TransportCatalogue::GetDistance(const std::string& stop_from, const std::string& stop_to) const {
		int result = 0;
		try {
			result = GetForwardDistance(stop_from, stop_to);
		}
		catch (std::out_of_range&) {
			try {
				result = GetForwardDistance(stop_to, stop_from);
			}
			catch (std::out_of_range&) {
				throw std::out_of_range("No information about distance between stops "s
					+ stop_from + " and "s + stop_to);
			}
		}
		return result;
	}

	int TransportCatalogue::CalculationGivenDistance(const Route* route) const {
		int result = 0;
		if (route != nullptr) {
			for (auto iter1 = route->stops_.begin(), iter2 = iter1 + 1;
				iter2 < route->stops_.end(); ++iter1, ++iter2) {
				result += GetDistance((*iter1)->name_, (*iter2)->name_);
			}
			if (route->type_route_ == RouteType::LINEAR) {
				for (auto iter1 = route->stops_.rbegin(), iter2 = iter1 + 1;
					iter2 < route->stops_.rend(); ++iter1, ++iter2) {
					result += GetDistance((*iter1)->name_, (*iter2)->name_);
				}
			}
		}
		return result;
	}
}//transport_catalogue

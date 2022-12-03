#pragma once
#include "geo.h"

#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <string_view>
#include <set>
#include <optional>

//класс транспортного справочника;

namespace transport_catalogue {

	enum class RouteType {
		UNKNOWN,
		LINEAR,
		CIRCLE,
	};

	struct RouteInfo {
		std::string name;//
		RouteType type_route;//
		int num_of_stops = 0;//
		int num_of_unique_stops = 0;//
		int route_length = 0;//
		double curvature = 0.0;
	};

	struct HasherCatalogue {
	private:
		std::hash<std::string_view> hasher_;
	public:
		size_t operator()(const std::string_view name) const;
	};

	class TransportCatalogue {
	private:
		struct Stop {
			Stop() {}
			Stop(const std::string& name, Coordinates coordinates);
			bool operator== (const Stop& rhs);

			std::string name_;
			Coordinates coordinates_;
		};

		struct Route {
			Route() {}
			Route(const std::string& name, RouteType type_route, std::vector<Stop*>& route);
			bool operator== (const Route& rhs);

			std::string name_;
			RouteType type_route_ = RouteType::UNKNOWN;
			std::vector<Stop*> stops_;
		};

		std::deque<Stop> stops_;
		std::deque<Route> buses_;

		std::unordered_map<std::string_view, Route*, HasherCatalogue> routes_name_;
		std::unordered_map<std::string_view, Stop*, HasherCatalogue> stops_name_;

		std::unordered_map<std::string_view, std::set<std::string_view>> buses_on_stops_;
		std::unordered_map<std::string_view, std::unordered_map<std::string_view, int>> distances_;
	public:
		void Addstop(const std::string& name, Coordinates& coordinate);
		void AddRoute(const std::string& name, RouteType type_route_, std::vector<std::string>& stops_str);

		Stop* FindStop(const std::string& name) const;
		Route* FindRoute(const std::string& name) const;

		RouteInfo GetBusInfo(std::string& name);
		double CalculateRouteLength(const Route* route) noexcept;

		const std::unordered_map<std::string_view, std::set<std::string_view>>&
			GetBusesOnStops() const;

		void SetDistance(const std::string& from, const std::string& to, int distance);
		int GetForwardDistance(const std::string& stop_from, const std::string& stop_to) const;
		int GetDistance(const std::string& stop_from, const std::string& stop_to) const;
		int CalculationGivenDistance(const Route* route) const;
	};
}//transport_catalogue 

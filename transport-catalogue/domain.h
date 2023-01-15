#pragma once
#include <string>
#include <vector>

#include "geo.h"

namespace domain {
	enum class RouteType {
		UNKNOWN,
		LINEAR,
		CIRCLE,
	};

	struct RouteInfo {
		std::string name;
		RouteType type_route;
		int num_of_stops = 0;
		int num_of_unique_stops = 0;
		int route_length = 0;
		double curvature = 0.0;
	};

	struct Stop {
		Stop() {}
		Stop(const std::string& name, geo::Coordinates coordinates);
		friend bool operator==(const Stop& lhs,const Stop& rhs);

		std::string name_;
		geo::Coordinates coordinates_;
	};

	struct Route {
		Route() {}
		Route(const std::string& name, RouteType type_route, std::vector<Stop*>& route);
		friend bool operator==(const Route& lhs, const Route& rhs);

		std::string name_;
		RouteType type_route_ = RouteType::UNKNOWN;
		std::vector<Stop*> stops_;
	};
}//namespace domain

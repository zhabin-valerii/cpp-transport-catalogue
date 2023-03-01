#include "domain.h"

namespace domain {
	Stop::Stop(const std::string& name, geo::Coordinates coordinates) :
		name_(name),
		coordinates_(coordinates) {}

	size_t StopHasher::operator()(const std::pair<Stop*, Stop*>& stops) const {
		size_t h_from = hasher(stops.first);
		size_t h_to = hasher(stops.second);
		return h_from * 31 + h_to;
	}

	Route::Route(const std::string& name, RouteType type_route, std::vector<Stop*>& route) :
		name_(name),
		type_route_(type_route),
		stops_(route) {}

	bool operator==(const Stop& lhs, const Stop& rhs) {
		return (lhs.name_ == rhs.name_ && lhs.coordinates_ == rhs.coordinates_);
	}

	bool operator==(const Route& lhs, const Route& rhs) {
		return lhs.name_ == rhs.name_;
	}

}//namespace domain
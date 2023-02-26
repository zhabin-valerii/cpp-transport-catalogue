#pragma once
#include "geo.h"
#include "domain.h"

#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <string_view>
#include <set>
#include <optional>
#include <memory>

//класс транспортного справочника;

namespace transport_catalogue {
	using namespace geo;

	class TransportCatalogue {
	public:
		void AddStop(const std::string& name, const Coordinates coordinate);
		void AddRoute(const std::string& name, domain::RouteType type_route_, const std::vector<std::string>& stops_str);

		domain::Stop* FindStop(const std::string& name) const;
		domain::Route* FindRoute(const std::string& name) const;

		domain::RouteInfo GetRouteInfo(const std::string& name) const;
		double CalculateRouteLength(const domain::Route* route) const noexcept;

		std::optional<std::reference_wrapper<const std::set<std::string_view>>> 
			GetBusesOnStop(const std::string& stop_name) const;
		const std::unordered_map<std::string_view, std::set<std::string_view>>& GetBusesOnStops() const;

		void SetDistance(const std::string& from, const std::string& to, int distance);
		int GetForwardDistance(const std::string& stop_from, const std::string& stop_to) const;
		int GetDistance(const std::string& stop_from, const std::string& stop_to) const;
		int CalculationGivenDistance(const domain::Route* route) const;

		const std::unordered_map<std::string_view, domain::Stop*>&
			GetStops() const;
		const std::unordered_map<std::string_view, domain::Route*>&
			GetRoutes() const;
	private:

		std::deque<domain::Stop> stops_;
		std::deque<domain::Route> buses_;

		std::unordered_map<std::string_view, domain::Route*> routes_name_;
		std::unordered_map<std::string_view, domain::Stop*> stops_name_;

		std::unordered_map<std::string_view, std::set<std::string_view>> buses_on_stops_;
		std::unordered_map<std::pair<std::string_view, std::string_view>,int, domain::StopHasher> distances_;
		//std::unordered_map<std::string_view, std::unordered_map<std::string_view, int>> distances_;
	};
}//transport_catalogue 

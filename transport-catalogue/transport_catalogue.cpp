#include "transport_catalogue.h"

namespace tr_cat {
namespace aggregations {

void TransportCatalogue::AddStop (std::string_view name, geo::Coordinates coords) {
    stops_data_.push_back({static_cast<std::string>(name), coords,
                             std::set<std::string_view>{}, vertex_count_++});
    stops_container_[stops_data_.back().name] = &(stops_data_.back());
}

void TransportCatalogue::AddBus (const std::string_view name,
                                 std::vector<std::string_view>& stops, const bool is_ring) {

    auto it = std::lower_bound(sorted_buses_.begin(), sorted_buses_.end(), name);
    if (it != sorted_buses_.end() && *it == name) {
        return;
    }
    buses_data_.push_back({static_cast<std::string>(name), {}});
    sorted_buses_.emplace(it, buses_data_.back().name);

    //добавление к каждой остановке названия этого автобуса
    for (std::string_view stop : stops) {
        stops_container_[stop]->buses.insert(buses_data_.back().name);
    }

    std::vector<Stop*> tmp_stops(stops.size());

    //из названий в указатели на существующие остановки
    std::transform(stops.begin(), stops.end(), tmp_stops.begin(), [&] (std::string_view element) {
        return stops_container_[element];});

    //если остановок нет
    if (tmp_stops.empty()) {
        buses_container_.insert({buses_data_.back().name, &(buses_data_.back())});
        return;
    }

    //запись указателей на уникальные остановки для подсчета
    std::unordered_set<Stop*>tmp_unique_stops;
    std::for_each(tmp_stops.begin(), tmp_stops.end(), [&](Stop* element) {
            tmp_unique_stops.insert(element); });
    buses_data_.back().unique_stops = static_cast<int>(tmp_unique_stops.size());

    //если линейный маршрут, то добавление обратного направления
    if (!is_ring) {
        tmp_stops.reserve(tmp_stops.size()*2-1);
        for (auto it = tmp_stops.end() - 2; it != tmp_stops.begin(); --it) {
            tmp_stops.push_back(*it);
        }
        tmp_stops.push_back(tmp_stops.front());
    } else {
        buses_data_.back().is_ring = true;
    }

    buses_data_.back().stops = move(tmp_stops);

    buses_container_.insert({buses_data_.back().name, &(buses_data_.back())});

    buses_data_.back().distance = ComputeRouteDistance(name);
    buses_data_.back().curvature = buses_data_.back().distance / ComputeGeoRouteDistance(name);
}

void TransportCatalogue::AddDistance(const std::string_view lhs_name, const std::string_view rhs_name, double distance) {

    const Stop* lhs = FindStop(lhs_name);
    const Stop* rhs = FindStop(rhs_name);

    distances_[{lhs, rhs}] = static_cast<int>(distance);
}

std::optional<const Bus*> TransportCatalogue::GetBusInfo (std::string_view name) const {

    const Bus* bus = FindBus(name);

    if (!bus) {
        return std::nullopt;
    }

    return bus;
}

std::optional<const Stop*> TransportCatalogue::GetStopInfo (std::string_view name) const {

    const Stop* stop = FindStop(name);
    if (!stop) {
        return std::nullopt;
    }
    return stop;
}

int TransportCatalogue::GetDistance(const Stop* lhs, const Stop* rhs) const{

    if (distances_.count ({lhs, rhs})) {
        return distances_.at({lhs, rhs});
    }

    if (distances_.count ({rhs, lhs})) {
        return distances_.at({rhs, lhs});
    }

    return static_cast<int>(geo::ComputeDistance(lhs->coordinates, rhs->coordinates));
}

std::vector<std::string_view> TransportCatalogue::GetSortedStopsNames() const {
    std::vector<std::string_view> result;
    result.reserve(stops_data_.size());
    for (const Stop& stop : stops_data_) {
        result.emplace_back(stop.name);
    }
    std::sort(result.begin(), result.end());
    return result;
}

std::deque<Bus> TransportCatalogue::GetBuses() {
    return buses_data_;
}

std::deque<Stop> TransportCatalogue::GetStops() {
    return stops_data_;
}

std::unordered_map<std::pair<const Stop*, const Stop*>, int, DistanceHasher, DistanceCompare>
TransportCatalogue::GetDistances() {
    return distances_;
}

//--------------------------private-------------------------------------
Stop* TransportCatalogue::FindStop (std::string_view name) const {
    if (!stops_container_.count(name)) {
        return nullptr;
    }
    return stops_container_.at(name);
}

Bus* TransportCatalogue::FindBus (std:: string_view name) const {
    if (!buses_container_.count(name)) {
        return nullptr;
    }
    return buses_container_.at(name);
}

int TransportCatalogue::ComputeRouteDistance (std::string_view name) const {

    const Bus* bus = FindBus(name);
    int distance = 0;
    const std::vector<Stop*>& stops = bus->stops;

    for (size_t i = 1; i < stops.size(); ++i) {
        distance += GetDistance(stops[i-1], stops[i]);
    }
    return distance;
}

double TransportCatalogue::ComputeGeoRouteDistance (std::string_view name) const {
    const Bus* bus = FindBus(name);
    double distance = 0;
    const std::vector<Stop*>& stops = bus->stops;

    for (size_t i = 1; i < stops.size(); ++i) {
        distance += geo::ComputeDistance(stops[i-1]->coordinates, stops[i]->coordinates);
    }
    return distance;
}

std::vector<const Stop*> TransportCatalogue::SortStops() const {
    std::vector<const Stop*>sorted_stops;
    sorted_stops.reserve(stops_data_.size());
    for (const Stop& stop : stops_data_) {
        sorted_stops.push_back(&stop);
    }
    std::sort(sorted_stops.begin(), sorted_stops.end(),
              [](const Stop* lhs, const Stop* rhs){return lhs->name < rhs->name;});
    return sorted_stops;
}
}//aggregations
}//tr_cat

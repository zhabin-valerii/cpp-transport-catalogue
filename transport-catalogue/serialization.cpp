#include <optional>
#include "serialization.h"

using namespace router;

namespace tr_cat {
namespace serialize {

size_t Serializator::Serialize(bool with_graph) const {
    transport_catalog_serialize::AllData all_data;
    *all_data.mutable_catalog() = SerializeCatalog();
    *all_data.mutable_render_settings() = SerializeRenderer();
    *all_data.mutable_router_data() = SerializeRouter(with_graph);
    std::ofstream out (path_to_serialize_, std::ios::binary | std::ios::trunc);
    all_data.SerializePartialToOstream(&out);
    return sizeof(path_to_serialize_);
}

bool Serializator::Deserialize(bool with_graph) {
    transport_catalog_serialize::AllData all_data;
    std::ifstream in (path_to_serialize_, std::ios::binary);
    all_data.ParseFromIstream(&in);
    DeserializeCatalog(*all_data.mutable_catalog());
    DeserializeRenderer(*all_data.mutable_render_settings());
    DeserializeRouter(*all_data.mutable_router_data(), with_graph);
    return true;
}


transport_catalog_serialize::Catalog Serializator::SerializeCatalog() const {
    std::vector<const Stop*> sorted_stops = catalog_.SortStops();
    //-------buses---------
    transport_catalog_serialize::BusList bus_list;
    for (const Bus& bus : catalog_.GetBuses()) {
        transport_catalog_serialize::Bus bus_to_out;
        bus_to_out.set_name(bus.name);
        bus_to_out.set_is_ring(bus.is_ring);
        if (!bus.stops.empty()) {
            //если некольцевой маршрут, записывается только половина остановок
            int stops_count = bus.is_ring ? bus.stops.size() : bus.stops.size() / 2 + 1;

            //список остановок состоит из их порядковых номеров в отсортированном массиве
            for (int i = 0; i < stops_count; ++i) {
                const Stop* stop = bus.stops[i];
                int pos = std::lower_bound(sorted_stops.begin(), sorted_stops.end(),
                    stop, [](const Stop* lhs, const Stop* rhs) {
                        return lhs->name < rhs->name; }) - sorted_stops.begin();
                        bus_to_out.add_stop(pos);
            }
        }
        bus_list.add_bus();
        *bus_list.mutable_bus(bus_list.bus_size() - 1) = bus_to_out;
    }
    //--------stops----------
    transport_catalog_serialize::StopList stop_list;
    for (const Stop& stop : catalog_.GetStops()) {
        transport_catalog_serialize::Stop stop_to_out;
        stop_to_out.set_name(stop.name);
        stop_to_out.set_latitude(stop.coordinates.lat);
        stop_to_out.set_longitude(stop.coordinates.lng);
        stop_list.add_stop();
        *stop_list.mutable_stop(stop_list.stop_size() - 1) = stop_to_out;
    }
    //-------distances--------
    transport_catalog_serialize::DistanceList distance_list;
    for (const auto& [key, value] : catalog_.GetDistances()) {
        transport_catalog_serialize::Distance distance_to_out;
        int pos = std::lower_bound(sorted_stops.begin(), sorted_stops.end(),
            key.first, [](const Stop* lhs, const Stop* rhs) {
                return lhs->name < rhs->name; }) - sorted_stops.begin();
                distance_to_out.set_index_from(pos);
                pos = std::lower_bound(sorted_stops.begin(), sorted_stops.end(),
                    key.second, [](const Stop* lhs, const Stop* rhs) {
                        return lhs->name < rhs->name; }) - sorted_stops.begin();
                        distance_to_out.set_index_to(pos);
                        distance_to_out.set_distance(value);
                        distance_list.add_distance();
                        *distance_list.mutable_distance(distance_list.distance_size() - 1) = distance_to_out;
    }
    //----------------------
    transport_catalog_serialize::Catalog catalog;
    *catalog.mutable_bus_list() = bus_list;
    *catalog.mutable_stop_list() = stop_list;
    *catalog.mutable_distance_list() = distance_list;
    return catalog;
}

bool Serializator::DeserializeCatalog(transport_catalog_serialize::Catalog& catalog) {
    //------------stops-----------------
    transport_catalog_serialize::StopList stop_list = catalog.stop_list();
    for (int i = 0; i < stop_list.stop_size(); ++i) {
        const transport_catalog_serialize::Stop& stop = stop_list.stop(i);
        catalog_.AddStop(stop.name(), { stop.latitude(), stop.longitude() });
    }

    std::vector<const Stop*> sorted_stops = catalog_.SortStops();
    //------------distances-------------
    transport_catalog_serialize::DistanceList distance_list = catalog.distance_list();
    for (int i = 0; i < distance_list.distance_size(); ++i) {
        const transport_catalog_serialize::Distance distance = distance_list.distance(i);
        catalog_.AddDistance(sorted_stops[distance.index_from()]->name, sorted_stops[distance.index_to()]->name, distance.distance());
    }
    //-----------buses------------------
    transport_catalog_serialize::BusList bus_list = catalog.bus_list();
    for (int i = 0; i < bus_list.bus_size(); ++i) {
        const transport_catalog_serialize::Bus& bus_from_input = bus_list.bus(i);
        std::vector<std::string_view> stops_in_bus;
        stops_in_bus.reserve(bus_from_input.stop_size());
        for (int i = 0; i < bus_from_input.stop_size(); ++i) {
            stops_in_bus.push_back(sorted_stops[bus_from_input.stop(i)]->name);
        }
        catalog_.AddBus(bus_from_input.name(), stops_in_bus, bus_from_input.is_ring());
    }
    return true;
}

transport_catalog_serialize::RenderSettings Serializator::SerializeRenderer() const {
    struct ColorGetter {

        optional<transport_catalog_serialize::Color> operator()(monostate) { return nullopt; }

        optional<transport_catalog_serialize::Color> operator()(const string& name) {
            transport_catalog_serialize::Color result_color;
            result_color.set_str(name);
            return result_color;
        }

        optional<transport_catalog_serialize::Color> operator() (const Rgb& rgb) {
            transport_catalog_serialize::Color result_color;
            transport_catalog_serialize::Rgb rgb_proto;
            rgb_proto.set_r(static_cast<uint32_t>(rgb.red));
            rgb_proto.set_g(static_cast<uint32_t>(rgb.green));
            rgb_proto.set_b(static_cast<uint32_t>(rgb.blue));
            *result_color.mutable_rgb() = rgb_proto;
            return result_color;
        }

        optional<transport_catalog_serialize::Color> operator() (const Rgba& rgba) {
            transport_catalog_serialize::Color result_color;
            transport_catalog_serialize::Rgba rgba_proto;
            rgba_proto.set_r(static_cast<uint32_t>(rgba.red));
            rgba_proto.set_g(static_cast<uint32_t>(rgba.green));
            rgba_proto.set_b(static_cast<uint32_t>(rgba.blue));
            rgba_proto.set_a(rgba.opacity);
            *result_color.mutable_rgba() = rgba_proto;
            return result_color;
        }

    };
    auto settings_ = renderer_.GetSettings();
    transport_catalog_serialize::RenderSettings settings_to_out;
    settings_to_out.set_width(settings_.width);
    settings_to_out.set_height(settings_.height);
    settings_to_out.set_padding(settings_.padding);
    settings_to_out.set_line_width(settings_.line_width);
    settings_to_out.set_stop_radius(settings_.stop_radius);
    settings_to_out.set_bus_label_font_size(settings_.bus_label_font_size);
    settings_to_out.set_bus_label_offset_x(settings_.bus_label_offset.x);
    settings_to_out.set_bus_label_offset_y(settings_.bus_label_offset.y);
    settings_to_out.set_stop_label_font_size(settings_.stop_label_font_size);
    settings_to_out.set_stop_label_offset_x(settings_.stop_label_offset.x);
    settings_to_out.set_stop_label_offset_y(settings_.stop_label_offset.y);
    *settings_to_out.mutable_underlayer_color() = *visit(ColorGetter(), settings_.underlayer_color);
    settings_to_out.set_underlayer_width(settings_.underlayer_width);

    for (auto& color : settings_.color_palette) {
        settings_to_out.add_color_palette();
        *settings_to_out.mutable_color_palette(settings_to_out.color_palette_size() - 1) =
            *visit(ColorGetter{}, color);
    }
    return settings_to_out;
}

bool Serializator::DeserializeRenderer(transport_catalog_serialize::RenderSettings& settings_in) {
    struct ColorGetter {
        Color operator() (transport_catalog_serialize::Color& color) {
            if (!color.str().empty()) {
                return color.str();
            }
            if (color.has_rgb()) {
                transport_catalog_serialize::Rgb rgb = *color.mutable_rgb();
                return Rgb{ rgb.r(), rgb.g(), rgb.b() };
            }
            if (color.has_rgba()) {
                transport_catalog_serialize::Rgba rgba = *color.mutable_rgba();
                return Rgba{ rgba.r(), rgba.g(), rgba.b(), rgba.a() };
            }
            return monostate();
        }
    };
    renderer_.GetSettingsRef().width = settings_in.width();
    renderer_.GetSettingsRef().height = settings_in.height();
    renderer_.GetSettingsRef().padding = settings_in.padding();
    renderer_.GetSettingsRef().line_width = settings_in.line_width();
    renderer_.GetSettingsRef().stop_radius = settings_in.stop_radius();
    renderer_.GetSettingsRef().bus_label_font_size = settings_in.bus_label_font_size();
    renderer_.GetSettingsRef().bus_label_offset = { settings_in.bus_label_offset_x(),
                                  settings_in.bus_label_offset_y() };
    renderer_.GetSettingsRef().stop_label_font_size = settings_in.stop_label_font_size();
    renderer_.GetSettingsRef().stop_label_offset = { settings_in.stop_label_offset_x(),
                                   settings_in.stop_label_offset_y() };
    renderer_.GetSettingsRef().underlayer_color = ColorGetter()(*settings_in.mutable_underlayer_color());
    renderer_.GetSettingsRef().underlayer_width = settings_in.underlayer_width();

    for (int i = 0; i < settings_in.color_palette_size(); ++i) {
        renderer_.GetSettingsRef().color_palette.push_back(ColorGetter()(*settings_in.mutable_color_palette(i)));
    }
    return true;
}

transport_catalog_serialize::Router Serializator::SerializeRouter(bool with_graph) const {
    transport_catalog_serialize::Router data_out;
    transport_catalog_serialize::RoutingSettings settings;
    settings.set_bus_wait_time(transport_router_.GetSettings().bus_wait_time);
    settings.set_bus_velocity(transport_router_.GetSettings().bus_velocity);
    *data_out.mutable_settings() = settings;
    *data_out.mutable_data() = transport_router_.GetRouter()->GetSerializeData();
    if (with_graph) {
        *data_out.mutable_graph() = transport_router_.GetGraph().GetSerializeData();
        std::vector<std::string_view> buses(catalog_.begin(), catalog_.end());
        std::vector<std::string_view> stops = catalog_.GetSortedStopsNames();
        for (const auto& [edge_id, edge_info] : transport_router_.GetEdges()) {
            transport_catalog_serialize::EdgeInfo info_to_out;
            auto it_stop = std::lower_bound(stops.begin(), stops.end(),
                edge_info.stop->name, std::less<>{});
            info_to_out.set_stop(static_cast<uint32_t>(it_stop - stops.begin()));
            auto it_bus = std::lower_bound(buses.begin(), buses.end(),
                edge_info.bus->name, std::less<>{});
            info_to_out.set_bus(static_cast<uint32_t>(it_bus - buses.begin()));
            info_to_out.set_count(edge_info.count);
            (*data_out.mutable_graph()->mutable_info())[edge_id] = info_to_out;
        }
    }
    return data_out;
}

bool Serializator::DeserializeRouter(transport_catalog_serialize::Router& router_data, bool with_graph) {
    transport_router_.GetSettingsRef() = { router_data.settings().bus_wait_time(),
                         router_data.settings().bus_velocity() };
    const transport_catalog_serialize::Graph& graph = router_data.graph();
    if (with_graph) {
        std::vector<std::string_view> buses(catalog_.begin(), catalog_.end());
        std::vector<std::string_view> stops = catalog_.GetSortedStopsNames();
        transport_router_.GetGraphRef().SetVertexCount(stops.size());
        for (int i = 0; i < graph.edges_size(); ++i) {
            uint32_t edge_id = transport_router_.GetGraphRef().AddEdge({ graph.edges(i).from(),
                                                 graph.edges(i).to(),
                                                 graph.edges(i).weight() });
            const transport_catalog_serialize::EdgeInfo& edge_info = (graph.info().at(edge_id));
            transport_router_.GetEdgesRef()[edge_id] = { *catalog_.GetStopInfo(stops[edge_info.stop()]),
                                       *catalog_.GetBusInfo(buses[edge_info.bus()]),
                                       edge_info.count() };
        }
    }
    else {
        transport_router_.CreateGraph(false);
    }
    transport_router_.GetRouterRef() = std::make_unique<graph::Router<double>>(transport_router_.GetGraphRef(), router_data.data());
    return true;
}



}//serialize
}//tr_cat

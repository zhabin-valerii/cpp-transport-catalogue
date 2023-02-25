#include "json_reader.h"

#include <tuple>

namespace json_reader {
	using namespace geo;
	using namespace std::string_literals;

	JsonReader::JsonReader(std::istream& input) : data_(json::Load(input)) {}

	void JsonReader::LoadData(transport_catalogue::TransportCatalogue& catalogue) const {
		if (data_.GetRoot().IsDict() && data_.GetRoot().AsDict().count("base_requests"s) != 0) {
			auto& base_request = data_.GetRoot().AsDict().at("base_requests"s);
			if (base_request.IsArray()) {
				LoadStops(base_request.AsArray(), catalogue);
				LoadRoutes(base_request.AsArray(), catalogue);
				LoadDistances(base_request.AsArray(), catalogue);
			}
		}
	}

	std::optional<renderer::RenderSettings> JsonReader::LoadRenderSettings() const {
		if (data_.GetRoot().IsDict() && data_.GetRoot().AsDict().count("render_settings"s) > 0) {
			auto& render_settings = data_.GetRoot().AsDict().at("render_settings"s);
			if (render_settings.IsDict()) {
				return LoadSettings(render_settings.AsDict());
			}
		}
		return std::nullopt;
	}

	renderer::RenderSettings JsonReader::LoadSettings(const json::Dict& data) const {
		renderer::RenderSettings result;
		if (data.count("width"s) != 0 && data.at("width"s).IsDouble()) {
			result.size.x = data.at("width"s).AsDouble();
		}
		if (data.count("height"s) != 0 && data.at("height"s).IsDouble()) {
			result.size.y = data.at("height"s).AsDouble();
		}
		if (data.count("padding"s) != 0 && data.at("padding"s).IsDouble()) {
			result.padding = data.at("padding"s).AsDouble();
		}
		if (data.count("line_width"s) != 0 && data.at("line_width"s).IsDouble()) {
			result.line_width = data.at("line_width"s).AsDouble();
		}
		if (data.count("stop_radius"s) != 0 && data.at("stop_radius"s).IsDouble()) {
			result.stop_radius = data.at("stop_radius"s).AsDouble();
		}
		if (data.count("bus_label_font_size"s) != 0 && data.at("bus_label_font_size"s).IsInt()) {
			result.bus_label_font_size = data.at("bus_label_font_size"s).AsInt();
		}
		if (data.count("bus_label_offset"s) != 0 && data.at("bus_label_offset"s).IsArray()) {
			result.bus_label_offset = ReadOffset(data.at("bus_label_offset"s).AsArray());
		}
		if (data.count("stop_label_font_size"s) != 0 && data.at("stop_label_font_size"s).IsInt()) {
			result.stop_label_font_size = data.at("stop_label_font_size"s).AsInt();
		}
		if (data.count("stop_label_offset"s) != 0 && data.at("stop_label_offset"s).IsArray()) {
			result.stop_label_offset = ReadOffset(data.at("stop_label_offset"s).AsArray());
		}
		if (data.count("underlayer_color"s) != 0) {
			result.underlayer_color = ReadColor(data.at("underlayer_color"s));
		}
		if (data.count("underlayer_width"s) != 0 && data.at("underlayer_width"s).IsDouble()) {
			result.underlayer_width = data.at("underlayer_width"s).AsDouble();
		}
		if (data.count("color_palette"s) != 0 && data.at("color_palette"s).IsArray()) {
			for (auto& color : data.at("color_palette"s).AsArray()) {
				result.color_palette.push_back(ReadColor(color));
			}
		}
		return result;
	}

	void JsonReader::LoadStops(const json::Array& data, transport_catalogue::TransportCatalogue& catalogue) {
		for (const auto& elem : data) {
			if (IsStop(elem)) {
				const auto& name = elem.AsDict().at("name"s).AsString();
				const auto lat = elem.AsDict().at("latitude"s).AsDouble();
				const auto lng = elem.AsDict().at("longitude"s).AsDouble();
				catalogue.AddStop(name, { lat,lng });
			}
		}
	}

	void JsonReader::LoadRoutes(const json::Array& data, transport_catalogue::TransportCatalogue& catalogue) {
		for (const auto& elem : data) {
			if (IsRoute(elem)) {
				const auto& name = elem.AsDict().at("name"s).AsString();
				domain::RouteType type;
				if (elem.AsDict().at("is_roundtrip"s).AsBool()) {
					type = domain::RouteType::CIRCLE;
				}
				else {
					type = domain::RouteType::LINEAR;
				}
				const auto stops = elem.AsDict().at("stops"s).AsArray();
				std::vector<std::string> stops_names;
				for (const auto& stop_name : stops) {
					if (stop_name.IsString()) {
						stops_names.push_back(stop_name.AsString());
					}
				}
				catalogue.AddRoute(name, type, stops_names);
			}
		}
	}

	void JsonReader::LoadDistances(const json::Array& data, transport_catalogue::TransportCatalogue& catalogue) {
		for (const auto& elem : data) {
			if (IsStop(elem)) {
				const auto& from = elem.AsDict().at("name"s).AsString();
				const auto distances = elem.AsDict().at("road_distances").AsDict();
				for (const auto& [to, dist] : distances) {
					catalogue.SetDistance(from, to, dist.AsInt());
				}
			}
		}
	}

	bool JsonReader::IsStop(const json::Node& node) {
		if (!node.IsDict()) {
			return false;
		}
		const auto& stop = node.AsDict();
		if (stop.count("type"s) == 0 || stop.at("type"s) != "Stop"s) {
			return false;
		}
		if (stop.count("name"s) == 0 || !(stop.at("name"s).IsString())) {
			return false;
		}
		if (stop.count("latitude"s) == 0 || !(stop.at("latitude"s).IsDouble())) {
			return false;
		}
		if (stop.count("longitude"s) == 0 || !(stop.at("longitude"s).IsDouble())) {
			return false;
		}
		if (stop.count("road_distances"s) == 0 || (stop.at("longitude"s).IsDict())) {
			return false;
		}
		return true;
	}

	bool JsonReader::IsRoute(const json::Node& node) {
		if (!node.IsDict()) {
			return false;
		}
		const auto& bus = node.AsDict();
		if (bus.count("type"s) == 0 || bus.at("type"s) != "Bus"s) {
			return false;
		}
		if (bus.count("name"s) == 0 || !(bus.at("name"s).IsString())) {
			return false;
		}
		if (bus.count("is_roundtrip"s) == 0 || !(bus.at("is_roundtrip"s).IsBool())) {
			return false;
		}
		if (bus.count("stops"s) == 0 || !(bus.at("stops"s).IsArray())) {
			return false;
		}
		return true;
	}

	void JsonReader::AnsverRequests(const transport_catalogue::TransportCatalogue& catalogue,
		const renderer::RenderSettings& render_settings,
		std::ostream& out) const {
		if (data_.GetRoot().IsDict() && data_.GetRoot().AsDict().count("stat_requests"s) != 0) {
			auto& requests = data_.GetRoot().AsDict().at("stat_requests"s);
			if (requests.IsArray()) {
				json::Array ansvers = LoadAnsvers(requests.AsArray(),catalogue, render_settings);
				json::Print(json::Document(json::Node{ ansvers }), out);
			}
		}
	}

	json::Array JsonReader::LoadAnsvers(const json::Array& requests,
		const transport_catalogue::TransportCatalogue& catalogue,
		const renderer::RenderSettings& render_settings) const {
		json::Array ansver;
		for (const auto& request : requests) {
			if (IsRouteRequest(request)) {
				ansver.push_back(LoadRouteAnsver(request.AsDict(), catalogue));
			}
			else if (IsStopRequest(request)) {
				ansver.push_back(LoadStopAnswer(request.AsDict(), catalogue));
			}
			else if (IsMapRequest(request)) {
				ansver.push_back(LoadMapAnswer(request.AsDict(), catalogue, render_settings));
			}
		}
		return ansver;
	}

	json::Dict JsonReader::LoadRouteAnsver(const json::Dict& request, const transport_catalogue::TransportCatalogue& catalogue) {
		int id = request.at("id"s).AsInt();
		const auto& name = request.at("name"s).AsString();
		try {
			auto answer = catalogue.GetRouteInfo(name);
			return json::Builder{}.StartDict().
				Key("request_id").Value(id).
				Key("curvature").Value(answer.curvature).
				Key("route_length").Value(answer.route_length).
				Key("stop_count").Value(answer.num_of_stops).
				Key("unique_stop_count").Value(answer.num_of_unique_stops).
				EndDict().Build().AsDict();
		}
		catch(std::out_of_range&) {
			return ErrorMessage(id);
		}
	}

	json::Dict JsonReader::LoadStopAnswer(const json::Dict& request, const transport_catalogue::TransportCatalogue& catalogue) {
		int id = request.at("id"s).AsInt();
		const auto& name = request.at("name"s).AsString();
		try {
			auto buses_names = catalogue.GetBusesOnStop(name);
			json::Array buses;
			if (buses_names) {
				for (auto bus_name : buses_names.value().get()) {
					buses.push_back(std::string(bus_name));
				}
			}
			return json::Builder{}.StartDict().
				Key("request_id"s).Value(id).
				Key("buses"s).Value(buses).
				EndDict().Build().AsDict();
		}
		catch (std::out_of_range&) {
			return ErrorMessage(id);
		}
	}

	json::Dict JsonReader::LoadMapAnswer(const json::Dict& request, const transport_catalogue::TransportCatalogue& catalogue,
		const renderer::RenderSettings& render_settings) {
		int id = request.at("id"s).AsInt();
		std::ostringstream out;
		renderer::MapRenderer renderer;
		renderer.SetSettings(render_settings);
		renderer.RenderMap(catalogue).Render(out);
		return json::Builder{}.StartDict().
			Key("request_id"s).Value(id).
			Key("map"s).Value(out.str()).
			EndDict().Build().AsDict();
	}

	json::Dict JsonReader::ErrorMessage(int id) {
		return json::Builder{}.StartDict().
			Key("request_id"s).Value(id).
			Key("error_message"s).Value("not found"s).
			EndDict().Build().AsDict();
	}

	bool JsonReader::IsRouteRequest(const json::Node& node) {
		if (!node.IsDict()) {
			return false;
		}
		const auto& request = node.AsDict();
		if (request.count("type"s) == 0 || request.at("type"s) != "Bus"s) {
			return false;
		}
		if (request.count("id"s) == 0 || !(request.at("id"s).IsInt())) {
			return false;
		}
		if (request.count("name"s) == 0 || !(request.at("name"s).IsString())) {
			return false;
		}
		return true;
	}

	bool JsonReader::IsStopRequest(const json::Node& node) {
		if (!node.IsDict()) {
			return false;
		}
		const auto& request = node.AsDict();
		if (request.count("type"s) == 0 || request.at("type"s) != "Stop"s) {
			return false;
		}
		if (request.count("id"s) == 0 || !(request.at("id"s).IsInt())) {
			return false;
		}
		if (request.count("name"s) == 0 || !(request.at("name"s).IsString())) {
			return false;
		}
		return true;
	}

	bool JsonReader::IsMapRequest(const json::Node& node) {
		if (!node.IsDict()) {
			return false;
		}
		const auto& request = node.AsDict();
		if (request.count("type"s) == 0 || request.at("type"s) != "Map"s) {
			return false;
		}
		if (request.count("id"s) == 0 || !(request.at("id"s).IsInt())) {
			return false;
		}
		return true;
	}

	svg::Color JsonReader::ReadColor(const json::Node& node) {
		if (node.IsString()) {
			return node.AsString();
		}
		else if (node.IsArray() && node.AsArray().size() == 3) {
			auto result = svg::Rgb(static_cast<uint8_t>(node.AsArray().at(0).AsInt()),
				static_cast<uint8_t>(node.AsArray().at(1).AsInt()),
				static_cast<uint8_t>(node.AsArray().at(2).AsInt()));
			return result;
		}
		else if (node.IsArray() && node.AsArray().size() == 4) {
			auto result = svg::Rgba(static_cast<uint8_t>(node.AsArray().at(0).AsInt()),
				static_cast<uint8_t>(node.AsArray().at(1).AsInt()),
				static_cast<uint8_t>(node.AsArray().at(2).AsInt()),
				node.AsArray().at(3).AsDouble());
			return result;
		}
		else {
			return svg::NoneColor;
		}
	}

	svg::Point JsonReader::ReadOffset(const json::Array& node) {
		svg::Point result;
		if (node.size() > 1) {
			if (node.at(0).IsDouble()) {
				result.x = node.at(0).AsDouble();
			}
			if (node.at(1).IsDouble()) {
				result.y = node.at(1).AsDouble();
			}
		}
		return result;
	}
}//namespace json_reader

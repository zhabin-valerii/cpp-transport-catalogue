#include "map_renderer.h"

using namespace std::literals;

namespace renderer {
	namespace {
		inline const double EPSILON = 1e-6;
		bool IsZero(double value) {
			return std::abs(value) < EPSILON;
		}
	}
	void MapRenderer::SetSettings(const RenderSettings& settings) {
		settings_ = settings;
	}
	
	svg::Document MapRenderer::RenderMap(const transport_catalogue::TransportCatalogue& catalogue) {
		field_size_ = ComputeFieldSize(catalogue);
	
		const auto& stops = catalogue.GetStops();
		const auto& routes = catalogue.GetRoutes();
	
		Routes sorted_routes;
		Stops sorted_stops;
		for (auto& route : routes) {
			sorted_routes.insert(route);
		}
		for (auto& stop : stops) {
			sorted_stops.insert(stop);
		}
		const auto& buses_on_stops = catalogue.GetBusesOnStops();
		svg::Document res;
		RenderLines(res, sorted_routes);
		RenderRouteNames(res, sorted_routes);
		RenderStops(res, sorted_stops, buses_on_stops);
		RenderStopNames(res, sorted_stops, buses_on_stops);
		return res;
	}
	
	std::pair<geo::Coordinates, geo::Coordinates>
		MapRenderer::ComputeFieldSize(const transport_catalogue::TransportCatalogue& catalogue) const {
		geo::Coordinates min{90.0, 180.0};
		geo::Coordinates max{-90.0, -180.0};
		for (const auto& stop : catalogue.GetStops()) {
			if (catalogue.GetBusesOnStop(std::string(stop.first))) {
				const auto& coordinates = stop.second->coordinates_;
				if (coordinates.lat < min.lat) {
					min.lat = coordinates.lat;
				}
				if (coordinates.lat > max.lat) {
					max.lat = coordinates.lat;
				}
				if (coordinates.lng < min.lng) {
					min.lng = coordinates.lng;
				}
				if (coordinates.lng > max.lng) {
					max.lng = coordinates.lng;
				}
			}
		}
		return {min, max};
	}
	
	svg::Point MapRenderer::GetPoint(geo::Coordinates coordinate) const {
		double zoom_coef = 0.0;
		double field_width = field_size_.second.lng - field_size_.first.lng;
		double field_height = field_size_.second.lat - field_size_.first.lat;
	
		if (IsZero(field_width) && IsZero(field_height)) {
			zoom_coef = 0;
		}
		else if (IsZero(field_width)) {
			zoom_coef = (settings_.size.y - 2 * settings_.padding) / field_height;
		}
		else if (IsZero(field_height)) {
			zoom_coef = (settings_.size.x - 2 * settings_.padding) / field_width;
		}
		else {
			zoom_coef = std::min((settings_.size.y - 2 * settings_.padding) / field_height,
				(settings_.size.x - 2 * settings_.padding) / field_width);
		}
		svg::Point res;
		res.x = (coordinate.lng - field_size_.first.lng) * zoom_coef + settings_.padding;
		res.y = (field_size_.second.lat - coordinate.lat) * zoom_coef + settings_.padding;
		return res;
	}
	
	void MapRenderer::RenderLines(svg::Document& doc, const Routes& routes) const {
		auto max_color_count = settings_.color_palette.size();
		size_t color_index = 0;
		for (const auto& route : routes) {
			if (route.second->stops_.size() > 0) {
				svg::Polyline line;
				line.SetStrokeColor(settings_.color_palette.at(color_index % max_color_count)).
					SetFillColor(svg::NoneColor).SetStrokeWidth(settings_.line_width).
					SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
				for (auto iter = route.second->stops_.begin(); iter < route.second->stops_.end(); ++iter) {
					line.AddPoint(GetPoint((*iter)->coordinates_));
				}
				if (route.second->type_route_ == domain::RouteType::LINEAR) {
					for (auto iter = std::next(route.second->stops_.rbegin()); iter < route.second->stops_.rend(); ++iter) {
						line.AddPoint(GetPoint((*iter)->coordinates_));
					}
				}
				doc.Add(line);
				++color_index;
			}
		}
	}

	void MapRenderer::RenderRouteNames(svg::Document& doc, const Routes& routes) const {
		auto max_color_count = settings_.color_palette.size();
		size_t color_index = 0;
		for (auto& route : routes) {
			if (route.second->stops_.size() > 0) {
				svg::Text text;
				svg::Text underlayer_text;
				text.SetData(std::string(route.first)).
					SetPosition(GetPoint(route.second->stops_.front()->coordinates_)).
					SetOffset(settings_.bus_label_offset).
					SetFontSize(static_cast<std::uint32_t>(settings_.bus_label_font_size)).
					SetFontFamily("Verdana"s).SetFontWeight("bold"s);
				underlayer_text = text;
				text.SetFillColor(settings_.color_palette.at(color_index % max_color_count));
				underlayer_text.SetFillColor(settings_.underlayer_color).
					SetStrokeColor(settings_.underlayer_color).
					SetStrokeWidth(settings_.underlayer_width).
					SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
				doc.Add(underlayer_text);
				doc.Add(text);
				if (route.second->type_route_ == domain::RouteType::LINEAR &&
					route.second->stops_.back() != route.second->stops_.front()) {
					text.SetPosition(GetPoint(route.second->stops_.back()->coordinates_));
					underlayer_text.SetPosition(GetPoint(route.second->stops_.back()->coordinates_));
					doc.Add(underlayer_text);
					doc.Add(text);
				}
				++color_index;
			}
		}
	}
	void MapRenderer::RenderStops(svg::Document& doc, const Stops& stops, const BusesOnStops& buses_on_stops) const {
		for (const auto& stop : stops) {
			if (buses_on_stops.count(stop.first) != 0) {
				svg::Circle circle;
				circle.SetCenter(GetPoint(stop.second->coordinates_)).
					SetRadius(settings_.stop_radius).SetFillColor("white"s);
				doc.Add(circle);
			}
		}
	}

	void MapRenderer::RenderStopNames(svg::Document& doc, const Stops& stops, const BusesOnStops& buses_on_stops) const {
		for (const auto& stop : stops) {
			if (buses_on_stops.count(stop.first) != 0) {
				svg::Text text;
				svg::Text underlayer_text;
				text.SetData(std::string(stop.first)).SetPosition(GetPoint(stop.second->coordinates_)).
					SetOffset(settings_.stop_label_offset).
					SetFontSize(static_cast<uint32_t>(settings_.stop_label_font_size)).
					SetFontFamily("Verdana"s);
				underlayer_text = text;
				text.SetFillColor("black"s);
				underlayer_text.SetFillColor(settings_.underlayer_color).
					SetStrokeColor(settings_.underlayer_color).SetStrokeWidth(settings_.underlayer_width).
					SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
				doc.Add(underlayer_text);
				doc.Add(text);
			}
		}
	}
}//namespace renderer
#pragma once

#include <sstream>
#include <stdexcept>
#include <string>

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_builder.h"

namespace json_reader {
	class JsonReader {
	public:
		JsonReader(std::istream& input);

		void LoadData(transport_catalogue::TransportCatalogue& catalogue) const;

		std::optional<renderer::RenderSettings> LoadRenderSettings() const;

		void AnsverRequests(const transport_catalogue::TransportCatalogue& catalogue,
			const renderer::RenderSettings& render_settings,
			std::ostream& out) const;
	private:
		renderer::RenderSettings LoadSettings(const json::Dict& data) const;
		static void LoadStops(const json::Array& data, transport_catalogue::TransportCatalogue& catalogue);
		static void LoadRoutes(const json::Array& data, transport_catalogue::TransportCatalogue& catalogue);
		static void LoadDistances(const json::Array& data, transport_catalogue::TransportCatalogue& catalogue);

		static bool IsStop(const json::Node& node);
		static bool IsRoute(const json::Node& node);
		static bool IsRouteRequest(const json::Node& node);
		static bool IsStopRequest(const json::Node& node);
		static bool IsMapRequest(const json::Node& node);

		json::Array LoadAnsvers(const json::Array& requests,
			const transport_catalogue::TransportCatalogue& catalogue,
			const renderer::RenderSettings& render_settings) const;

		static json::Dict LoadRouteAnsver(const json::Dict& request, const transport_catalogue::TransportCatalogue& catalogue);
		static json::Dict LoadStopAnswer(const json::Dict& request, const transport_catalogue::TransportCatalogue& catalogue);
		static json::Dict LoadMapAnswer(const json::Dict& request, const transport_catalogue::TransportCatalogue& catalogue,
			const renderer::RenderSettings& render_settings);
		static json::Dict ErrorMessage(int id);

		static svg::Color ReadColor(const json::Node& node);
		static svg::Point ReadOffset(const json::Array& node);

		json::Document data_;
	};
}//namespace json_reader

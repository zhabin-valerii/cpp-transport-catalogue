#pragma once
#include "json_reader.h"
#include "map_renderer.h"

#include <iostream>

namespace transport_catalogue {
    class RequestHandler {
    public:
        explicit RequestHandler(TransportCatalogue& catalogue) :
            catalogue_(catalogue) {}

        domain::RouteInfo GetRouteInfo(const std::string& bus_name) const;
        std::optional<std::reference_wrapper<const std::set<std::string_view>>>
            GetBusesOnStop(const std::string& stop_name) const;

        void ProcessRequest(std::istream& input, std::ostream& out);

        svg::Document RenderMap() const;

        void SetRenderSettings(const renderer::RenderSettings& render_settings);

    private:
        TransportCatalogue& catalogue_;
        std::optional<renderer::RenderSettings> render_settings_;
    };
}//namespace request_handler

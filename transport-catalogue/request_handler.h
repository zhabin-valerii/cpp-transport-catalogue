#pragma once
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "transport_catalogue.h"

#include <iostream>
#include <optional>

namespace transport_catalogue {
    class RequestHandler {
    public:

        using Route = transport_router::TransportRouter::TransportRoute;
        using RoutingSettings = transport_router::TransportRouter::RoutingSettings;

        explicit RequestHandler(TransportCatalogue& catalogue) :
            catalogue_(catalogue) {}

        domain::RouteInfo GetRouteInfo(const std::string& bus_name) const;

        const std::set<std::string_view>
            GetBusesOnStop(const std::string& stop_name) const;

        void ProcessRequest(std::istream& input, std::ostream& out);

        svg::Document RenderMap() const;

        void SetRenderSettings(const renderer::RenderSettings& render_settings);

        std::optional<Route> BuildRoute(const std::string& from, const std::string& to);

        bool ReInitRouter();

        void SetRouterSettings(const RoutingSettings& routing_settings);

    private:
        bool InitRouter();

        TransportCatalogue& catalogue_;
        std::optional<renderer::RenderSettings> render_settings_;

        std::unique_ptr<transport_router::TransportRouter> router_;
        std::optional<RoutingSettings> routing_settings_;
    };
}//namespace request_handler

#include "request_handler.h"

#include <string>

using namespace std::literals;

namespace transport_catalogue {

	using namespace json_reader;

	domain::RouteInfo RequestHandler::GetRouteInfo(const std::string& bus_name) const{
		return catalogue_.GetRouteInfo(bus_name);
	}

	const std::set<std::string_view>
		RequestHandler::GetBusesOnStop(const std::string& stop_name) const {
		return catalogue_.GetBusesOnStop(stop_name);
	}

	void RequestHandler::ProcessRequest(std::istream& input, std::ostream& out) {
		json_reader::JsonReader json(input);
		json.LoadData(catalogue_);
		render_settings_ = json.LoadRenderSettings();
		routing_settings_ = json.LoadRoutingSettings();
		if (!InitRouter()) {
			std::cerr << "Can't init Transport Router"s << std::endl;
			return;
		}
		json.AnsverRequests(catalogue_, render_settings_.value_or(renderer::RenderSettings{}), *router_, out);
	}

	svg::Document RequestHandler::RenderMap() const {
		if (render_settings_) {
			renderer::MapRenderer renderer;
			renderer.SetSettings(*render_settings_);
			return renderer.RenderMap(catalogue_);
		}
		else {
			return {};
		}
	}

	void RequestHandler::SetRenderSettings(const renderer::RenderSettings& render_settings) {
		render_settings_ = render_settings;
	}

	void RequestHandler::SetRouterSettings(const RoutingSettings& routing_settings) {
		routing_settings_ = routing_settings;
	}

	std::optional<RequestHandler::Route>
		RequestHandler::BuildRoute(const std::string& from, const std::string& to) {
		if (!InitRouter()) {
			return std::nullopt;
		}
		else {
			return router_->BuildRoute(from, to);
		}
	}

	bool RequestHandler::ReInitRouter() {
		if (routing_settings_) {
			router_ = std::make_unique<transport_router::TransportRouter>(catalogue_, routing_settings_.value());
			return true;
		}
		else {
			return false;
		}
	}

	bool RequestHandler::InitRouter() {
		if (!router_) {
			return ReInitRouter();
		}
		return true;
	}

}//namespace request_handler
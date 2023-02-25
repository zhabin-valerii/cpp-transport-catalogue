#include "request_handler.h"

#include <string>

using namespace std::literals;

namespace transport_catalogue {

	using namespace json_reader;

	domain::RouteInfo RequestHandler::GetRouteInfo(const std::string& bus_name) const{
		return catalogue_.GetRouteInfo(bus_name);
	}

	std::optional<std::reference_wrapper<const std::set<std::string_view>>>
		RequestHandler::GetBusesOnStop(const std::string& stop_name) const {
		return catalogue_.GetBusesOnStop(stop_name);
	}

	void RequestHandler::ProcessRequest(std::istream& input, std::ostream& out) {
		json_reader::JsonReader json(input);
		json.LoadData(catalogue_);
		auto render_setings = json.LoadRenderSettings();
		json.AnsverRequests(catalogue_, render_setings.value_or(renderer::RenderSettings{}), out);
	}

	svg::Document RequestHandler::RenderMap() const {
		if (render_settings_) {
			renderer::MapRenderer renderer;
			renderer.SetSettings(render_settings_.value());
			return renderer.RenderMap(catalogue_);
		}
		else {
			return {};
		}
	}

	void RequestHandler::SetRenderSettings(const renderer::RenderSettings& render_settings) {
		render_settings_ = render_settings;
	}

}//namespace request_handler
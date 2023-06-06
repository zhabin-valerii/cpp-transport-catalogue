#include "json_reader.h"

namespace tr_cat {
    namespace interface {
        using namespace std;

        void JsonReader::ReadDocument() {
            document_ = json::Load(input_);
        }

        void JsonReader::ParseDocument() {
            if (document_.GetRoot().IsNull()) {
                return;
            }
            auto& it = document_.GetRoot().AsMap();
            if (it.count ("base_requests"s)){
                ParseBase(it.at("base_requests"s));
            }
            if (it.count("stat_requests"s) && (it.at("stat_requests"s).IsArray())) {
                ParseStats(it.at("stat_requests"s));
            }
            if (it.count("render_settings"s)) {
                ParseRenderSettings(it.at("render_settings"s));
            }

            if (it.count("routing_settings"s)) {
                ParseRoutingSettings(it.at("routing_settings"s));
            }

            if (it.count ("serialization_settings"s)) {
                serializator_.SetPathToSerialize(it.at("serialization_settings"s)
                                    .AsMap().at ("file"s).AsString());
            }

        }
//-----------------------Parse Base------------------------------------
        void JsonReader::ParseBase(json::Node& base_node) {

            auto& base = base_node.AsArray();
            for (auto& element_node: base) {
                auto& element = element_node.AsMap();
                if (element.at("type"s).AsString() == "Stop"s) {

                    stops_.push_back({});

                    stops_.back().name = element.at("name"s).AsString();
                    stops_.back().coordinates.lat = element.at("latitude"s).AsDouble();
                    stops_.back().coordinates.lng = element.at("longitude"s).AsDouble();

                    if (element.count("road_distances"s)) {
                        auto& map_distances = element.at("road_distances"s).AsMap();
                        for (auto& [name, value] : map_distances) {
                            distances_[stops_.back().name].push_back({name, value.AsInt()});
                        }
                    }

                } else if (element.at("type"s).AsString() == "Bus"s) {
                    
                    buses_.push_back({});

                    buses_.back().name = element.at("name"s).AsString();
                    buses_.back().is_ring = element.at("is_roundtrip"s).AsBool();

                    auto& it = element.at("stops"s).AsArray();
                    buses_.back().stops.reserve(it.size());
                    for (json::Node& elem : it) {
                        buses_.back().stops.push_back(elem.AsString());
                    }

                } else {
                    throw invalid_argument ("Unknown type"s);
                }

            }
        }
//------------------------------Parse Stats---------------------------
        void JsonReader::ParseStats(json::Node& stats_node) {

            auto& stats = stats_node.AsArray();
            stats_.reserve(stats.size());

            for (auto& element_node : stats) {
                auto& element = element_node.AsMap();
                const string& type = element.at("type"s).AsString();
                if ((type == "Bus"s) || (type == "Stop"s)) {
                    stats_.push_back({element.at("id"s).AsInt(), 
                                      type, 
                                      element.at("name"s).AsString(),
                                      "", ""});
                } else if (type == "Map"s) {
                    stats_.push_back({element.at("id"s).AsInt(), 
                                      type, "", "", ""sv});
                } else if (type == "Route"s) {
                    stats_.push_back({element.at("id"s).AsInt(), 
                                      type, "",
                                      element.at("from"s).AsString(),
                                      element.at("to"s).AsString()});
                } else {
                    throw invalid_argument("Unknown type"s);
                }
            }
        }

//-----------------------------Parse Render Settings--------------------------------------
        void JsonReader::ParseRenderSettings(json::Node& settings_node) {

            auto& settings = settings_node.AsMap();
            render::RenderSettings render_settings;

            if (settings.count("width"s)) {
                render_settings.width = settings.at("width"s).AsDouble();
            }
            if (settings.count("height"s)) {
                render_settings.height = settings.at("height"s).AsDouble();
            }
            if (settings.count("padding"s)) {
                render_settings.padding = settings.at("padding"s).AsDouble();
            }
            if (settings.count("line_width"s)) {
                render_settings.line_width = settings.at("line_width"s).AsDouble();
            }
            if (settings.count("stop_radius"s)) {
                render_settings.stop_radius = settings.at("stop_radius"s).AsDouble();
            }
            if (settings.count("bus_label_font_size"s)) {
                render_settings.bus_label_font_size = settings.at("bus_label_font_size"s).AsDouble();
            }
            if (settings.count("bus_label_offset"s)) {
                auto it = settings.at("bus_label_offset"s).AsArray();
                render_settings.bus_label_offset = {it[0].AsDouble(), it[1].AsDouble()};
            }
            if (settings.count("stop_label_font_size"s)) {
                render_settings.stop_label_font_size = settings.at("stop_label_font_size"s).AsDouble();
            }
            if (settings.count("stop_label_offset"s)) {
                auto it = settings.at("stop_label_offset"s).AsArray();
                render_settings.stop_label_offset = {it[0].AsDouble(), it[1].AsDouble()};
            }

            auto get_color = [&] (json::Node& key, svg::Color* field) {
                if (key.IsString()) {
                    *field = key.AsString();
                } else if (key.AsArray().size() == 3) {
                    *field = svg::Rgb({key.AsArray()[0].AsInt(), key.AsArray()[1].AsInt(), key.AsArray()[2].AsInt()});
                } else if (key.AsArray().size() == 4) {
                    *field = svg::Rgba({key.AsArray()[0].AsInt(), key.AsArray()[1].AsInt(), key.AsArray()[2].AsInt(), 
                                                                                            key.AsArray()[3].AsDouble()});
                }
            };
            
            if (settings.count("underlayer_color"s)) {
                get_color(settings.at("underlayer_color"s), &render_settings.underlayer_color);
            }
            if (settings.count("underlayer_width"s)) {
                render_settings.underlayer_width = settings.at("underlayer_width"s).AsDouble();
            }

            if (settings.count("color_palette"s)) {
                auto& array = settings.at("color_palette"s).AsArray();
                render_settings.color_palette.reserve(array.size());
                for (auto& node : array) {
                    render_settings.color_palette.push_back({});
                    get_color(node, &render_settings.color_palette.back());
                }
            }
            renderer_.SetRenderSettings(move(render_settings));
        }

//------------------------------Parse Routing Settings-----------------------------
        void JsonReader::ParseRoutingSettings (json::Node& routing_settings) {

            auto& settings = routing_settings.AsMap();
            int velocity = settings.at("bus_velocity"s).AsInt();
            int wait_time = settings.at("bus_wait_time"s).AsInt();
            if (velocity < 0 || wait_time < 0 || velocity > 1000 || wait_time > 1000) {
                throw invalid_argument("invalid routing_settings: 0 <= velocity, wait_time <= 1000"s);
            }
            transport_router_.SetSettings({static_cast<uint32_t>(wait_time),
                                           static_cast<uint32_t>(velocity)});

        }

//--------------------------------------Print--------------------------------------
        void JsonReader::PrepareToPrint() {
            json::Builder builder;
            builder.StartArray();
            for (auto& answer : answers_) {
                builder.Value(visit(CreateNode{renderer_, transport_router_}, answer));
            }
            builder.EndArray();
            document_answers_ = builder.Build();
        }  

        void JsonReader::PrintAnswers() {
            PrepareToPrint();
            json::Print(document_answers_, output_);
        }

//-----------------------------CreateNode-----------------------------------

//если ошибка, передаём только id запроса, текст ошибки одинаковый, для любого неудачного запроса
        json::Node JsonReader::CreateNode::operator() (int value) {
            json::Builder builder;
            return builder.StartDict()  .Key("request_id"s).Value(value)
                                        .Key("error_message"s).Value("not found"s).EndDict().Build();
        }

        json::Node JsonReader::CreateNode::operator() (StopOutput& value) {
            json::Builder builder;
            builder.StartDict().Key("request_id"s).Value(value.id)
                                 .Key("buses"s).StartArray();
            for (string_view bus : value.stop->buses) {
                builder.Value(static_cast<string>(bus));
            }
            return builder.EndArray().EndDict().Build();
        }

        json::Node JsonReader::CreateNode::operator() (BusOutput& value) {
            json::Builder builder;
            return builder.StartDict()  .Key("request_id"s).Value(value.id)
                                        .Key("curvature"s).Value(value.bus->curvature)
                                        .Key("route_length"s).Value(static_cast<double>(value.bus->distance))
                                        .Key("stop_count"s).Value(static_cast<int>(value.bus->stops.size()))
                                        .Key("unique_stop_count"s).Value(value.bus->unique_stops).EndDict().Build();
        }

        json::Node JsonReader::CreateNode::operator() (MapOutput& value) {
            json::Builder builder;
            ostringstream output;
            renderer_.Render(output);

            return builder.StartDict().Key("request_id"s).Value(value.id)
                                      .Key("map"s).Value(output.str()).EndDict().Build();
        }

        json::Node JsonReader::CreateNode::operator() (RouteOutput& value) {

            std::optional<router::CompletedRoute> result = transport_router_.ComputeRoute(value.from->vertex_id, 
                                                                   value.to->vertex_id);
            json::Builder builder;
            if (!result) {
                return builder.StartDict().Key("request_id"s).Value(value.id)
                                          .Key("error_message"s).Value("not found"s).EndDict().Build();
            }

            builder.StartDict().Key("request_id"s).Value(value.id)
                               .Key("total_time"s).Value(result->total_time)
                               .Key("items"s).StartArray();

            for (const router::CompletedRoute::Line& line : result->route) {
                builder.StartDict() .Key("stop_name"s).Value(line.stop->name)
                                    .Key("time"s).Value(line.wait_time)
                                    .Key("type"s).Value("Wait"s).EndDict()
                       .StartDict() .Key("bus"s).Value(line.bus->name)
                                    .Key("span_count"s).Value(static_cast<int>(line.count_stops))
                                    .Key("time"s).Value(line.run_time)
                                    .Key("type").Value("Bus"s).EndDict();
            }
            builder.EndArray().EndDict();
            return builder.Build();
        }

        bool NodeCompare(json::Node lhs, json::Node rhs) {
            if (lhs.IsArray() && rhs.IsArray()) {
                for (size_t i = 0; i < lhs.AsArray().size(); ++i) {
                    if (!NodeCompare(lhs.AsArray()[i], rhs.AsArray()[i])) {
                        cerr << "Arrays elements not equal. Index: "s << i << endl;
                        return false;
                    }
                }
                return true;
            }
            if (lhs.IsMap() && rhs.IsMap()) {
                for (auto& [key, value] : lhs.AsMap()) {
                    if (!NodeCompare(value, rhs.AsMap().at(key))) {
                        cerr << "Maps elements not equal. Key: "s << key << endl;
                        return false;
                    }
                }
                return true;
            }
            if (lhs.IsString() && rhs.IsString()) {
                string lhs_s = lhs.AsString();
                string rhs_s = rhs.AsString();
                for (auto it = lhs_s.find(' '); it != string::npos; it = lhs_s.find(' ')) {
                    lhs_s.erase(it, 1);
                }
                for (auto it = lhs_s.find('\n'); it != string::npos; it = lhs_s.find('\n')) {
                    lhs_s.erase(it, 1);
                }
                for (auto it = rhs_s.find(' '); it != string::npos; it = rhs_s.find(' ')) {
                    rhs_s.erase(it, 1);
                }
                for (auto it = rhs_s.find('\n'); it != string::npos; it = rhs_s.find('\n')) {
                    rhs_s.erase(it, 1);
                }
                for (size_t i = 0; i < lhs_s.size(); ++i) {
                    if (lhs_s[i] != rhs_s[i]) {
                        int i_min = min(i - 10, size_t(0));
                        int i_max = max(i + 10, lhs_s.length());
                        i_max = max(i + 10, rhs_s.length());
                        cerr << "IN OUTPUT:  "sv << lhs_s.substr(i_min, i_max - i_min) << endl;
                        cerr << "IN_EXAMPLE: "sv << rhs_s.substr(i_min, i_max - i_min) << endl;
                        return false;
                    }
                }
                return true;
            }
            if (lhs.IsPureDouble() && rhs.IsPureDouble()) {
                const double inaccuracy = 1e-5;
                return std::abs(lhs.AsDouble() - rhs.AsDouble()) < inaccuracy;
            }
            return lhs == rhs;
        }


        bool JsonReader::TestingFilesOutput(std::string filename_lhs, std::string filename_rhs) {
            
            json::Document lhs, rhs;
            {
                std::ifstream inf {filename_lhs};
                lhs = json::Load(inf);
            }

            {
                std::ifstream inf {filename_rhs};
                rhs = json::Load(inf);
            }
            return NodeCompare(lhs.GetRoot(), rhs.GetRoot());
        }

    }//interface
}//tr_cat

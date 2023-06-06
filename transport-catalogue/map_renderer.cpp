#include "map_renderer.h"

namespace tr_cat {
namespace render {

using namespace std;
using namespace aggregations;
using namespace svg;

enum class ColorVariant {
    STRING,
    RGB,
    RGBA
};

void MapRenderer::Render(ostream& out) {
    Document doc_to_render;
    auto coords = CollectCoordinates();
    SphereProjector project (coords.begin(), coords.end(), settings_.width, settings_.height, settings_.padding);

    set<string_view> stops_in_buses = RenderBuses(project, doc_to_render);
    RenderStops(project, doc_to_render, stops_in_buses);

    doc_to_render.Render(out);
}

transport_catalog_serialize::RenderSettings MapRenderer::Serialize() const {
    struct ColorGetter {

        optional<transport_catalog_serialize::Color> operator()(monostate) {return nullopt;}

        optional<transport_catalog_serialize::Color> operator()(const string& name) {
            transport_catalog_serialize::Color result_color;
            result_color.set_str (name);
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
        *settings_to_out.mutable_color_palette(settings_to_out.color_palette_size()-1) =
                                                            *visit(ColorGetter{}, color);
    }
    return settings_to_out;
}

bool MapRenderer::Deserialize(transport_catalog_serialize::RenderSettings &settings_in) {
    struct ColorGetter {
        Color operator() (transport_catalog_serialize::Color& color) {
            if (!color.str().empty()) {
                return color.str();
            }
            if (color.has_rgb()) {
                transport_catalog_serialize::Rgb rgb = *color.mutable_rgb();
                return Rgb{rgb.r(), rgb.g(), rgb.b()};
            }
            if (color.has_rgba()) {
                transport_catalog_serialize::Rgba rgba = *color.mutable_rgba();
                return Rgba{rgba.r(), rgba.g(), rgba.b(), rgba.a()};
            }
            return monostate();
        }
    };
    settings_.width = settings_in.width();
    settings_.height = settings_in.height();
    settings_.padding = settings_in.padding();
    settings_.line_width = settings_in.line_width();
    settings_.stop_radius = settings_in.stop_radius();
    settings_.bus_label_font_size = settings_in.bus_label_font_size();
    settings_.bus_label_offset = {settings_in.bus_label_offset_x(),
                                  settings_in.bus_label_offset_y()};
    settings_.stop_label_font_size = settings_in.stop_label_font_size();
    settings_.stop_label_offset = {settings_in.stop_label_offset_x(),
                                   settings_in.stop_label_offset_y()};
    settings_.underlayer_color = ColorGetter()(*settings_in.mutable_underlayer_color());
    settings_.underlayer_width = settings_in.underlayer_width();

    for (int i = 0; i < settings_in.color_palette_size(); ++i) {
        settings_.color_palette.push_back(ColorGetter()(*settings_in.mutable_color_palette(i)));
    }
    return true;
}

unordered_set<geo::Coordinates, CoordinatesHasher> MapRenderer::CollectCoordinates () const {
    unordered_set<geo::Coordinates, CoordinatesHasher> result;
    for (string_view bus_name : catalog_) {
        optional<const Bus*> bus_finded = catalog_.GetBusInfo(bus_name);
        if (!bus_finded) {
            throw logic_error("Catalog error. No Bus info"s + static_cast<string> (bus_name));
        }
        for (const Stop* stop : (*bus_finded)->stops) {
            result.insert(stop->coordinates);
        }
    }
    return result;
}

pair<unique_ptr<Text>, unique_ptr<Text>> MapRenderer::AddBusLabels(SphereProjector& project, int index_color,
                                                                const Stop* stop, string_view name) {
    Text bus_name_underlabel, bus_name_label;
    bus_name_underlabel.SetData(static_cast<string>(name)).SetPosition(project(stop->coordinates))
                            .SetOffset(settings_.bus_label_offset).SetFontSize(settings_.bus_label_font_size)
                            .SetFontFamily("Verdana"s).SetFontWeight("bold"s).SetStrokeWidth(settings_.underlayer_width)
                            .SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color)
                            .SetStrokeLineCap(StrokeLineCap::ROUND).SetStrokeLineJoin(StrokeLineJoin::ROUND);

    bus_name_label.SetData(static_cast<string>(name)).SetPosition(project(stop->coordinates))
                    .SetOffset(settings_.bus_label_offset).SetFontSize(settings_.bus_label_font_size)
                    .SetFontFamily("Verdana"s).SetFontWeight("bold"s).SetFillColor(settings_.color_palette[index_color]);

    return {make_unique<Text>(bus_name_underlabel), make_unique<Text>(bus_name_label)};
}

set<string_view> MapRenderer::RenderBuses(SphereProjector& project, Document& doc_to_render) {
    int index_color = 0;
    int color_counts = settings_.color_palette.size();
    vector<unique_ptr<Object>> bus_lines;
    vector<unique_ptr<Object>> bus_labels;
    bus_lines.reserve(catalog_.size());
    bus_labels.reserve(bus_lines.capacity()*4);
    set<string_view> stops_in_buses;

    for (string_view bus_name : catalog_) {

        index_color %= color_counts;

        const Bus* bus = *catalog_.GetBusInfo(bus_name);
        if (bus->stops.empty()) {
            continue;
        }

        unique_ptr<Polyline> line = make_unique<Polyline>(Polyline().SetFillColor("none"s)
            .SetStrokeColor(settings_.color_palette[index_color]).SetStrokeWidth(settings_.line_width)
            .SetStrokeLineCap(StrokeLineCap::ROUND).SetStrokeLineJoin(StrokeLineJoin::ROUND));

        unique_ptr<Text> bus_label_start, bus_underlabel_start,
                         bus_label_finish, bus_underlabel_finish;
        tie(bus_underlabel_start, bus_label_start) =  AddBusLabels(project, index_color, bus->stops.front(), bus_name);
        if (!bus->is_ring && (bus->stops.front() != bus->stops[bus->stops.size()/2])) {
            tie(bus_underlabel_finish, bus_label_finish) = AddBusLabels(project,
                                                                index_color, bus->stops[bus->stops.size()/2], bus_name);
        }

        for (const Stop* stop : bus->stops) {
            line->AddPoint(project(stop->coordinates));
            stops_in_buses.insert(stop->name);
        }

        bus_lines.push_back(move(line));
        bus_labels.push_back(move(bus_underlabel_start));
        bus_labels.push_back(move(bus_label_start));
        if (bus_label_finish && bus_underlabel_finish) {
            bus_labels.push_back(move(bus_underlabel_finish));
            bus_labels.push_back(move(bus_label_finish));
        }
        ++index_color;
    }

    for (auto& pointer : bus_lines) {
        doc_to_render.AddPtr(move(pointer));
    }
    for (auto& pointer : bus_labels) {
        doc_to_render.AddPtr(move(pointer));
    }
    return stops_in_buses;
}

void MapRenderer::RenderStops(SphereProjector& project, svg::Document& doc_to_render, set<string_view> stops_in_buses) {
    vector<unique_ptr<Circle>> stop_points;
    vector<unique_ptr<Text>> stop_labels;
    stop_points.reserve(stops_in_buses.size());
    stop_labels.reserve(stops_in_buses.size()*2);

    for (string_view stop_name : stops_in_buses) {
        const Stop* stop = *(catalog_.GetStopInfo(stop_name));
        Point coords = project(stop->coordinates);

        unique_ptr<Circle> stop_point = make_unique<Circle>(Circle().SetCenter(coords)
                                                                    .SetRadius(settings_.stop_radius)
                                                                    .SetFillColor("white"s));

        unique_ptr<Text> stop_underlabel = make_unique<Text>(Text().SetPosition(coords)
                                        .SetData(static_cast<string>(stop_name)).SetOffset(settings_.stop_label_offset)
                                        .SetFontSize(settings_.stop_label_font_size).SetFontFamily("Verdana"s)
                                        .SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color)
                                        .SetStrokeWidth(settings_.underlayer_width).SetStrokeLineCap(StrokeLineCap::ROUND)
                                        .SetStrokeLineJoin(StrokeLineJoin::ROUND));
        unique_ptr<Text> stop_label = make_unique<Text>(Text().SetPosition(coords)
                                        .SetData(static_cast<string>(stop_name)).SetOffset(settings_.stop_label_offset)
                                        .SetFontSize(settings_.stop_label_font_size).SetFontFamily("Verdana"s)
                                        .SetFillColor("black"s));

        stop_points.push_back(move(stop_point));
        stop_labels.push_back(move(stop_underlabel));
        stop_labels.push_back(move(stop_label));
    }

    for (auto& pointer : stop_points) {
        doc_to_render.AddPtr(move(pointer));
    }
    for (auto& pointer : stop_labels) {
        doc_to_render.AddPtr(move(pointer));
    }
}
}//render
}//tr_cat

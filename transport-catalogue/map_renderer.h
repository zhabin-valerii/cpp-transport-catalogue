#pragma once

#include "svg.h"
#include "transport_catalogue.h"
#include "geo.h"

#include <ostream>

namespace tr_cat {
namespace render {

inline bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(tr_cat::geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

struct RenderSettings {
    double width = 0;
    double height = 0;

    double padding = 0;

    double line_width = 0;
    double stop_radius = 0;

    double bus_label_font_size = 0;
    svg::Point bus_label_offset {0, 0};

    double stop_label_font_size = 0;
    svg::Point stop_label_offset {0, 0};

    svg::Color underlayer_color;
    double underlayer_width = 0;
    std::vector<svg::Color> color_palette;

};

struct CoordinatesHasher {
    size_t operator() (const geo::Coordinates& coords) const {
        return std::hash<double>{}(coords.lat) + std::hash<double>{}(coords.lng)*37;
    }
};

class MapRenderer {
public:
    MapRenderer() = delete;
    MapRenderer(const aggregations::TransportCatalogue& catalog)
    :catalog_(catalog) {}

    transport_catalog_serialize::RenderSettings Serialize() const;
    bool Deserialize (transport_catalog_serialize::RenderSettings& settings);

    void SetRenderSettings(RenderSettings&& settings) {settings_ = settings;}
    void Render(std::ostream& out);

private:
    const aggregations::TransportCatalogue& catalog_;
    RenderSettings settings_;
    std::unordered_set<geo::Coordinates, CoordinatesHasher> CollectCoordinates () const;
    std::pair<std::unique_ptr<svg::Text>, std::unique_ptr<svg::Text>> AddBusLabels(SphereProjector& project,
                                                    int index_color, const Stop* stop, std::string_view name);
    std::set<std::string_view> RenderBuses(SphereProjector& project, svg::Document& doc_to_render);
    void RenderStops(SphereProjector& project, svg::Document& doc_to_render, std::set<std::string_view> stops_in_buses);
};
}//render
}//tr_cat

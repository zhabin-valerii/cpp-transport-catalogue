#define _USE_MATH_DEFINES
#include "svg.h"

#include <cmath>

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    void OverloadedColor::operator()(std::monostate) const {
        out_ << NoneColor;
    }
    
    void OverloadedColor::operator()(const std::string& color) const {
        out_ << color;
    }
    
    void OverloadedColor::operator()(Rgb color) const {
        out_ << "rgb("s
            << std::to_string(color.red) << ","s
            << std::to_string(color.green) << ","s
            << std::to_string(color.blue) << ")"s;
    }
    
    void OverloadedColor::operator()(Rgba color) const {
        out_ << "rgba("s
            << std::to_string(color.red) << ","s
            << std::to_string(color.green) << ","s
            << std::to_string(color.blue) << ","s;
        out_ << color.opacity << ")"s;
    }

    std::ostream& operator<<(std::ostream& out, const Color& color) {
        std::visit(OverloadedColor{ out }, color);
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineCap cap) {
        switch (cap){
            case StrokeLineCap::BUTT: return out << "butt"s;
            case StrokeLineCap::ROUND: return out << "round"s;
            case StrokeLineCap::SQUARE: return out << "square"s;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin join) {
        switch (join) {
            case StrokeLineJoin::ARCS: return out << "arcs"s;
            case StrokeLineJoin::BEVEL: return out << "bevel"s;
            case StrokeLineJoin::MITER: return out << "miter"s;
            case StrokeLineJoin::MITER_CLIP: return out << "miter-clip"s;
            case StrokeLineJoin::ROUND: return out << "round"s;
        }
        return out;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Polyline ----------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool first = true;
        for (auto& point : points_) {
            if (first) {
                out << point.x << ","sv << point.y;
                first = false;
            }
            else {
                out << " "sv << point.x << ","sv << point.y;
            }
        }
        out << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Text --------------------

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(const std::string& font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(const std::string& font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(const std::string& data) {
        data_ = data;
        return *this;
    }

    std::string Text::Replacement(const std::string& input_text) const {
        std::string result = input_text;
        for (const auto& [ch, replacement] : vec_characters_) {
            size_t position{ 0 };
            while (true) {
                position = result.find(ch, position);
                if (position == std::string::npos) {
                    break;
                }
                result.replace(position, 1, replacement);
                position = position + replacement.size();
            }
        }
        return result;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"sv;
        RenderAttrs(out);
        out << " " << "x=\""sv << pos_.x << "\" y=\"" << pos_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\"" << offset_.y << "\""sv;
        out << " font-size=\"" << font_size_ << "\"";
        if (!font_weight_.empty()) {
            out << " font-weight=\"" << font_weight_ << "\"";
        }
        if (!font_family_.empty()) {
            out << " font-family=\"" << font_family_ << "\"";
        }
        out << ">" << Replacement(data_) <<"</text>";
    }

    // ---------- Document ----------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        for (const auto& object: objects_) {
            object->Render(out);
        }
        out << "</svg>"sv;
    }


}  // namespace svg
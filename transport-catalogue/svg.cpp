#include "svg.h"

namespace svg {

using namespace std::literals;

std::ostream& operator<< (std::ostream& out, StrokeLineCap elem) {
    switch (elem) {
    case StrokeLineCap::BUTT:
        out << "butt"sv;
        break;
    case StrokeLineCap::ROUND:
        out << "round"sv;
        break;
    case StrokeLineCap::SQUARE:
        out << "square"sv;
        break;
    }
    return out;
}

std::ostream& operator<< (std::ostream& out, StrokeLineJoin elem) {
    switch (elem) {
    case StrokeLineJoin::ARCS:
        out << "arcs"sv;
        break;
    case StrokeLineJoin::BEVEL:
        out << "bevel"sv;
        break;
    case StrokeLineJoin::MITER:
        out << "miter"sv;
        break;
    case StrokeLineJoin::MITER_CLIP:
        out << "miter-clip"sv;
        break;
    case StrokeLineJoin::ROUND:
        out << "round"sv;
        break;
    }
    return out;
}

struct PrintColor {
    std::ostream& out;

    void operator() (std::monostate) {
        out << "none"sv;
    }
    void operator() (std::string& color) {
        out << color;
    }
    void operator() (Rgb color) {
        out << "rgb("sv << +color.red << ',' << +color.green << ',' << +color.blue << ')';
    }
    void operator() (Rgba color) {
        out << "rgba("sv << +color.red << ',' << +color.green << ',' << +color.blue << ',' << color.opacity << ')';
    }

};

std::ostream& operator<< (std::ostream& out, Color elem) {
    std::visit(PrintColor{out}, elem);
    return out;
}

void Object::Render(const RenderContext& context) const {

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

//----------- Polyline ---------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool is_first = true;
    for (const Point& point : points_ ) {
        if (!is_first) {
            out << ' ';
        } else {
            is_first = false;
        }
        out << point.x << ","sv << point.y;
    }
    out << "\""sv;

    RenderAttrs(out);

    out << "/>"sv;
}

//------------ Text ------------------

Text& Text::SetPosition(Point pos) {
    position_ = pos;
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

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text"sv;
    RenderAttrs(out);
    out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" dx=\""sv;
    out << offset_.x << "\" dy=\""sv << offset_.y << "\" font-size=\""sv << font_size_;
    if (!font_family_.empty()) {
        out << "\" font-family=\""sv << font_family_;
    }
    
    if (!font_weight_.empty()) {
        out << "\" font-weight=\""sv << font_weight_;
    }
    out << "\">"sv;

    for (char c : data_) {
        switch (c) {
        case '"':
            out << "&quot;"sv;
            break;
        case 39: //символ '
            out << "&apos;"sv;
            break;
        case '<':
            out << "&lt;"sv;
            break;
        case '>':
            out << "&gt;"sv;
            break;
        case '&':
            out << "&amp;"sv;
            break;
        default:
            out << c;
        }
    }
    out << "</text>"sv;
}

//---------- Document ---------------

void Document::AddPtr (std::unique_ptr<Object>&& obj) {
    objects_.push_back(move(obj));
}

void Document::Render(std::ostream& out) const {
    RenderContext context(out, 1);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for (const std::unique_ptr<Object>& obj : objects_) {
        context.Indented().RenderIndent();
        obj->Render(context);
    }
    out << "</svg>"sv;
}

}  // namespace svg
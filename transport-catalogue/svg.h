#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <string_view>
#include <variant>


namespace svg {


    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    struct Rgb {
        Rgb() = default;
        Rgb(uint8_t red, uint8_t green, uint8_t blue)
            :red(red), green(green), blue(blue) {}

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba : public Rgb {
        Rgba() = default;
        Rgba(uint8_t red, uint8_t green, uint8_t blue, double visibility)
            :Rgb(red, green, blue), opacity(visibility) {}

        double opacity = 1;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    inline const Color NoneColor{ "none" };


    struct OverloadedColor {
        std::ostream& out_;
    
        void operator()(std::monostate) const;
        void operator()(const std::string& color) const;
        void operator()(Rgb color) const;
        void operator()(Rgba color) const;
    };

    std::ostream& operator<<(std::ostream& out, const Color& color);

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap cap);

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin join);

    template<typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_line_cap_ = line_cap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_line_join_ = line_join;
            return AsOwner();
        }

    protected:
        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            PrintProperties(out, "fill"sv, fill_color_);
            PrintProperties(out, "stroke"sv, stroke_color_);
            PrintProperties(out, "stroke-width"sv, stroke_width_);
            PrintProperties(out, "stroke-linecap"sv, stroke_line_cap_);
            PrintProperties(out, "stroke-linejoin"sv, stroke_line_join_);
        }

        ~PathProps() = default;

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double>  stroke_width_;
        std::optional<StrokeLineCap>  stroke_line_cap_;
        std::optional<StrokeLineJoin>  stroke_line_join_;
    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }

        template<typename Type>
        void PrintProperties(std::ostream& out, std::string_view tag_name,
            const std::optional<Type>& tag) const {
            if (tag) {
                out << " " << tag_name << "=\"" << *tag << "\"";
            }
        }
    };

    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    class Polyline : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_;
    };

    class Text : public Object, public PathProps<Text> {
    public:
        struct Elements {
            char ch;
            std::string replacement;
        };

        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);
    
        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);
    
        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);
    
        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(const std::string& font_family);
    
        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(const std::string& font_weight);
    
        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(const std::string& data);
    
    private:
        void RenderObject(const RenderContext& context) const override;
        std::string Replacement(const std::string& input_text) const;

        inline static const std::vector<Elements> vec_characters_{
        {'&', "&amp;"}, {'"', "&quot;"}, {'\'', "&apos;"}, {'<', "&lt;"}, {'>', "&gt;"} };

        Point pos_;
        Point offset_;
        uint32_t font_size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;
    };


    class ObjectContainer {
    public:
        template<typename Obj>
        void Add(Obj obj) {
             AddPtr(std::make_unique<Obj>(std::move(obj)));
        }

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

        virtual ~ObjectContainer() = default;

    };

    class Document: public ObjectContainer {
    public:
        void AddPtr(std::unique_ptr<Object>&& obj) override;
        void Render(std::ostream& out) const;
    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& /*container*/) const = 0;
        virtual ~Drawable() = default;
    };
}  // namespace svg
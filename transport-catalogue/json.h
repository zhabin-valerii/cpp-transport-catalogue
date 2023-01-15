#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    class Node;
    // ��������� ���������� Dict � Array ��� ���������
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;


    // ��� ������ ������ ������������� ��� ������� �������� JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    struct NodeOverloaded {
        std::ostream& out;

        void operator()(std::nullptr_t) const;
        void operator()(bool value) const;
        void operator()(int value) const;
        void operator()(double value) const;
        void operator()(const std::string& value) const;
        void operator()(const Dict& map) const;
        void operator()(const Array& value) const;
    };

    class Node final: private std::variant<std::nullptr_t, bool, int, double, std::string, Dict, Array> {
    public:
        using variant::variant;
        using Value = variant;

        bool IsNull() const;
        bool IsBool() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsString() const;
        bool IsArray() const;
        bool IsMap() const;

        const Value& GetValue() const;
        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        friend bool operator==(const Node& lhs, const Node& rhs);
        friend bool operator!=(const Node& lhs, const Node& rhs);
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        friend bool operator==(const Document& lhs, const Document& rhs);
        friend bool operator!=(const Document& lhs, const Document& rhs);

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json
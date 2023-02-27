#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;


    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final: private std::variant<std::nullptr_t, bool, int, double, std::string, Dict, Array> {
    public:
        using variant::variant;
        using Value = variant;

        Node() = default;
        Node(Value& value);

        bool IsNull() const;
        bool IsBool() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsString() const;
        bool IsArray() const;
        bool IsDict() const;

        const Value& GetValue() const;
        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        Array& AsArray();
        const Dict& AsDict() const;
        Dict& AsDict();

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
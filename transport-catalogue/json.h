#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <utility>
#include <map>
#include <iomanip>

namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using NodeType = std::variant<std::nullptr_t, bool, int, double, std::string, Array, Dict>;
const int TAB = 4;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    Node() = default;
    Node(std::nullptr_t){}
    Node(bool value);
    Node(const int value);
    Node(const double value);
    Node(const std::string value);
    Node(const Array array);
    Node(const Dict map);

    bool operator== (Node& other) const {return other.data_ == data_;}
    bool operator!= (Node& other) const {return other.data_ != data_;}
    bool operator== (const Node& other) const {return other.data_ == data_;}
    bool operator!= (const Node& other) const {return other.data_ != data_;}
    
    bool AsBool();
    int AsInt();
    double AsDouble();
    std::string& AsString();
    Array& AsArray();
    Dict& AsMap();

    bool IsNull() const {return std::holds_alternative<std::nullptr_t>(data_);}
    bool IsBool() const {return std::holds_alternative<bool>(data_);}
    bool IsInt() const {return std::holds_alternative<int>(data_);}
    bool IsDouble() const {return std::holds_alternative<double>(data_) || std::holds_alternative<int>(data_);}
    bool IsPureDouble() const {return std::holds_alternative<double>(data_);}
    bool IsString() const {return std::holds_alternative<std::string>(data_);}
    bool IsArray() const {return std::holds_alternative<Array>(data_);}
    bool IsMap() const {return std::holds_alternative<Dict>(data_);}

    const NodeType& GetValue() const {return data_;} 

private:
    NodeType data_;
};

class Document {
public:
    Document() = default;
    explicit Document(Node root);
    Document& operator= (Document& other);
    Document& operator= (Node& other);
    Document& operator= (Document&& other);
    Document& operator= (Node&& other);
    Node& GetRoot();
    bool operator== (Document& other) const {return root_ == other.GetRoot();}
    bool operator!= (Document& other) const {return root_ != other.GetRoot();}
    bool operator== (const Document& other) const {return root_ == other.root_;}
    bool operator!= (const Document& other) const {return root_ != other.root_;}
private:
    Node root_ = {};
};

Document Load(std::istream& input);

void Print(Document& doc, std::ostream& output);

}  // namespace json
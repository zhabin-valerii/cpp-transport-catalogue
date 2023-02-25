#include "json.h"

using namespace std;

namespace json {

    namespace {

        std::string ProcessingInput(istream& input) {
            std::string result;
            while (std::isalpha(static_cast<unsigned char>(input.peek()))) {
                result.push_back(static_cast<char>(input.get()));
            }
            return result;
        }

        Node LoadNode(istream& input);

        Node LoadNull(istream& input) {
            if (auto value = ProcessingInput(input); value == "null"s) {
                return Node{ nullptr };
            }
            throw ParsingError("Incorrect format for Null Node parsing. \"null\" expected"s);
        }

        Node LoadBool(istream& input) {
            std::string str = ProcessingInput(input);
            if (str == "true"s) {
                return Node{true};
            }
            else if (str == "false"s) {
                return Node{ false };
            }
            throw ParsingError("Incorrect format for boll Node parsing. \"true\" or \"false\" expected"s);
        }

        Node LoadArray(istream& input) {
            Array result;

            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));

                if (input >> c) {
                    // Break if this is the end of array
                    if (c == ']')
                        break;
                    // If parsed element was not last, but there is no "," afterwards
                    if (c != ',')
                        throw ParsingError("All elements of the Array should be separated with the \",\" symbol"s);
                }
                else {
                    throw ParsingError("During Array Node parsing expected \",\" or \"]\" symbols"s);
                }
            }

            if (!input){
                throw ParsingError("Incorrect format for Array Node parsing"s);
            }

            return Node{ move(result) };
        }

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return Node{ std::stoi(parsed_num) };
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node{ std::stod(parsed_num) };
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
        Node LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string str;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        str.push_back('\n');
                        break;
                    case 't':
                        str.push_back('\t');
                        break;
                    case 'r':
                        str.push_back('\r');
                        break;
                    case '"':
                        str.push_back('"');
                        break;
                    case '\\':
                        str.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    str.push_back(ch);
                }
                ++it;
            }

            return Node{ std::move(str) };
        }

        Node LoadDict(istream& input) {
            Dict result;

            for (char c; input >> c && c != '}';) {
                if (c == '"') {
                    std::string key = LoadString(input).AsString();
                    if (result.count(key) > 0)
                        throw ParsingError("Key "s + key + " is already exists in the Dict"s);

                    if (input >> c && c != ':')
                        throw ParsingError("Dict \"key\" should be separated from \"value\" with \":\" symbol"s);

                    result.emplace(std::move(key), LoadNode(input));
                }
                else if (c != ',') {
                    throw ParsingError("Dict {\"key\":value} pairs should be separated with \",\" symbol"s);
                }
            }

            if (!input)
                throw ParsingError("Incorrect format for Dict Node parsing"s);

            return Node{ std::move(result) };
        }

        Node LoadNode(istream& input) {
            char c;
            if (!(input >> c)) {
                throw ParsingError("Error parsing");
            }
            if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            }
            if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            }
            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace

    void NodeOverloaded::operator()(std::nullptr_t) const{
        out << "null"s;
    }

    void NodeOverloaded::operator()(bool value) const {
        out << (value ? "true"s : "false"s);
    }

    void NodeOverloaded::operator()(int value) const {
        out << value;
    }

    void NodeOverloaded::operator()(double value) const {
        out << value;
    }

    void NodeOverloaded::operator()(const std::string& value) const {
        out << '"';
        for (const char ch : value) {
            switch (ch)
            {
            case '\r':
                out << "\\r"s;
                break;
            case '\n':
                out << "\\n"s;
                break;
            case '\t':
                out << "\t"s;
                break;
            case '"':
                [[fallthrough]];
            case '\\':
                out << '\\';
                [[fallthrough]];
            default:
                out << ch;
                break;
            }
        }
        out << '"';
    }

    void NodeOverloaded::operator()(const Dict& map) const {
        out << '{';
        int count = 0;
        for (const auto& [key, value] : map) {
            if (count++ != 0) {
                out << ", "s;
            }
            std::visit(NodeOverloaded{out}, Node{key}.GetValue());
            out << ':';
            std::visit(NodeOverloaded{ out }, value.GetValue());
        }
        out << '}';
    }

    void NodeOverloaded::operator()(const Array& value) const {
        out << '[';
        int cout = 0;
        for (const auto& elem : value) {
            if (cout++ != 0) {
                out << ", "s;
            }
            std::visit(NodeOverloaded{ out }, elem.GetValue());
        }
        out << ']';
    }

    //
    //----------bool methods---------

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(*this);
    }
    bool Node::IsBool() const {
        return std::holds_alternative<bool>(*this);
    }
    bool Node::IsInt() const {
        return std::holds_alternative<int>(*this);
    }
    bool Node::IsDouble() const {
        return std::holds_alternative<double>(*this) || std::holds_alternative<int>(*this);;
    }
    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }
    bool Node::IsString() const {
        return std::holds_alternative<std::string>(*this);
    }
    bool Node::IsArray() const {
        return std::holds_alternative<Array>(*this);
    }
    bool Node::IsDict() const {
        return std::holds_alternative<Dict>(*this);
    }

    //----------access methods---------

    const Node::Value& Node::GetValue() const {
        return *this;
    }

    int Node::AsInt() const {
        if (auto value = std::get_if<int>(this)) {
            return *value;
        }
        throw std::logic_error("Impossible to parse node as Int"s);
    }

    bool Node::AsBool() const {
        if (auto value = std::get_if<bool>(this)) {
            return *value;
        }
        throw std::logic_error("Impossible to parse node as bool"s);
    }

    double Node::AsDouble() const {
        if (auto value = std::get_if<double>(this)) {
            return *value;
        }
        if (auto value = std::get_if<int>(this)) {
            return *value;
        }
        throw std::logic_error("Impossible to parse node as double"s);
    }

    const string& Node::AsString() const {
        if (auto value = std::get_if<std::string>(this)) {
            return *value;
        }
        throw std::logic_error("Impossible to parse node as string"s);
    }

    const Array& Node::AsArray() const {
        if(auto value = std::get_if<Array>(this)) {
            return *value;
        }
        throw std::logic_error("Impossible to parse node as Array"s);
    }

    const Dict& Node::AsDict() const {
        if (auto value = std::get_if<Dict>(this)) {
            return *value;
        }
        throw std::logic_error("Impossible to parse node as Dict"s);
    }

    //----------operators---------

    bool operator==(const Node& lhs, const Node& rhs) {
        return lhs.GetValue() == rhs.GetValue();
    }

    bool operator!=(const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }

    //----------document---------

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs == rhs);
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(const Document& doc, std::ostream& output) {
        std::visit(NodeOverloaded{ output }, doc.GetRoot().GetValue());
    }

}  // namespace json
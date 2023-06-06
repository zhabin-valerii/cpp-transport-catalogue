#pragma once

#include "json.h"

#include <deque>

namespace json {

    class Builder;
    class DictReturn;
    class KeyReturn;

    class ArrayReturn {
    public:
        virtual ArrayReturn& Value (Node value) = 0;
        virtual DictReturn& StartDict() = 0;
        virtual ArrayReturn& StartArray() = 0;
        virtual Builder& EndArray() = 0;
        virtual ~ArrayReturn() = default;
    };

    class KeyReturn {
    public:
        virtual DictReturn& Value(Node value) = 0;
        virtual ArrayReturn& StartArray() = 0;
        virtual DictReturn& StartDict() = 0;
        virtual ~KeyReturn() = default;
    };

    class DictReturn {
    public: 
        virtual KeyReturn& Key(std::string key) = 0;
        virtual Builder& EndDict() = 0;
        virtual ~DictReturn() = default;
    };

    class Builder : public DictReturn, public KeyReturn, public ArrayReturn {
    public:
        DictReturn& StartDict() override;
        Builder& EndDict() override;
        ArrayReturn& StartArray() override;
        Builder& EndArray() override;    
        KeyReturn& Key(std::string key) override;
        Builder& Value(Node value) override;
        Node Build();
        ~Builder() override = default;
    private:
        struct KeyWithFlag {
            void operator() (std::string&& key) {
                value = move(key);
                is_value = true;
            }
            std::string value;
            bool is_value = false;
        };
        Node root_;
        std::deque<Node*> stack_; //набор "не закрытых" контейнеров (Array или Dict)
        KeyWithFlag key_; // ключ для Dict, записывается в root_ в момент поступления валидного значения к этому ключу
        bool AddNewNodeContainer(Node node); 
    };
}//json
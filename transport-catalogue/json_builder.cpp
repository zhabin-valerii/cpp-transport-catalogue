#include "json_builder.h"

namespace json {
    using namespace std;

    DictReturn& Builder::StartDict() {
        if (AddNewNodeContainer(Node(Dict{}))) {
            return *this;
        }
        throw logic_error("Start Dict unexpected in this context"s);
    }

    Builder& Builder::EndDict() {
        if (stack_.empty()) {
            throw logic_error ("Dict is not started, but you say End"s);
        }
        if (stack_.back()->IsMap() && !key_.is_value) {
            stack_.pop_back();
            return *this;
        }
        throw logic_error ("Unexpected End of Dict"s);
    }

    ArrayReturn& Builder::StartArray() {
        if (AddNewNodeContainer(Node(Array{}))) {
            return *this;
        }
        throw logic_error("Start Array unexpected in this context"s);
    }

    Builder& Builder::EndArray(){
        if (stack_.empty()) {
            throw logic_error ("Array is not started, but you say End"s);
        }
        if (stack_.back()->IsArray()) {
            stack_.pop_back();
            return *this;
        }
        throw logic_error ("Unexpected End of Array"s);
    }

    KeyReturn& Builder::Key(std::string key){
        if (stack_.empty()) {
            throw logic_error ("Is not the Dict, but you tnter a Key"s);
        }
        if (stack_.back()->IsMap() && !key_.is_value) {
            key_(move(key));
            return *this;
        }
        throw logic_error("Key unexpected in this context"s);
    }

    Builder& Builder::Value(Node value){
        if (stack_.empty() && root_.IsNull()) {
            root_ = move(value);
            return *this;
        }
        if (stack_.empty()) {
            throw logic_error("Not have container from Value"s);
        }
        if (stack_.back()->IsArray()) {
            const_cast<Array&>(get<Array>(stack_.back()->GetValue())).emplace_back(value);
            return *this;
        }
        if (stack_.back()->IsMap() && key_.is_value) {
            const_cast<Dict&>(get<Dict>(stack_.back()->GetValue())).emplace(key_.value, value);
            key_.is_value = false;
            return *this;
        }
        throw logic_error ("Value unexpected in this context"s);
    }

    Node Builder::Build(){
        if (!stack_.empty()) {
            throw logic_error ("Dict or Array is not ended"s);
        }
        if (root_.IsNull()) {
            throw logic_error ("Empty Node"s);
        }
        return move(root_);
    }

    bool Builder::AddNewNodeContainer(Node node) {
        if (stack_.empty() && root_.IsNull()) {
            root_ = move(node);
            stack_.push_back(&root_);
            return true;
        }
        if (stack_.empty()) {
            return false;
        }
        if (stack_.back()->IsArray()) {
            auto& array = const_cast<Array&>(get<Array>(stack_.back()->GetValue()));
            array.emplace_back(node);
            stack_.push_back(&array.back());
            return true;
        }
        if (stack_.back()->IsMap() && key_.is_value) {
            auto& dict = const_cast<Dict&>(get<Dict>(stack_.back()->GetValue()));
            auto it = dict.emplace(key_.value, node).first;
            key_.is_value = false;
            stack_.push_back(&(it->second));
            return true;
        }
        return false;
    }

}//json
#include "json_builder.h"

using namespace std::literals;

namespace json {
	const Node& Builder::Build() const {
		if (is_empty_ || !nodes_stack_.empty()) {
			throw std::logic_error("Builder state is invalid"s);
		}
		return root_;
	}

	Builder::KeyItemContext Builder::Key(std::string key) {
		if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && !has_key_) {
			has_key_ = true;
			key_ = std::move(key);
			return KeyItemContext(*this);
		}
		throw std::logic_error("Incorrect place for key : "s + key);
	}

	Builder& Builder::Value(Node::Value value) {
		Node new_node = value;

		if (is_empty_) {
			root_ = new_node;
			is_empty_ = false;
			return *this;
		}

		bool no_empty = !nodes_stack_.empty();
		auto back = nodes_stack_.back();

		if (no_empty && back->IsDict() && has_key_) {
			back->AsDict().insert({ key_,new_node });
			has_key_ = false;
			return *this;
		}

		if (no_empty && back->IsArray()) {
			back->AsArray().push_back(new_node);
			return *this;
		}
		throw std::logic_error("Incorrect place for value"s);
	}

	Builder::DictItemContext Builder::StartDict() {
		Value(Dict{});
		AddRef(Node(Dict{}));
		return DictItemContext(*this);
	}

	Builder& Builder::EndDict() {
		if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {
			nodes_stack_.pop_back();
			return *this;
		}
		throw std::logic_error("Incorrect place for EndDict"s);
	}

	Builder::ArrayItemContext Builder::StartArray() {
		Value(Array{});
		AddRef(Node(Array{}));
		return ArrayItemContext(*this);
	}

	Builder& Builder::EndArray() {
		if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
			nodes_stack_.pop_back();
			return *this;
		}
		throw std::logic_error("Incorrect place for EndArray"s);
	}

	void Builder::AddRef(const Node& value) {
		if (value.IsArray() || value.IsDict()) {
			if (nodes_stack_.empty()) {
				nodes_stack_.push_back(&root_);
				return;
			}

			auto back = nodes_stack_.back();

			if (back->IsArray()) {
				auto p = &back->AsArray().back();
				nodes_stack_.push_back(std::move(const_cast<Node*>(p)));
				return;
			}

			if (back->IsDict()) {
				auto p = &back->AsDict().at(key_);
				nodes_stack_.push_back(std::move(const_cast<Node*>(p)));
				return;
			}
		}
	}

	Builder::KeyItemContext Builder::ItemContext::Key(std::string key) {
		return builder_.Key(std::move(key));
	}

	Builder::DictItemContext Builder::ItemContext::StartDict() {
		return builder_.StartDict();
	}

	Builder& Builder::ItemContext::EndDict() {
		return builder_.EndDict();
	}

	Builder::ArrayItemContext Builder::ItemContext::StartArray() {
		return builder_.StartArray();
	}

	Builder& Builder::ItemContext::EndArray() {
		return builder_.EndArray();
	}

	Builder::KeyValueItemContext Builder::KeyItemContext::Value(Node::Value value) {
		builder_.Value(std::move(value));
		return KeyValueItemContext{ builder_ };
	}

	Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
		builder_.Value(std::move(value));
		return ArrayItemContext{ builder_ };
	}

}// namespace json
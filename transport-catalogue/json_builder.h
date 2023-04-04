#pragma once

#include <vector>
#include <string>
#include <variant>

#include "json.h"

namespace json {

	class Builder final {
		class ItemContext;
		class KeyItemContext;
		class DictItemContext;
		class ArrayItemContext;
	public:
		Builder() = default;

		const Node& Build() const;
		KeyItemContext Key(std::string key);
		Builder& Value(Node::Value value);

		DictItemContext StartDict();
		Builder& EndDict();

		ArrayItemContext StartArray();
		Builder& EndArray();
	private:
		void AddRef(const Node& value);
		Node root_;
		std::vector<Node*> nodes_stack_;
		bool is_empty_ = true;
		bool has_key_ = false;
		json::Dict::mapped_type* place; // пытался реализовать со свтавкой
	};                                  // сразу в вектор без сохранения места,
										// но не вышло...
	class Builder::ItemContext {
	public:
		ItemContext(Builder& builder) :
			builder_(builder) {}
	protected:
		KeyItemContext Key(std::string key);
		DictItemContext StartDict();
		Builder& EndDict();
		ArrayItemContext StartArray();
		Builder& EndArray();

		Builder& builder_;
	};

	class Builder::KeyItemContext final : public ItemContext {
	public:
		using ItemContext::ItemContext;
		DictItemContext Value(Node::Value value);
	};

	class Builder::DictItemContext final : public ItemContext {
	public:
		using ItemContext::ItemContext;
		using ItemContext::Key;
		using ItemContext::EndDict;
	};

	class Builder::ArrayItemContext final : public ItemContext {
	public:
		using ItemContext::ItemContext;
		ArrayItemContext Value(Node::Value value);
	};
}// namespace json
#pragma once
#include "transport_catalogue.h"

#include <iostream>
#include <string>
#include <string_view>
#include <map>
#include <vector>
//чтение запросов на заполнение базы;

namespace input_reader {
	class Input_reader {
	private:
		std::unordered_map<std::string, std::pair<transport_catalogue::RouteType, std::vector<std::string>>> buses_;
		std::unordered_map<std::string, std::unordered_map<std::string, int>> distances_;
	public:
		std::vector<std::string> SplitIntoWords(std::string& text, transport_catalogue::RouteType& type);

		void AddElemToTC(int count, std::istream& input, transport_catalogue::TransportCatalogue& catalogue);
		void AddStop(std::vector<std::string>& info, transport_catalogue::TransportCatalogue& catalogue);
	};
}//input_reader

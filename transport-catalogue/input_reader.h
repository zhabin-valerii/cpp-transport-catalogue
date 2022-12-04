#pragma once
#include "transport_catalogue.h"

#include <iostream>
#include <string>
#include <string_view>
#include <map>
#include <vector>
//чтение запросов на заполнение базы;

namespace input_reader {

	void InputRequest(transport_catalogue::TransportCatalogue& catalogue);
	void CutWord(std::string& word, size_t begin, size_t end);
	void ParsNum(std::string& word, std::vector<std::string>& words);
	void ParsName(std::string& word, std::vector<std::string>& words, bool& first);
	void ParsDistance(std::string& word, std::vector<std::string>& words);
	void ParseName(std::string& word, std::vector<std::string>& words);
	void ParsCircleRoute(std::string& word, std::vector<std::string>& words, transport_catalogue::RouteType& type);
	void ParsLinearRoute(std::string& word, std::vector<std::string>& words, transport_catalogue::RouteType& type);

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

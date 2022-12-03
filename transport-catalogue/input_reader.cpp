#include "input_reader.h"
#include "geo.h"

#include <iomanip>

using namespace transport_catalogue;
namespace input_reader {
    std::vector<std::string> Input_reader::SplitIntoWords(std::string& text, RouteType& type) {
        std::vector<std::string> words;
        std::string word;
        bool first = true;
        for (const char c : text) {
            if (c == ' ' && first) {
                if (!word.empty()) {
                    words.push_back(std::move(word));
                    first = false;
                }
            }
            if (words.size() >= 4) {
                if (word.size() > 3) {
                    if (word[word.size() - 1] == ' ' && word[word.size() - 2] == 'o' &&
                        word[word.size() - 3] == 't' && word[word.size() - 4] == ' ') {
                        word = word.substr(word.find_first_not_of(' '), word.find('m') - 1);
                        words.push_back(std::move(word));
                    }
                }
            }
            if (c == ':') {
                if (!word.empty()) {
                    word = word.substr(word.find_first_not_of(' '), word.size());
                    words.push_back(std::move(word));
                    continue;
                }
            }
            else if (c == '>') {
                word = word.substr(word.find_first_not_of(' '), word.find_last_not_of(' '));
                words.push_back(std::move(word));
                type = RouteType::CIRCLE;
                continue;
            }
            else if (word.size() > 1) {
                if (word[word.size() - 1] == '-' && c == ' ') {
                    word = word.substr(word.find_first_not_of(' '), word.size());
                    word = word.substr(0, word.size() - 2);
                    words.push_back(std::move(word));
                    type = RouteType::LINEAR;
                    continue;
                }
            }
            if (c == ',') {
                if (!word.empty()) {
                    word = word.substr(word.find_first_not_of(' '), word.size());
                    words.push_back(std::move(word));
                }
            }
            else {
                word += c;
            }
        }
        if (!word.empty()) {
            word = word.substr(word.find_first_not_of(' '), word.size());
            words.push_back(std::move(word));
            //word.clear();
        }

        return words;
    }

    void Input_reader::AddStop(std::vector<std::string>& request, TransportCatalogue& catalogue) {
        std::string name = request[1];
        Coordinates point;
        point.lat = std::stod(request[2]);
        point.lng = std::stod(request[3]);
        catalogue.Addstop(name, point);
        for (size_t i = 4; i < request.size(); i += 2) {
            int dist = std::stoi(request[i]);
            std::string to = request[i + 1];
            distances_[name][to] = dist;
        }
    }

    void Input_reader::AddElemToTC(int count, std::istream& line, TransportCatalogue& catalogue) {
        RouteType type;
        std::string text;
        int i = 0;
        while (i < count && std::getline(line, text)) {
            std::vector<std::string> request = SplitIntoWords(text, type);
            if (request[0] == "Stop") {
                AddStop(request, catalogue);
            }
            else if (request[0] == "Bus") {
                std::string name = request[1];
                request.erase(request.begin(), request.begin() + 2);
                buses_.emplace(name, std::make_pair(type, request));
            }
            ++i;
        }
        for (auto& [name, to_dist] : distances_) {
            for (auto& [to, dist] : to_dist) {
                catalogue.SetDistance(name, to, dist);
            }
        }
        for (auto& [name, type_str] : buses_) {
            catalogue.AddRoute(name, type_str.first, type_str.second);
        }
    }
}//input_reader

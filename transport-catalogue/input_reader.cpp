#include "input_reader.h"
#include "geo.h"

#include <iomanip>

using namespace transport_catalogue;
namespace input_reader {

    void InputRequest(transport_catalogue::TransportCatalogue& catalogue) {
        std::string x;
        std::getline(std::cin, x);
        int count = std::stoi(x);
        Input_reader reader;
        reader.AddElemToTC(count, std::cin, catalogue);
    }

    void CutWord(std::string& word, size_t begin, size_t end) {
        word = word.substr(begin, end);
    }

    void ParseNum(std::string& word, std::vector<std::string>& words) {
        if (!word.empty()) {
            CutWord(word, word.find_first_not_of(' '), word.size());
            words.push_back(std::move(word));
        }
    }

    void ParseRequest(std::string& word, std::vector<std::string>& words, bool& first) {
        if (!word.empty()) {
            words.push_back(std::move(word));
            first = false;
        }
    }

    void ParseDistance(std::string& word, std::vector<std::string>& words) {
        if (word.size() > 3) {
            if (word[word.size() - 1] == ' ' && word[word.size() - 2] == 'o' &&
                word[word.size() - 3] == 't' && word[word.size() - 4] == ' ') {
                CutWord(word, word.find_first_not_of(' '), word.find('m') - 1);
                words.push_back(std::move(word));
            }
        }
    }

    void ParseName(std::string& word, std::vector<std::string>& words) {
        CutWord(word, word.find_first_not_of(' '), word.size());
        words.push_back(std::move(word));
    }

    void ParseCircleRoute(std::string& word, std::vector<std::string>& words, RouteType& type) {
        CutWord(word, word.find_first_not_of(' '), word.find_last_not_of(' '));
        words.push_back(std::move(word));
        type = RouteType::CIRCLE;
    }

    void ParseLinearRoute(std::string& word, std::vector<std::string>& words, RouteType& type) {
        CutWord(word, word.find_first_not_of(' '), word.size());
        CutWord(word, 0, word.size() - 2);
        words.push_back(std::move(word));
        type = RouteType::LINEAR;
    }

    std::vector<std::string> Input_reader::SplitIntoWords(std::string& text, RouteType& type) {
        std::vector<std::string> words;
        std::string word;
        bool first = true;
        for (const char c : text) {
            if (c == ' ' && first) {
                ParseRequest(word, words, first);
            }
            if (words.size() >= 4) {
                ParseDistance(word, words);
            }
            if (c == ':') {
                if (!word.empty()) {
                    ParseName(word, words);
                    continue;
                }
            }
            else if (c == '>') {
                ParseCircleRoute(word, words, type);
                continue;
            }
            else if (word.size() > 1) {
                if (word[word.size() - 1] == '-' && c == ' ') {
                    ParseLinearRoute(word, words, type);
                    continue;
                }
            }
            if (c == ',') {
                ParseNum(word, words);
            }
            else {
                word += c;
            }
        }
        if (!word.empty()) {
            CutWord(word, word.find_first_not_of(' '), word.size());
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
        catalogue.AddStop(name, point);
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

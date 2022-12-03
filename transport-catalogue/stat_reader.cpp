#include "stat_reader.h"

#include <iomanip>
#include <vector>
#include <sstream>

using namespace transport_catalogue;

void PrintBusInfo(RouteInfo& info) {
	std::cout << "Bus " << info.name << ": " << info.num_of_stops << " stops on route, "
	<< info.num_of_unique_stops << " unique stops, " << std::setprecision(6) << info.route_length
	<< " route length, " << info.curvature << " curvature" << std::endl;
}

void PrintStopInfo(std::string name, std::set<std::string_view>& buses) {
	std::stringstream out;
	if (buses.empty()) {
		out << "Stop " << name << ": no buses";
	}
	else {
		out << "Stop " << name << ": buses";
		for (auto& bus : buses) {
			out << " " << bus;
		}
	}
	std::cout << out.str() << std::endl;
}

void OutInfo(int count, std::istream& line, TransportCatalogue& cataloge) {
	std::string text;
	int i = 0;
	
	//std::vector<std::tuple<std::string, int, int, double>> result(count);
	while (i < count && std::getline(line, text)) {
		std::vector<std::string> request;
		request.push_back(text.substr(0, text.find(' ')));
		request.push_back(text.substr(text.find(' ') + 1, text.find_last_not_of(' ')));
		if (request[0] == "Bus") {
			try {
				auto temp = cataloge.GetBusInfo(request[1]);
				PrintBusInfo(temp);
			}
			catch (std::out_of_range&) {
				std::cout << "Bus " << request[1] << ": not found" << std::endl;
			}
		}
		else if (request[0] == "Stop") {
			try {
				auto name = cataloge.FindStop(request[1])->name_;
				std::set<std::string_view> buses;
				if (cataloge.GetBusesOnStops().find(name) != cataloge.GetBusesOnStops().end()) {
					buses = cataloge.GetBusesOnStops().at(name);
					PrintStopInfo(name, buses);
				}
				else {
					PrintStopInfo(name, buses);
				}
			}
			catch (std::out_of_range&) {
				std::cout << "Stop " << request[1] << ": not found" << std::endl;
			}
		}
		++i;
	}
}
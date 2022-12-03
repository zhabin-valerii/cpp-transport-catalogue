#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

#include <iostream>
#include <string>
#include <cassert>
#include <tuple>
#include <iomanip>

using namespace transport_catalogue;
using namespace input_reader;
using namespace std::string_literals;

void Tests() {
	{
		TransportCatalogue cat;
		Coordinates x;
		x.lat = 1.1;
		x.lng = 2.2;
		std::string stop = "stop1"s;
		cat.Addstop(stop, x);
		assert(cat.FindStop(stop)->name_ == stop);
		assert(cat.FindStop(stop)->coordinates_ == x);
	}
	{
		TransportCatalogue cat;
		Coordinates x1;
		x1.lat = 1.1;
		x1.lng = 2.2;
		Coordinates x2;
		x2.lat = 3.3;
		x2.lng = 4.4;
		Coordinates x3;
		x3.lat = 2.3;
		x3.lng = 3.4;
		std::string stop1 = "stop1"s;
		std::string stop2 = "stop2"s;
		std::string stop3 = "stop3"s;
		cat.Addstop(stop1, x1);
		cat.Addstop(stop2, x2);
		cat.Addstop(stop3, x3);
		cat.Addstop(stop3, x3);
		std::vector<std::string> stops;
		stops.push_back(stop1);
		stops.push_back(stop2);
		stops.push_back(stop3);
		std::string bus_name1 = "123";
		cat.AddRoute(bus_name1, RouteType::LINEAR, stops);
		//cat.AddBus(bus_name1, RouteType::UNKNOWN, stops);
		assert(cat.FindRoute(bus_name1)->name_ == bus_name1);
		auto info = cat.GetBusInfo(bus_name1);
	}
}

int main() {
	//Tests();
	TransportCatalogue catalogue;
	int count;
	std::string x;
	std::getline(std::cin, x);
	count = std::stoi(x);
	{
		Input_reader reader;
		reader.AddElemToTC(count, std::cin, catalogue);
	}
	std::getline(std::cin, x);
	count = std::stoi(x);
	{
		OutInfo(count, std::cin, catalogue);
	}
	std::cout << "ok"s << std::endl;
}
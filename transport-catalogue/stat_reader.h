#pragma once
#include "transport_catalogue.h"

#include <iostream>
#include <string>
//чтение запросов на вывод и сам вывод;

void PrintBusInfo(transport_catalogue::RouteInfo& info);
void PrintStopInfo(std::string name, std::set<std::string_view>& buses);

void OutInfo(int count, std::istream& line, transport_catalogue::TransportCatalogue& cataloge);

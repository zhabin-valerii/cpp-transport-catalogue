#pragma once
#include "transport_catalogue.h"

#include <iostream>
#include <string>
//чтение запросов на вывод и сам вывод;
void InputStat(transport_catalogue::TransportCatalogue& catalogue);

void PrintBusInfo(transport_catalogue::RouteInfo& info);
void PrintStopInfo(std::string name, std::set<std::string_view>& buses);

void CutRequest(std::string& text, std::vector<std::string>& request);
void OutInfo(int count, std::istream& line, transport_catalogue::TransportCatalogue& cataloge);

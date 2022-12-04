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

int main() {
	TransportCatalogue catalogue;
	InputRequest(catalogue);
	InputStat(catalogue);
}
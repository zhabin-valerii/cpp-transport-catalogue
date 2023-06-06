#include "transport_catalogue.h"
#include "request_handler.h"
#include "json_reader.h"

#include <cassert>
#include <fstream>
#include <iostream>

using namespace std;
using namespace tr_cat;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        aggregations::TransportCatalogue catalog;
        interface::JsonReader reader(catalog);
        reader.ReadDocument ();
        reader.ParseDocument ();
        reader.AddStops ();
        reader.AddDistances ();
        reader.AddBuses ();
        reader.CreateGraph();
        reader.Serialize (true);
    } else if (mode == "process_requests"sv) {
        aggregations::TransportCatalogue catalog;
        interface::JsonReader reader(catalog);
        reader.ReadDocument ();
        reader.ParseDocument ();
        reader.Deserialize (true);
        reader.GetAnswers ();
        reader.PrintAnswers ();
    } else {
        PrintUsage();
        return 1;
    }
}

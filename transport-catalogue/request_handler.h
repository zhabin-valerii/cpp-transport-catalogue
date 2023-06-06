#pragma once

#include "transport_catalogue.h"

#include <iostream>
#include <optional>
#include <variant>
#include <sstream>

namespace tr_cat {
namespace interface {

using namespace std::string_literals;
class RequestInterface {
public:
    explicit RequestInterface(aggregations::TransportCatalogue& catalog)
    :catalog_(catalog) {}
    RequestInterface(aggregations::TransportCatalogue& catalog, std::istream& input)
    :input_(input), catalog_(catalog) {}
    RequestInterface(aggregations::TransportCatalogue& catalog, std::ostream& output)
    :output_(output), catalog_(catalog) {}
    RequestInterface(aggregations::TransportCatalogue& catalog, std::istream& input, std::ostream& output)
    :input_(input), output_(output), catalog_(catalog) {}

//------------------------------------------read-------------------------------------------------------
    virtual void ReadDocument () = 0;
    virtual void ParseDocument () = 0;
//--------------------------------------------base filling-----------------------------------------------------
    void AddStops ();
    void AddDistances ();
    void AddBuses ();
    virtual void CreateGraph() = 0;
//-------------------------------------------print------------------------------------------------------  
    virtual void RenderMap(std::ostream& out = std::cout) = 0;
    void GetAnswers ();
    virtual void PrintAnswers () = 0;
//--------------------------------------------serializstion-----------------------------------------------------
    virtual bool Serialize (bool with_graph) const = 0;
    virtual bool Deserialize (bool with_graph) = 0;
//------------------------------------------------------------------------------------------------------

    virtual bool TestingFilesOutput(std::string filename_lhs,
                         std::string filename_rhs) = 0;

    virtual ~RequestInterface() = default;

protected:
    const aggregations::TransportCatalogue& GetCatalog() const {return catalog_;}
    struct BusInput {
        std::string_view name;
        std::vector <std::string_view> stops;
        bool is_ring;
    };
    struct StopInput {
        std::string_view name;
        geo::Coordinates coordinates;
    };
    struct Stat {
        int id;
        std::string_view type;
        std::string_view name;
        std::string_view from;
        std::string_view to;
    };
    struct StopOutput {
        int id;
        const Stop* stop;
    };
    struct BusOutput {
        int id;
        const Bus* bus;
    };
    struct MapOutput {
        explicit MapOutput(int i, aggregations::TransportCatalogue& cat)
        :id(i), catalog(&cat){}
        int id;
        aggregations::TransportCatalogue* catalog;
    };
    struct RouteOutput {
        int id;
        const Stop* from;
        const Stop* to;
    };
    std::vector<StopInput> stops_;
    std::vector<BusInput> buses_;
    std::unordered_map<std::string_view, std::vector<std::pair<std::string_view, int>>> distances_;
    std::vector<Stat> stats_;
    std::vector<std::variant<int, StopOutput, BusOutput, MapOutput, RouteOutput>> answers_;
    std::istream& input_ = std::cin;
    std::ostream& output_ = std::cout;

private:
    aggregations::TransportCatalogue& catalog_;
};

void Process(interface::RequestInterface& reader);



}//interface
}//tr_cat

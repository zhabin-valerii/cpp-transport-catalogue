#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "transport_router.h"
#include "serialization.h"

#include <iostream>
#include <fstream>
#include <exception>
#include <sstream>
#include <cmath>

namespace tr_cat {
namespace interface {
class JsonReader : public RequestInterface {
public:

    explicit JsonReader(aggregations::TransportCatalogue& catalog)
    :RequestInterface (catalog)
    , transport_router_(catalog)
    , renderer_(catalog)
    , serializator_(catalog, renderer_, transport_router_){}

    JsonReader(aggregations::TransportCatalogue& catalog, std::istream& input)
    :RequestInterface (catalog, input)
    , transport_router_(catalog)
    , renderer_(catalog)
    , serializator_(catalog, renderer_, transport_router_){}

    JsonReader(aggregations::TransportCatalogue& catalog, std::ostream& output)
    :RequestInterface (catalog, output)
    , transport_router_(catalog)
    , renderer_(catalog)
    , serializator_(catalog, renderer_, transport_router_){}

    JsonReader(aggregations::TransportCatalogue& catalog, std::istream& input, std::ostream& output)
        :RequestInterface (catalog, input, output)
        , transport_router_(catalog)
        , renderer_(catalog)
        , serializator_(catalog, renderer_, transport_router_){}

    void ReadDocument () override;
    void ParseDocument () override;
    bool Serialize(bool with_graph = false) const override {return serializator_.Serialize(with_graph);}
    bool Deserialize(bool with_graph = false) override {return serializator_.Deserialize(with_graph); }
    void RenderMap(std::ostream& out = std::cout) override {renderer_.Render(out);}
    void CreateGraph() override {transport_router_.CreateGraph();}
    void PrintAnswers () override;
    bool TestingFilesOutput(std::string filename_lhs, std::string filename_rhs) override;
    const render::RenderSettings& GetRenderSettings() const;
private:
    struct CreateNode {
        friend class JsonReader;
        explicit CreateNode(render::MapRenderer& renderer, router::TransportRouter& router)
        :renderer_(renderer), transport_router_(router){}
        json::Node operator() (int value);
        json::Node operator() (StopOutput& value);
        json::Node operator() (BusOutput& value);
        json::Node operator() (MapOutput& value);
        json::Node operator() (RouteOutput& value);
    private:
        render::MapRenderer& renderer_;
        router::TransportRouter& transport_router_;
    };
    json::Document document_ = {};
    json::Document document_answers_ = {};
    router::TransportRouter transport_router_;
    render::MapRenderer renderer_;
    serialize::Serializator serializator_;

    void ParseBase (json::Node& base);
    void ParseStats (json::Node& stats);
    void ParseRenderSettings(json::Node& render_settings);
    void ParseRoutingSettings(json::Node& routing_settings);
    void PrepareToPrint ();

};
} //interface
}//tr_cat

#pragma once

#include "geo.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "svg.h"
#include "router.h"
#include "request_handler.h"

using namespace std;
using namespace svg;
using namespace router;

namespace tr_cat {
namespace serialize {

class Serializator {
public:
    Serializator (aggregations::TransportCatalogue& catalog,
                  render::MapRenderer& renderer,
                  router::TransportRouter& router)
        :catalog_(catalog)
        ,renderer_(renderer)
        ,transport_router_(router) {}

    void SetPathToSerialize(const std::filesystem::path& path) {path_to_serialize_= path;}
    size_t Serialize(bool with_graph = false) const;
    bool Deserialize(bool with_graph = false);

    transport_catalog_serialize::Catalog SerializeCatalog() const;
    bool DeserializeCatalog(transport_catalog_serialize::Catalog& catalog);

    transport_catalog_serialize::RenderSettings SerializeRenderer() const;
    bool DeserializeRenderer(transport_catalog_serialize::RenderSettings& settings);

    transport_catalog_serialize::Router SerializeRouter(bool with_graph) const;
    bool DeserializeRouter(transport_catalog_serialize::Router& router_data, bool with_graph);
private:
    aggregations::TransportCatalogue& catalog_;
    render::MapRenderer& renderer_;
    router::TransportRouter& transport_router_;
    std::filesystem::path path_to_serialize_;
};

}//serialize
}//tr_cat

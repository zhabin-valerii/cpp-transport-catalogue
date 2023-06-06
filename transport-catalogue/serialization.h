#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

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

private:
    aggregations::TransportCatalogue& catalog_;
    render::MapRenderer& renderer_;
    router::TransportRouter& transport_router_;
    std::filesystem::path path_to_serialize_;
};

}//serialize
}//tr_cat

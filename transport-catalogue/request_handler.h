#pragma once
#include "json_reader.h"
#include "map_renderer.h"

#include <iostream>

namespace transport_catalogue {
    class RequestHandler {
    public:
        explicit RequestHandler(TransportCatalogue& catalogue) :
            catalogue_(catalogue) {}

        domain::RouteInfo GetRouteInfo(const std::string& bus_name) const;
        std::optional<std::reference_wrapper<const std::set<std::string_view>>>
            GetBusesOnStop(const std::string& stop_name) const;

        void ProcessRequest(std::istream& input, std::ostream& out);

        svg::Document RenderMap() const;

        void SetRenderSettings(const renderer::RenderSettings& render_settings);

    private:
        TransportCatalogue& catalogue_;
        std::optional<renderer::RenderSettings> render_settings_;
        //const renderer::MapRenderer& renderer_;
    };
}//namespace request_handler

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

 // Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
 // с другими подсистемами приложения.
 // См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
 /*
 class RequestHandler {
 public:
     // MapRenderer понадобится в следующей части итогового проекта
     RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);

     // Возвращает информацию о маршруте (запрос Bus)
     std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

     // Возвращает маршруты, проходящие через
     const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

     // Этот метод будет нужен в следующей части итогового проекта
     svg::Document RenderMap() const;

 private:
     // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
     const TransportCatalogue& db_;
     const renderer::MapRenderer& renderer_;
 };
 */
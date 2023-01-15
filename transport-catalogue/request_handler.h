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
 * ����� ����� ���� �� ���������� ��� ����������� �������� � ����, ����������� ������, ������� ��
 * �������� �� �������� �� � transport_catalogue, �� � json reader.
 *
 * � �������� ��������� ��� ���� ���������� ��������� �� ���� ������ ����������� ��������.
 * �� ������ ����������� ��������� �������� ��������, ������� ������� ���.
 *
 * ���� �� ������������� �������, ��� ����� ���� �� ��������� � ���� ����,
 * ������ �������� ��� ������.
 */

 // ����� RequestHandler ������ ���� ������, ����������� �������������� JSON reader-�
 // � ������� ������������ ����������.
 // ��. ������� �������������� �����: https://ru.wikipedia.org/wiki/�����_(������_��������������)
 /*
 class RequestHandler {
 public:
     // MapRenderer ����������� � ��������� ����� ��������� �������
     RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);

     // ���������� ���������� � �������� (������ Bus)
     std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

     // ���������� ��������, ���������� �����
     const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

     // ���� ����� ����� ����� � ��������� ����� ��������� �������
     svg::Document RenderMap() const;

 private:
     // RequestHandler ���������� ��������� �������� "������������ ����������" � "������������ �����"
     const TransportCatalogue& db_;
     const renderer::MapRenderer& renderer_;
 };
 */
#include "motis/loader/netex/service_frame/service_frame_parse.h"

#include <map>
#include <string_view>
#include <vector>

#include "pugixml.hpp"

#include "motis/loader/netex/service_frame/service_frame.h"

namespace xml = pugi;

namespace motis::loader::netex {
// TODO Rückgabe wert mit pointern? da so nur kopiert wird
std::map<std::string, line> parse_line(
    xml::xpath_node const& service_jorney,
    std::map<std::string, Operator_Authority> const& operator_map) {
  auto line_map = std::map<std::string, line>{};
  for (auto const& line_get :
       service_jorney.node().select_nodes(".//lines/Line")) {
    auto const id = std::string(line_get.node().attribute("id").as_string());
    auto const operator_id = std::string(line_get.node()
                                             .child("additionalOperators")
                                             .child("OperatorRef")
                                             .attribute("ref")
                                             .as_string());
    auto line_parse = line{};
    line_parse.name_ = line_get.node().child("Name").text().as_int();
    line_parse.short_name_ =
        line_get.node().child("ShortName").text().as_string();
    line_parse.transport_mode_ =
        line_get.node().child("TransportMode").text().as_string();
    line_parse.id_ = id;
    line_parse.operator_ = operator_map.at(operator_id);
    line_map.try_emplace(id, line_parse);
  }  // Line
  return line_map;
}

std::map<std::string, Operator_Authority> parse_operator(xml::xml_document& d) {
  auto operator_map = std::map<std::string, Operator_Authority>{};
  for (auto const& operator_get :
       d.select_nodes("/PublicationDelivery/dataObjects/CompositeFrame/frames/"
                      "ResourceFrame/organisations/Operator")) {
    auto const id =
        std::string(operator_get.node().attribute("id").as_string());
    auto operator_parse = Operator_Authority{};
    operator_parse.name_ = operator_get.node().child("Name").text().as_string();
    operator_parse.short_name_ =
        operator_get.node().child("ShortName").text().as_string();
    operator_parse.legal_name_ =
        operator_get.node().child("LegalName").text().as_string();
    operator_parse.public_code_ =
        operator_get.node().child("PublicCode").text().as_string();
    operator_map.try_emplace(id, operator_parse);
  }  // Operator
  return operator_map;
}

std::map<std::string, direction> parse_direction(
    xml::xpath_node const& service_jorney) {
  auto direction_map = std::map<std::string, direction>{};
  for (auto const& direction_get :
       service_jorney.node().select_nodes(".//directions/Direction")) {
    auto const id =
        std::string(direction_get.node().attribute("id").as_string());
    auto dir = direction{};
    dir.name_ = direction_get.node().child("Name").text().as_string();
    dir.short_name_ =
        direction_get.node().child("ShortName").text().as_string();
    direction_map.try_emplace(id, dir);
  }  // Direction
  return direction_map;
}

std::map<std::string, passenger_assignments> parse_passenger_assignment(
    xml::xpath_node const& service_jorney) {
  auto passengers_assignment_map =
      std::map<std::string, passenger_assignments>{};
  auto p_a = passenger_assignments{};
  for (auto const& passenger_assignments : service_jorney.node().select_nodes(
           ".//stopAssignments/PassengerStopAssignment")) {
    auto const key =
        std::string(passenger_assignments.node().attribute("id").as_string());
    p_a.scheduled_place_id_ = passenger_assignments.node()
                                  .child("ScheduledStopPointRef")
                                  .attribute("ref")
                                  .as_string();
    p_a.stop_point_id_ = passenger_assignments.node()
                             .child("StopPlaceRef")
                             .attribute("ref")
                             .as_string();
    p_a.quay_id_ = passenger_assignments.node()
                       .child("QuayRef")
                       .attribute("ref")
                       .as_string();
    passengers_assignment_map.try_emplace(key, p_a);
  }  // PassengerStopAssignment
  return passengers_assignment_map;
}
std::map<std::string, stop_point> parse_stop_place(xml::xml_document& d) {
  auto stops = std::map<std::string, stop_point>{};
  for (auto const& stop_points :
       d.select_nodes("/PublicationDelivery/dataObjects/CompositeFrame/"
                      "frames/SiteFrame/stopPlaces/StopPlace")) {
    auto const id = std::string(stop_points.node().attribute("id").as_string());
    auto stop = stop_point{};
    stop.key_ = stop_points.node()
                    .child("keyList")
                    .child("KeyValue")
                    .child("Key")
                    .text()
                    .as_string();
    stop.value_ = stop_points.node()
                      .child("keyList")
                      .child("KeyValue")
                      .child("Value")
                      .text()
                      .as_string();
    stop.name_ = stop_points.node().child("Name").text().as_string();
    stop.lon_ = stop_points.node()
                    .child("Centroid")
                    .child("Location")
                    .child("Longitude")
                    .text()
                    .as_double();
    stop.lat_ = stop_points.node()
                    .child("Centroid")
                    .child("Location")
                    .child("Latitude")
                    .text()
                    .as_double();
    stop.timezone_ =
        stop_points.node().child("Locale").child("TimeZone").text().as_string();
    auto quay_v = std::vector<std::string>{};
    for (auto const& quay : stop_points.node().select_nodes(".//Quay")) {
      auto const name =
          std::string(quay.node().child("Name").text().as_string());
      quay_v.push_back(name);
    }  // Quay
    stop.quay_ = quay_v;
    stops.try_emplace(id, stop);
  }  // StopPlace
  return stops;
}
std::map<std::string, scheduled_points> parse_scheduled_points(
    xml::xpath_node const& service_jorney) {
  auto scheduled_stops_map = std::map<std::string, scheduled_points>{};
  for (auto const& scheduled_points_get : service_jorney.node().select_nodes(
           ".//scheduledStopPoints/ScheduledStopPoint")) {
    auto const id =
        std::string(scheduled_points_get.node().attribute("id").as_string());
    auto points = scheduled_points{};
    points.short_name_ =
        scheduled_points_get.node().child("ShortName").text().as_string();
    points.public_code_ =
        scheduled_points_get.node().child("PublicCode").text().as_string();
    points.stop_type_ =
        scheduled_points_get.node().child("StopType").text().as_string();

    scheduled_stops_map.try_emplace(id, points);
  }  // ScheduledStopPoint
  return scheduled_stops_map;
}
void parse_frame(xml::xml_document& d, std::map<std::string, line>& l_m,
                 std::map<std::string, scheduled_points>& s_m,
                 std::map<std::string, stop_point>& s_p_m,
                 std::map<std::string, direction>& d_m,
                 std::map<std::string, passenger_assignments>& p_m) {
  auto o = parse_operator(d);
  s_p_m = parse_stop_place(d);
  for (auto const& sf : d.select_nodes("/PublicationDelivery/dataObjects/"
                                       "CompositeFrame/frames/ServiceFrame")) {
    l_m = parse_line(sf, o);
    s_m = parse_scheduled_points(sf);
    d_m = parse_direction(sf);
    p_m = parse_passenger_assignment(sf);
  }  // ServiceFrame
}
}  // namespace motis::loader::netex
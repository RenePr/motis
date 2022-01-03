#include "motis/loader/netex/service_frame_parse.h"

#include <iostream>
#include <map>
#include <string_view>

#include "pugixml.hpp"

#include "motis/loader/netex/service_journey.h"
#include "motis/loader/netex/service_frame.h"

namespace xml = pugi;

namespace motis::loader::netex {

std::map<std::string, line> parse_line(xml::xpath_node const& service_jorney, std::map<std::string, Operator_Authority>& operator_map) {
  std::map<std::string, line> line_map;
  for(auto const& line_get : service_jorney.node().select_nodes("//lines/Line")) {
    auto const id = std::string(line_get.node().attribute("id").as_string());
    auto const operator_id = std::string(line_get.node().child("AuthorityRef").attribute("ref").as_string());
    line line_parse;
    line_parse.name_ = line_get.node().child("Name").text().as_string();
    line_parse.short_name_ = line_get.node().child("ShortName").text().as_string();
    line_parse.transport_mode_ = line_get.node().child("TransportMode").text().as_string();
    //std::string_view operator_ref {line_get.node().child("OperatorRef").attribute("ref").as_string() };
    if(operator_map.find(operator_id) != operator_map.end()) {
      line_parse.operator_ = operator_map.at(operator_id);
    } else {
      //keine Ref gefunden
    }
    line_map.try_emplace(id, line_parse);
  }
  return line_map;
}

std::map<std::string, Operator_Authority> parse_operator(xml::xml_document& d){
  std::map<std::string, Operator_Authority> operator_map;
  for(auto const& operator_get : d.select_nodes("//dataObjects/CompositeFrame/frames/ResourceFrame/organisations/Operator")) {
    auto const id = std::string(operator_get.node().attribute("id").as_string());
    Operator_Authority operator_parse;
    //childs: PublicCode, Name, Shortname, LegalName, ContactDetails child Url, Organisation Type, Address
    operator_parse.name_ = operator_get.node().child("Name").text().get();
    operator_parse.short_name_ = operator_get.node().child("ShortName").text().as_string();
    operator_parse.legal_name_ = operator_get.node().child("LegalName").text().as_string();
    operator_parse.public_code_ = operator_get.node().child("PublicCode").text().as_string();
    operator_map.try_emplace(id, operator_parse);
  }
  return operator_map;
}

std::map<std::string, direction> parse_direction(xml::xpath_node const& service_jorney) {
  std::map<std::string, direction> direction_map;
  for(auto const& direction_get : service_jorney.node().select_nodes("//directions/Direction")) {
    auto const id = std::string(direction_get.node().attribute("id").as_string());
    direction dir;
    dir.name_ = direction_get.node().child("Name").text().as_string();
    dir.short_name_ = direction_get.node().child("ShortName").text().as_string();
    direction_map.try_emplace(id, dir);
  }
  return direction_map;
}

std::map<std::string, std::string> parse_passenger_assignment(xml::xpath_node const& service_jorney) {
  std::map<std::string, std::string> passengers_assignment_map;
  for(auto const& passenger_assignments : service_jorney.node().select_nodes("//stopAssignments/PassengerStopAssignment")) {
    auto const scheduled_place = passenger_assignments.node().child("//ScheduledStopPointRef").attribute("ref").as_string();
    auto const stop_place = passenger_assignments.node().child("//StopPlaceRef").attribute("ref").as_string();
    passengers_assignment_map.try_emplace(scheduled_place, stop_place);
  }
  return passengers_assignment_map;
}

std::map<std::string, scheduled_points> parse_scheduled_points(xml::xpath_node const& service_jorney, xml::xml_document& d) {
  //TODO über PassengerStopAssignment -> StopPlace für latidude altidude hinzufügen
  auto passenger_assignments = parse_passenger_assignment(service_jorney);
  std::map<std::string, scheduled_points> scheduled_stops_map;
  for(auto const& scheduled_points_get : service_jorney.node().select_nodes("//scheduledStopPoints/ScheduledStopPoint")) {
    auto const id = std::string(scheduled_points_get.node().attribute("id").as_string());
    scheduled_points points;
    points.short_name_ = scheduled_points_get.node().child("ShortName").text().as_string();
    points.public_code_ = scheduled_points_get.node().child("PublicCode").text().as_string();
    points.stop_type_ = scheduled_points_get.node().child("StopType").text().as_string();
    stop_point stop;
    for(auto const& stop_points : d.select_nodes("//dataObjects/CompositeFrame/frames/SiteFrame/stopPlaces/StopPlace")) {
      auto const id_stop_point = std::string_view(stop_points.node().attribute("id").as_string());
      auto const id_scheduled = std::string_view(passenger_assignments.at(id));
      if(id_stop_point == id_scheduled) {
        stop.key_ = stop_points.node().child("keyList").child("KeyValue").child("Key").text().as_string();
        stop.value_ = stop_points.node().child("keyList").child("KeyValue").child("Value").text().as_string();
        stop.name_ = stop_points.node().child("keyList").child("Name").text().as_string();
        stop.lon_ = stop_points.node().child("keyList").child("Centroid").child("Location").child("Longitude").text().as_double();
        stop.lat_ = stop_points.node().child("keyList").child("Centroid").child("Location").child("Latitude").text().as_double();
        stop.timezone_ = stop_points.node().child("keyList").child("Locale").child("TimeZone").text().as_string();
        points.stop_point_ = stop;
      }
    }
    scheduled_stops_map.try_emplace(id, points);
  }

  return scheduled_stops_map;
}

} //motis::loader::netex
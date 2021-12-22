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
    line line_parse;
    std::string id = line_get.node().attribute("id").as_string();
    line_parse.name_ = line_get.node().child("Name").text().as_string();
    line_parse.short_name_ = line_get.node().child("ShortName").text().as_string();
    line_parse.transport_mode_ = line_get.node().child("TransportMode").text().as_string();
    std::string operator_id = line_get.node().child("AuthorityRef").attribute("ref").as_string();
    //std::string_view operator_ref {line_get.node().child("OperatorRef").attribute("ref").as_string() };
    if(operator_map.find(operator_id) != operator_map.end()) {
      line_parse.operator_ = operator_map.at(operator_id);
    } else {
      //keine Ref gefunden
    }
    auto const& pair = std::make_pair(id, line_parse);
    line_map.insert(pair);
  }
  return line_map;
}

std::map<std::string, Operator_Authority> parse_operator(xml::xml_document& d){
  std::map<std::string, Operator_Authority> operator_map;
  for(auto const& operator_get : d.select_nodes("//dataObjects/CompositeFrame/frames/ResourceFrame/organisations/Operator")) {
    Operator_Authority operator_parse;
    std::string id = operator_get.node().attribute("id").as_string();
    //childs: PublicCode, Name, Shortname, LegalName, ContactDetails child Url, Organisation Type, Address
    //service_jor.name_operator_ = operator_r.node().child("Name").text().get();
    operator_parse.name_ = operator_get.node().child("Name").text().get();
    operator_parse.short_name_ = operator_get.node().child("ShortName").text().as_string();
    operator_parse.legal_name_ = operator_get.node().child("LegalName").text().as_string();
    operator_parse.public_code_ = operator_get.node().child("PublicCode").text().as_string();
    auto const& pair = std::make_pair(id, operator_parse);
    operator_map.insert(pair);
  }
  return operator_map;
}

std::map<std::string, direction> parse_direction(xml::xpath_node const& service_jorney) {
  std::map<std::string, direction> direction_map;
  for(auto const& direction_get : service_jorney.node().select_nodes("//directions/Direction")) {
    direction dir;
    dir.name_ = direction_get.node().child("Name").text().as_string();
    dir.short_name_ = direction_get.node().child("ShortName").text().as_string();
    std::string id = direction_get.node().attribute("id").as_string();
    auto const& pair = std::make_pair(id, dir);
    direction_map.insert(pair);
  }
  return direction_map;
}

std::map<std::string, std::string> parse_passenger_assignment(xml::xpath_node const& service_jorney) {
  std::map<std::string, std::string> passengers_assignment_map;
  for(auto const& passenger_assignments : service_jorney.node().select_nodes("//stopAssignments/PassengerStopAssignment")) {
    auto scheduled_place = passenger_assignments.node().child("//ScheduledStopPointRef").attribute("ref").as_string();
    auto stop_place = passenger_assignments.node().child("//StopPlaceRef").attribute("ref").as_string();
    auto const& pair = std::make_pair(std::string(scheduled_place), std::string(stop_place));
    passengers_assignment_map.insert(pair);
    //passenger_assignments.node(pair);
  }
  return passengers_assignment_map;
}

std::map<std::string, scheduled_points> parse_scheduled_points(xml::xpath_node const& service_jorney, xml::xml_document& d) {
  //TODO über PassengerStopAssignment -> StopPlace für latidude altidude hinzufügen
  auto passenger_assignments = parse_passenger_assignment(service_jorney);
  std::map<std::string, scheduled_points> scheduled_stops_map;
  for(auto const& scheduled_points_get : service_jorney.node().select_nodes("//scheduledStopPoints/ScheduledStopPoint")) {
    scheduled_points points;
    points.short_name_ = scheduled_points_get.node().child("ShortName").text().as_string();
    points.public_code_ = scheduled_points_get.node().child("PublicCode").text().as_string();
    points.stop_type_ = scheduled_points_get.node().child("StopType").text().as_string();
    std::string id = scheduled_points_get.node().attribute("id").as_string();
    stop_point stop;
    for(auto const& stop_points : d.select_nodes("//dataObjects/CompositeFrame/frames/SiteFrame/stopPlaces/StopPlace")) {
      auto id_stop_point = std::string_view(stop_points.node().attribute("id").as_string());
      auto id_scheduled = std::string_view(passenger_assignments.at(id));
      if(id_stop_point == id_scheduled) {
        stop.key_ = stop_points.node().child("keyList").child("KeyValue").child("Key").text().as_string();
        stop.value_ = stop_points.node().child("keyList").child("KeyValue").child("Value").text().as_string();
        stop.name_ = stop_points.node().child("keyList").child("Name").text().as_string();
        stop.lon_ = stop_points.node().child("keyList").child("Centroid").child("Location").child("Longitude").text().as_string();
        stop.lat_ = stop_points.node().child("keyList").child("Centroid").child("Location").child("Latitude").text().as_string();
        stop.timezone_ = stop_points.node().child("keyList").child("Locale").child("TimeZone").text().as_string();
        points.stop_point_ = stop;
      }
    }
    auto const& pair = std::make_pair(id, points);
    scheduled_stops_map.insert(pair);
  }

  return scheduled_stops_map;
}

} //motis::loader::netex
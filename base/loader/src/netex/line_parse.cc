#include "motis/loader/netex/line_parse.h"

#include <iostream>
#include <map>
#include <string_view>

#include "pugixml.hpp"

#include "motis/loader/netex/service_journey.h"
#include "motis/loader/netex/line.h"

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

} //motis::loader::netex
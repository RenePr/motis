#include "motis/loader/netex/line_parse.h"

#include <iostream>
#include <vector>
#include <string_view>

#include "pugixml.hpp"

#include "motis/loader/netex/service_journey.h"
#include "motis/loader/netex/line.h"

namespace xml = pugi;

namespace motis::loader::netex {

std::vector<line> parse_line(xml::xpath_node const& service_jorney, xml::xml_document& d ) {
  std::vector<line> line_list;
  for (auto const& line_ref :
       service_jorney.node().select_nodes("RouteView")) {
    //std::cout << "Are we here? " << std::endl;
    std::string_view line2 {
        line_ref.node().child("LineRef").attribute("ref").as_string() };
    for(auto const& line_get : d.select_nodes("//Line")) {
      line line_parse;
      std::string_view line_node {line_get.node().attribute("id").as_string()};
      if(line2 == line_node) {
        //std::cout << line_get.node().child("TransportMode").text().get() << std::endl;
        line_parse.name_ = line_get.node().child("Name").text().as_string();
        //std::cout << service_jor.name_ << std::endl;
        line_parse.short_name_ = line_get.node().child("ShortName").text().as_string();
        line_parse.transport_mode_ = line_get.node().child("TransportMode").text().as_string();
        std::string_view authority_ref { line_get.node().child("AuthorityRef").attribute("ref").as_string() };
        std::string_view operator_ref {line_get.node().child("OperatorRef").attribute("ref").as_string() };
        for(auto const& authority : d.select_nodes("//Authority")) {
          Operator_Authority authority_parse;
          std::string_view authority_node {authority.node().attribute("id").as_string()};
          if(authority_ref == authority_node) {
            //childs: Name, PublicCode, LegalName, OrganisationType, Address, ContactDetails
            authority_parse.name_ = authority.node().child("Name").text().as_string();
            authority_parse.short_name_ = authority.node().child("ShortName").text().as_string();
            authority_parse.legal_name_= authority.node().child("LegalName").text().as_string();
            authority_parse.public_code_ = authority.node().child("PublicCode").text().as_string();
            line_parse.authority_.push_back(authority_parse);
          }
        }
        for(auto const& operator_r : d.select_nodes("//Operator")) {
          Operator_Authority operator_parse;
          std::string_view operator_node {operator_r.node().attribute("id").as_string()};
          if(operator_ref == operator_node){
            //childs: PublicCode, Name, Shortname, LegalName, ContactDetails child Url, Organisation Type, Address
            //service_jor.name_operator_ = operator_r.node().child("Name").text().get();
            operator_parse.name_ = operator_r.node().child("Name").text().get();
            operator_parse.short_name_ = operator_r.node().child("ShortName").text().as_string();
            operator_parse.legal_name_ = operator_r.node().child("LegalName").text().as_string();
            operator_parse.public_code_ = operator_r.node().child("PublicCode").text().as_string();
            line_parse.operator_.push_back(operator_parse);
          }
        }
      }
      line_list.push_back(line_parse);
    }
  }
  return line_list ;
}

} //motis::loader::netex
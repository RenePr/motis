#pragma once

#include <iostream>
#include <map>
#include <set>

#include "pugixml.hpp"

#include "motis/loader/netex/service_frame.h"

namespace xml = pugi;

namespace motis::loader::netex {

//TODO void noch verändern
std::map<std::string ,line> parse_line(xml::xpath_node const&, std::map<std::string, Operator_Authority>&);

std::map<std::string, Operator_Authority> parse_operator(xml::xml_document&);

std::map<std::string, direction> parse_direction(xml::xpath_node const&);

std::map<std::string, std::string> parse_passenger_assignment(xml::xpath_node const&);

std::map<std::string, scheduled_points> parse_scheduled_points(xml::xpath_node const&, xml::xml_document&);


} //motis::loader::netex
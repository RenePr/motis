#pragma once

#include <iostream>
#include <map>
#include <set>

#include "pugixml.hpp"

#include "service_frame.h"

namespace xml = pugi;

namespace motis::loader::netex {

std::map<std::string, line> parse_line(
    xml::xpath_node const&, std::map<std::string, Operator_Authority> const&);

std::map<std::string, Operator_Authority> parse_operator(xml::xml_document&);

std::map<std::string, direction> parse_direction(xml::xpath_node const&);

std::map<std::string, passenger_assignments> parse_passenger_assignment(
    xml::xpath_node const&);

std::map<std::string, scheduled_points> parse_scheduled_points(
    xml::xpath_node const&, xml::xml_document&);

void parse_frame(xml::xml_document&, std::map<std::string, line>&,
                 std::map<std::string, scheduled_points>&,
                 std::map<std::string, direction>&,
                 std::map<std::string, passenger_assignments>&);

}  // namespace motis::loader::netex
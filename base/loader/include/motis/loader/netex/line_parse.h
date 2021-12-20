#pragma once

#include <iostream>
#include <map>
#include <set>

#include "pugixml.hpp"

#include "motis/loader/netex/line.h"

namespace xml = pugi;

namespace motis::loader::netex {

//TODO void noch verändern
std::map<std::string ,line> parse_line(xml::xpath_node const& service_jorney, std::map<std::string, Operator_Authority>& operator_map);

std::map<std::string, Operator_Authority> parse_operator(xml::xml_document& d);


} //motis::loader::netex
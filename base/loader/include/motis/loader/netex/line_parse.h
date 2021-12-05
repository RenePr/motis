#pragma once

#include <iostream>
#include <vector>

#include "pugixml.hpp"

#include "motis/loader/netex/netex_parser.h"
#include "motis/loader/netex/service_journey.h"
#include "motis/loader/netex/line.h"

namespace xml = pugi;

namespace motis::loader::netex {

//TODO void noch verändern
std::vector<line> parse_line(xml::xpath_node const& node, xml::xml_document& d );


} //motis::loader::netex
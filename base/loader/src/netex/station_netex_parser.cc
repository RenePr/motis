// Created by Rene Prinz on 08.11.21.
//

#include "motis/loader/netex/station_netex_parser.h"

#include <iostream>

#include "pugixml.hpp"

#include "utl/verify.h"
#include "utl/get_or_create.h"

namespace motis::loader::netex {

std::map<std::string, stop> parse_xml_stops(xml::xpath_node_set stop_nodes) {
  for (auto const& stop : stop_nodes) {
    // std::cout << "     " << stop.node().child("Name").child_value() << "\n";
    // Stoplace id
    std::cout << "     " << stop.node().attribute("id").value() << "\n";
  }
}
} // namespace motis::loader::nete
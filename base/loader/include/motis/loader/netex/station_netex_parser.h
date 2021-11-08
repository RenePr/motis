#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "geo/latlng.h"
#include "geo/point_rtree.h"

#include "pugixml.hpp"

#include "motis/loader/loaded_file.h"

namespace xml = pugi;

namespace motis::loader::netex {

struct stop {

  std::string id_;
  std::string name_;
};

std::map<std::string, stop> parse_xml_stops(xml::xpath_node_set stop_nodes);

} // namespace motis::loader::netex

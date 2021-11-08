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

struct station_netex {

  std::string id_;
  std::string name_;
};
//using station_map = std::map<std::string, std::unique_ptr<station_netex>>;

std::map<std::string, station_netex> parse_xml_station(xml::xpath_node_set const& station_nodes);
} // namespace motis::loader::netex

// Created by Rene Prinz on 08.11.21.
//

#include "motis/loader/netex/station_netex_parser.h"

#include <iostream>
#include <algorithm>
#include <string>
#include <tuple>
#include "motis/hash_map.h"

#include "pugixml.hpp"

#include "utl/get_or_create.h"

namespace motis::loader::netex {
//TODO station_netex noch  nicht fertig, erstmal damit die struktur klar ist
std::map<std::string, station_netex> parse_xml_station(xml::xpath_node_set const& station_nodes) {
  std::map<std::string, station_netex> stations;
  for (auto const& station : station_nodes) {
    station_netex st1;
    st1.id_ = station.node().attribute("id").value();
    st1.name_ = station.node().child("Name").text().get();
    //stations.insert(st1.id_ ,st1);

    stations.insert(std::pair<std::string, station_netex>(st1.id_, st1));
    std::cout << "     " << st1.id_ << st1.name_ << stations.size()<< "\n";
    /*auto const new_station =
        utl::get_or_create(stations, station.node().attribute("id").value(), [&]() {
          return std::make_unique<station_netex>();
        });
     //new_station->id_ = station.node().attribute("id").value();
     //new_station->name_ = station.node().child("Name").text().get();
     new_station->name_ = station.node().child("Name").text().get();

     station_map.insert(std::pair<std::string, station_netex>(st1.id_, st1));

     //std::cout << "     " << st1.id_ << st1.name_ << "\n"; */

  }
  return stations;
}

} // namespace motis::loader::nete
#pragma once

#include <map>
#include <iostream>

#include "motis/schedule-format/Station_generated.h"

#include "motis/loader/netex/station_netex_parser.h"

namespace fbs64 = flatbuffers64;

namespace motis::loader::netex {

struct netex_station_builder {
  netex_station_builder(std::map<std::string, station_netex>);

  fbs64::Offset<station_netex> get_or_create_station(
      std::string, fbs64::FlatBufferBuilder&);

  std::map<std::string, station_netex> netex_stations_;
  //timezones timezones_;
  std::map<std::string, flatbuffers64::Offset<station_netex>> fbs_stations_;
  //std::map<timezone_entry const*, flatbuffers64::Offset<Timezone>>
  //    fbs_timezones_;
};

}  // namespace motis::loader::netex
#include "motis/loader/netex/station_netex_builder.h"

#include "motis/loader/netex/station_netex_parser.h"

#include <map>
#include <iostream>

#include "motis/schedule-format/Station_generated.h"

#include "motis/loader/netex/station_netex_parser.h"

#include "utl/get_or_create.h"

using namespace flatbuffers64;

namespace motis::loader::netex {

  netex_station_builder::netex_station_builder(std::map<std::string, station_netex> netex_stations) : netex_station_(std::move(netex_stations)){
  };

  Offset<station_netex>  netex_station_builder::get_or_create_station(
      std::string id, flatbuffers64::FlatBufferBuilder& fbb) {
      return utl::get_or_create(fbs_stations_, id, [&]() {
        auto it = netex_station_.find(id);
        utl::verify(it != end(netex_station_), "missing id from netex station"),
        return CreateStation(
              fbb, to_fbs_string(fbb, to_fbs_string(id)),
              to_fbs_string(fbb, it->second.name_ ),
              to_fbs_string(fbb, ""),
              to_fbs_string(fbb, ""),
              to_fbs_string(fbb, 0),
              to_fbs_string(fbb, ""),
              to_fbs_string(fbb, ""),
              to_fbs_string(fbb, "")
            );
        });
  }

  std::map<std::string, station_netex> netex_stations_;
  //timezones timezones_;
  std::map<std::string, flatbuffers64::Offset<station_netex>> fbs_stations_;
  //std::map<timezone_entry const*, flatbuffers64::Offset<Timezone>>
  //    fbs_timezones_;


}  // namespace motis::loader::netex
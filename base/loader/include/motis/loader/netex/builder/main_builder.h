#pragma once

#include <iostream>
#include <map>
#include <string_view>
#include <vector>

#include "pugixml.hpp"

#include "motis/loader/hrd/model/timezones.h"
#include "motis/loader/hrd/parser/stations_parser.h"

#include "motis/schedule-format/Station_generated.h"

#include "motis/loader/netex/builder/helper_builder.h"
#include "motis/loader/netex/days.h"
#include "motis/loader/netex/service_frame/service_frame.h"
#include "motis/loader/netex/service_journey/service_journey.h"
#include "motis/loader/netex/service_journey_interchange/service_journey_interchange.h"
#include "motis/loader/util.h"

namespace fbs64 = flatbuffers64;
namespace xml = pugi;

namespace motis::loader::netex {

struct build {
  std::map<std::string, line> l_m_;
  std::map<std::string, scheduled_points> s_m_;
  std::map<std::string, direction> d_m_;
  std::map<std::string, passenger_assignments> p_m_;

  std::map<std::string, ids> days_m_;

  std::map<std::string, service_journey_pattern> sjp_m_;
  std::map<std::string, service_journey> sj_m_;
  std::vector<service_journey_interchange> sji_v_;

  std::string_view file_;
};

void build_fbs(build const&, std::vector<fbs64::Offset<Route>>&,
               std::vector<fbs64::Offset<Service>>&,
               std::map<std::string, fbs64::Offset<Station>>&,
               std::vector<fbs64::Offset<RuleService>>&,
               fbs64::FlatBufferBuilder&);

}  // namespace motis::loader::netex
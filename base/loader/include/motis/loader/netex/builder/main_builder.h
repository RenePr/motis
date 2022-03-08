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
#include "motis/loader/netex/days/days.h"
#include "motis/loader/netex/service_frame/service_frame.h"
#include "motis/loader/netex/service_journey/service_journey.h"
#include "motis/loader/netex/service_journey_interchange/service_journey_interchange.h"
#include "motis/loader/util.h"

namespace fbs64 = flatbuffers64;
namespace xml = pugi;

namespace motis::loader::netex {

struct build {
  std::map<std::string, line> l_m_;
  std::map<std::string, direction> d_m_;
  std::map<std::string, stations_direction> stations_map_;

  std::map<std::string, ids> days_m_;
  std::map<std::string, season> seasons_m_;
  std::map<std::string, service_journey_pattern> sjp_m_;
  std::map<std::string, service_journey> sj_m_;

  std::string_view file_;
};
struct service_journey_parse {
  std::vector<int> times_v_;
  std::vector<routes> routes_;
  std::vector<fbs64::Offset<Attribute>> a_v_;
  fbs64::Offset<Category> category_;
  fbs64::Offset<Provider> provider_;
  std::string key_sj_;
};

void build_fbs(build const&, std::vector<service_journey_parse>&,
               fbs64::FlatBufferBuilder&);
void create_stations_routes_services_fbs(
    std::vector<service_journey_parse> const&, std::string const&,
    std::map<std::string, fbs64::Offset<Station>>&,
    std::vector<fbs64::Offset<Route>>&,
    std::map<std::string, fbs64::Offset<Service>>&, fbs64::FlatBufferBuilder&);
void create_rule_service(std::vector<service_journey_interchange> const&,
                         std::map<std::string, fbs64::Offset<Service>> const&,
                         std::map<std::string, fbs64::Offset<Station>> const&,
                         std::vector<fbs64::Offset<RuleService>>&,
                         fbs64::FlatBufferBuilder&);

}  // namespace motis::loader::netex
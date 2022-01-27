#pragma once

#include <map>
#include <vector>

#include "motis/loader/hrd/model/timezones.h"
#include "motis/loader/hrd/parser/stations_parser.h"

#include "motis/schedule-format/Schedule_generated.h"

#include "motis/loader/netex/days.h"
#include "motis/loader/netex/service_frame/service_frame.h"
#include "motis/loader/netex/service_journey/service_journey.h"
#include "motis/loader/netex/service_journey_pattern/service_journey_pattern.h"

namespace fbs64 = flatbuffers64;

namespace motis::loader::netex {

struct station_dir_section {
  std::map<std::string, stop_point_in_journey_pattern> s_p_m_;
  std::map<std::string, scheduled_points> s_m_;
  std::map<std::string, line> l_m_;
  std::pair<std::string, std::string> traffic_days;
  time_table_passing_time ttpt_;
  std::string start_time_;
  std::string direction_;
  fbs64::Offset<Category> category_;
  fbs64::Offset<Provider> provider_;
  fbs64::Offset<Timezone> timezone_;
  std::vector<fbs64::Offset<Attribute>> a_v_;
};
int time_realtive_to_0(std::string const&, std::string const&);
int time_realtive_to_0_season(std::string const&, std::string const&);
// vector, normalerweise nur 1 eintrag.
std::pair<std::string, std::string> get_valid_day_bits(
    std::map<std::string, ids> const&, std::vector<std::string> const&);
// 3 const noch ok oder auch über struct?
void get_provider_operator_fbs(std::vector<std::string> const&,
                               std::map<std::string, line> const&,
                               fbs64::Offset<Category>&,
                               fbs64::Offset<Provider>&, int&,
                               fbs64::FlatBufferBuilder&);
void get_attribute_fbs(std::vector<std::string> const&,
                       std::vector<notice_assignment> const&,
                       std::map<std::string, ids> const&,
                       std::vector<fbs64::Offset<Attribute>>&,
                       fbs64::FlatBufferBuilder&);
void get_station_dir_section(station_dir_section const&, std::vector<int>&,
                             uint8_t&, uint8_t&, fbs64::Offset<Station>&,
                             fbs64::Offset<Direction>&, fbs64::Offset<Section>&,
                             fbs64::Offset<Track>&, fbs64::FlatBufferBuilder&);
// std::vector<fbs64::Offset<AttributeInfo>> get_attribute_info_fbs();

}  // namespace motis::loader::netex
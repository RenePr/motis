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

struct station_dir {
  std::map<std::string, stop_point_in_journey_pattern> s_p_m_;
  std::map<std::string, scheduled_points> s_m_;
  std::map<std::string, line> l_m_;
  std::pair<std::string, std::string> traffic_days;
  std::string stop_point_ref_;
  std::string direction_;
  fbs64::Offset<Timezone> timezone_;
};
struct build_sec {
  fbs64::Offset<Category> category_;
  fbs64::Offset<Provider> provider_;
  std::vector<fbs64::Offset<Attribute>> a_v_;
  std::string line_id;
  fbs64::Offset<Direction> direction_;
};

struct stations_direction {
  std::string id_;
  std::string name_;
  double lat_;
  double lng_;
  fbs64::Offset<Timezone> timezone_;
  std::string timezone_name_;
  std::string direction_;
};

struct ttpt_index {
  stations_direction st_dir_;
  uint8_t in_allowed_;
  uint8_t out_allowed_;
  std::string quay_;
  std::string stop_point_ref_;
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
void get_service_times(time_table_passing_time const&, std::string const&,
                       std::vector<int>&);
void get_section_fbs(build_sec const&, fbs64::Offset<Section>&,
                     fbs64::FlatBufferBuilder&);
void get_station_dir_fbs(stations_direction const&, fbs64::Offset<Station>&,
                         fbs64::Offset<Direction>&, fbs64::FlatBufferBuilder&);

}  // namespace motis::loader::netex
#pragma once

#include <iostream>
#include <vector>

namespace motis::loader::netex {

struct station_sj {
};

struct time_table_passing_time {
  std::string stop_point_in_journey_pattern_ref_;

};

struct service_journey{
  std::string key_sjp_;
  std::vector<std::string> keys_day_;
  std::vector<std::string> keys_ttpt_;

};

} //motis::loader::netex
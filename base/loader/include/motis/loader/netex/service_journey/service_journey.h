#pragma once

#include <iostream>
#include <vector>

namespace motis::loader::netex {

struct station_sj {};

struct time_table_passing_time {
  std::string stop_point_ref;
  std::string dep_time;
  std::string arr_time;
};

struct service_journey {
  std::string key_sjp_;
  std::vector<std::string> keys_day_;
  std::vector<time_table_passing_time> keys_ttpt_;
};

}  // namespace motis::loader::netex
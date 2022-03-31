#pragma once

#include <iostream>
#include <vector>

namespace motis::loader::netex {

struct time_table_passing_time {
  std::string stop_point_ref_;
  std::string dep_time_;
  std::string arr_time_;
};
struct service_journey {
  std::string key_sjp_;
  std::string key_sj_;
  std::vector<std::string> keys_day_;
  std::vector<time_table_passing_time> keys_ttpt_;
};

}  // namespace motis::loader::netex
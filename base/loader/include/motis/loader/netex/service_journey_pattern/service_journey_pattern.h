#pragma once

#include <iostream>

#include "motis/schedule-format/Schedule_generated.h"

namespace fbs64 = flatbuffers64;

namespace motis::loader::netex {

struct stop_point_in_journey_pattern {
  std::string id_;
  bool in_allowed_;
  bool out_allowed_;
};
struct notice_assignment {
  std::string text_;
  std::string public_code_;
  std::string start_point_in_journey_pattern_;
  std::string stop_point_in_journey_pattern_;
};
struct service_journey_pattern {
  std::vector<notice_assignment> notice_assignments_;
  std::vector<std::string> lines_;
  std::map<std::string, stop_point_in_journey_pattern> stop_point_map_;
  int name_;
  std::string direction_;
};

}  // namespace motis::loader::netex
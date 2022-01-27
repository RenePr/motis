#pragma once

#include <iostream>
#include <vector>

namespace motis::loader::netex {

struct service_journey_interchange{
 std::string from_station_;
 std::string to_station_;
 std::string from_journey_;
 std::string to_journey_;

};

}  // namespace motis::loader::netex
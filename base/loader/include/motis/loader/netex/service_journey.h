#pragma once

#include <iostream>

#include "motis/loader/netex/service_journey_pattern.h"

namespace motis::loader::netex {

struct service_journey {
  std::string day_type_ref_;
  std::string service_journey_pattern_ref_;
  motis::loader::netex::service_journey_pattern journey_pattern;
};

} //motis::loader::netex
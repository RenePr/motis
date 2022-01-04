#pragma once

#include <iostream>

#include "motis/schedule-format/Schedule_generated.h"

namespace fbs64 = flatbuffers64;

namespace motis::loader::netex {

struct service_journey_pattern {
  std::vector<fbs64::Offset<Station>> stations_vec_;
  std::vector<fbs64::Offset<AttributeInfo>> attributeinfo_vec_;
  //fbs64::Offset<Season> season_;
  //fbs64::Offset<Timezone> timezone_;
  fbs64::Offset<Provider> provider_;
  fbs64::Offset<Category> category_;

  int name_;
  std::string direction_;
};

} //motis::loader::netex
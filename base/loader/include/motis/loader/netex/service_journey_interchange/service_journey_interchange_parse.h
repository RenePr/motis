#pragma once

#include "pugixml.hpp"

#include <iostream>
#include <vector>

#include "motis/loader/netex/service_journey_interchange/service_journey_interchange.h"
#include "motis/schedule-format/Schedule_generated.h"

namespace fbs64 = flatbuffers64;
namespace xml = pugi;

namespace motis::loader::netex {

void parse_service_journey_interchange(
    xml::xml_document&, std::vector<service_journey_interchange>&);

}  // namespace motis::loader::netex
#pragma once

#include "pugixml.hpp"

#include <iostream>
#include <map>

#include "motis/schedule-format/Schedule_generated.h"
#include "motis/loader/netex/service_journey_pattern/service_journey_pattern.h"

namespace fbs64 = flatbuffers64;
namespace xml = pugi;

namespace motis::loader::netex {

void parse_service_journey_pattern(xml::xml_document&, std::map<std::string, service_journey_pattern>&);

} //motis::loader::netex
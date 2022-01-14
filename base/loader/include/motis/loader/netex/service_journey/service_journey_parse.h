#pragma once

#include "pugixml.hpp"

#include <iostream>
#include <map>

#include "motis/schedule-format/Schedule_generated.h"
#include "motis/loader/netex/service_journey/service_journey.h"

namespace fbs64 = flatbuffers64;
namespace xml = pugi;

namespace motis::loader::netex {

void parse_service_journey(xml::xml_document&, std::map<std::string, service_journey>&);

} //motis::loader::netex
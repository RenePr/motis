#pragma once

#include "pugixml.hpp"

#include <iostream>
#include <map>

#include "motis/schedule-format/Schedule_generated.h"
#include "motis/loader/netex/service_journey_pattern/service_journey_pattern.h"
#include "motis/loader/netex/builder/main_builder.h"

namespace fbs64 = flatbuffers64;
namespace xml = pugi;

namespace motis::loader::netex {

void verfiy_xml_header(xml::xml_document&);
void verfiy_build(build const&);

} //motis::loader::netex
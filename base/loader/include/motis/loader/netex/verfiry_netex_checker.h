#pragma once

#include "pugixml.hpp"

#include <iostream>
#include <map>

#include "motis/loader/netex/builder/main_builder.h"
#include "motis/loader/netex/service_journey_pattern/service_journey_pattern.h"
#include "motis/schedule-format/Schedule_generated.h"

namespace fbs64 = flatbuffers64;
namespace xml = pugi;

namespace motis::loader::netex {

void verfiy_xml_header(xml::xml_document&);
bool verfiy_build(build const&);

}  // namespace motis::loader::netex
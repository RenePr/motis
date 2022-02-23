#pragma once

#include <iostream>
#include <map>

#include "motis/loader/netex/days/days.h"

#include "pugixml.hpp"

namespace xml = pugi;

namespace motis::loader::netex {

std::map<std::string, daytype> transform_to_map_daytype(xml::xml_document&);

std::map<std::string, uic_opertion_period> transform_to_map_uic_opertion(
    xml::xml_document&);

std::map<std::string, ids> parse_daytypes_uicoperation(xml::xml_document&);

}  // namespace motis::loader::netex
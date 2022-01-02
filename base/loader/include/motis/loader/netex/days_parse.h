#pragma once

#include <iostream>
#include <map>

#include "motis/loader/netex/days.h"

#include "pugixml.hpp"

namespace xml = pugi;

namespace motis::loader::netex {

std::map<std::string, daytype> transform_to_map_daytype(xml::xml_document&);

std::map<std::string, uic_opertion_period> transform_to_map_uic_opertion(xml::xml_document&);

std::map<std::string, ids> combine_daytyps_uic_opertions(xml::xml_document&);

} //motis::loader::netex
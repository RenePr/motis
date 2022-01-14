#pragma once

#include <map>
#include <vector>

#include "motis/loader/hrd/model/timezones.h"
#include "motis/loader/hrd/parser/stations_parser.h"

#include "motis/schedule-format/Schedule_generated.h"

#include "motis/loader/netex/days.h"
#include "motis/loader/netex/service_frame/service_frame.h"
#include "motis/loader/netex/service_journey_pattern/service_journey_pattern.h"

namespace fbs64 = flatbuffers64;

namespace motis::loader::netex {

std::string get_valid_day_bits(std::map<std::string, ids> const&, std::string);
void get_provider_operator_fbs(std::vector<std::string> const&,
                               std::map<std::string, line> const&,
                               fbs64::Offset<Category>&,
                               fbs64::Offset<Provider>&, int&,
                               fbs64::FlatBufferBuilder&);

void get_attribute_fbs(std::vector<std::string> const&,
                       std::vector<notice_assignment> const&,
                       std::map<std::string, ids> const&,
                       std::vector<fbs64::Offset<Attribute>>&,
                       fbs64::FlatBufferBuilder&);
// std::vector<fbs64::Offset<AttributeInfo>> get_attribute_info_fbs();

}  // namespace motis::loader::netex
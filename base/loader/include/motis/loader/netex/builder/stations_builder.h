#pragma once

#include <map>

#include "motis/loader/hrd/model/timezones.h"
#include "motis/loader/hrd/parser/stations_parser.h"

#include "motis/schedule-format/Station_generated.h"

#include "motis/loader/netex/service_frame/service_frame.h"

namespace fbs64 = flatbuffers64;

namespace motis::loader::netex {
#include "motis/loader/netex/builder/helper_builder.h"

#include <iostream>
#include <map>
#include <string_view>

#include "pugixml.hpp"

#include "motis/loader/netex/days.h"
#include "motis/loader/util.h"

namespace xml = pugi;

namespace motis::loader::netex {}  // namespace motis::loader::netex
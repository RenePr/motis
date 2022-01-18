#include "gtest/gtest.h"

#include <functional>

#include "utl/verify.h"

#include "boost/filesystem.hpp"

#include "pugixml.hpp"

#include "motis/loader/netex/service_journey_pattern/service_journey_pattern.h"
#include "motis/loader/netex/service_journey_pattern/service_journey_pattern_parse.h"

#include "motis/schedule-format/Schedule_generated.h"

namespace fbs64 = flatbuffers64;
namespace xml = pugi;
namespace fs = boost::filesystem;
using namespace motis::loader::netex;

TEST(service_journey_pattern_parse, service_journey_p) {

  try {
    const char* file =
        "base/loader/test_resources/netex_schedules/"
        "NX-PI-01_DE_NAP_LINE_123-ERLBUS-371_20211029_line.xml";
    xml::xml_document d;
    auto r = d.load_file(file);
    auto sjp_m = std::map<std::string, service_journey_pattern>{};
    parse_service_journey_pattern(d, sjp_m);
    ASSERT_TRUE(
        std::string_view("DE::Direction:H::") ==
        std::string_view(
            sjp_m.at("DE::ServiceJourneyPattern:105870562_0::").direction_));
  } catch (std::exception& e) {
  }
}
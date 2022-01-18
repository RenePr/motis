#include "gtest/gtest.h"

#include <functional>

#include "utl/verify.h"

#include "boost/filesystem.hpp"

#include "pugixml.hpp"

#include "motis/loader/netex/service_journey/service_journey.h"
#include "motis/loader/netex/service_journey/service_journey_parse.h"

#include "motis/schedule-format/Schedule_generated.h"

namespace fbs64 = flatbuffers64;
namespace xml = pugi;
namespace fs = boost::filesystem;
using namespace motis::loader::netex;

TEST(service_journey_parse, service_journey) {

  try {
    const char* file =
        "base/loader/test_resources/netex_schedules/"
        "NX-PI-01_DE_NAP_LINE_123-ERLBUS-371_20211029_line.xml";
    xml::xml_document d;
    auto r = d.load_file(file);
    auto sj_m = std::map<std::string, service_journey>{};
    parse_service_journey(d, sj_m);
    ASSERT_TRUE(std::string_view("DE::ServiceJourneyPattern:105870562_0::") ==
                std::string_view(
                    sj_m.at("DE::ServiceJourney:1198650792_0::").key_sjp_));
  } catch (std::exception& e) {
  }
}
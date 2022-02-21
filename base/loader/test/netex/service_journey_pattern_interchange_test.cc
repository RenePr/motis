#include "gtest/gtest.h"

#include <functional>

#include "utl/verify.h"

#include "boost/filesystem.hpp"

#include "pugixml.hpp"

#include "motis/loader/netex/service_journey_interchange/service_journey_interchange.h"
#include "motis/loader/netex/service_journey_interchange/service_journey_interchange_parse.h"

#include "motis/schedule-format/Schedule_generated.h"

namespace fbs64 = flatbuffers64;
namespace xml = pugi;
namespace fs = boost::filesystem;
using namespace motis::loader::netex;

TEST(service_journey_interchange_test, service_journey_interchange_empty) {
  try {
    const char* file =
        "base/loader/test_resources/netex_schedules/"
        "NX-PI-01_DE_NAP_LINE_123-ERLBUS-371_20211029_line.xml";
    auto d = xml::xml_document{};
    auto r = d.load_file(file);
    auto sji_v = std::vector<service_journey_interchange>{};
    parse_service_journey_interchange(d, sji_v);
    ASSERT_TRUE(0 == sji_v.size());
  } catch (std::exception& e) {
  }
}

TEST(service_journey_interchange_test, service_journey_interchange) {
  try {
    const char* file =
        "base/loader/test_resources/netex_schedules/"
        "NX-PI-01_DE_NAP_LINE_126-HEAGTRAM-2_20220104.xml";
    auto d = xml::xml_document{};
    auto r = d.load_file(file);
    auto sji_v = std::vector<service_journey_interchange>{};
    parse_service_journey_interchange(d, sji_v);
    ASSERT_TRUE(107 == sji_v.size());
    ASSERT_TRUE(std::string_view("DE::ScheduledStopPoint:10473401_126_::") ==
                std::string_view(sji_v.front().from_station_));
    ASSERT_TRUE(std::string_view("DE::ScheduledStopPoint:10473401_126_::") ==
                std::string_view(sji_v.front().to_station_));
    ASSERT_TRUE(std::string_view("DE::ServiceJourney:1601499506_0::") ==
                std::string_view(sji_v.front().from_journey_));
    ASSERT_TRUE(std::string_view("DE::ServiceJourney:1601574915_0::") ==
                std::string_view(sji_v.front().to_journey_));
    ASSERT_TRUE(true == sji_v.front().stay_seated_);
  } catch (std::exception& e) {
  }
}
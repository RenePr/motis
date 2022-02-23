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

TEST(service_journey_pattern_parse, service_journey_pattern) {
  try {
    const char* file =
        "base/loader/test_resources/netex_schedules/"
        "NX-PI-01_DE_NAP_LINE_126-HEAGTRAM-2_20220104.xml";
    xml::xml_document d;
    auto r = d.load_file(file);
    auto sjp_m = std::map<std::string, service_journey_pattern>{};
    parse_service_journey_pattern(d, sjp_m);
    ASSERT_TRUE(20 == sjp_m.size());
    ASSERT_TRUE(
        std::string_view("DE::Direction:1::") ==
        std::string_view(
            sjp_m.at("DE::ServiceJourneyPattern:150268219_0::").direction_));
    ASSERT_TRUE(7 == sjp_m.at("DE::ServiceJourneyPattern:150268219_0::")
                         .notice_assignments_.size());
    ASSERT_TRUE(
        std::string_view("SteFa") ==
        std::string_view(sjp_m.at("DE::ServiceJourneyPattern:150268219_0::")
                             .notice_assignments_.front()
                             .public_code_));
    ASSERT_TRUE(
        std::string_view(
            "Fahrzeug hat Mehrzweckfläche / Rollstuhlstellplatz") ==
        std::string_view(sjp_m.at("DE::ServiceJourneyPattern:150268219_0::")
                             .notice_assignments_.front()
                             .text_));
    ASSERT_TRUE(
        std::string_view("DE::StopPointInJourneyPattern:150268219_1_0::") ==
        std::string_view(sjp_m.at("DE::ServiceJourneyPattern:150268219_0::")
                             .notice_assignments_.front()
                             .start_point_in_journey_pattern_));
    ASSERT_TRUE(
        std::string_view("DE::StopPointInJourneyPattern:150268219_1_6::") ==
        std::string_view(sjp_m.at("DE::ServiceJourneyPattern:150268219_0::")
                             .notice_assignments_.front()
                             .stop_point_in_journey_pattern_));
    ASSERT_TRUE(
        1 == sjp_m.at("DE::ServiceJourneyPattern:150268219_0::").lines_.size());
    ASSERT_TRUE(
        std::string_view("DE::Line:71301::") ==
        std::string_view(sjp_m.at("DE::ServiceJourneyPattern:150268219_0::")
                             .lines_.front()));
    ASSERT_TRUE(7 == sjp_m.at("DE::ServiceJourneyPattern:150268219_0::")
                         .stop_point_map_.size());
    ASSERT_TRUE(
        std::string_view("DE::ScheduledStopPoint:11597001_126_::") ==
        sjp_m.at("DE::ServiceJourneyPattern:150268219_0::")
            .stop_point_map_.at("DE::StopPointInJourneyPattern:150268219_1_1::")
            .id_);
    ASSERT_TRUE(true == sjp_m.at("DE::ServiceJourneyPattern:150268219_0::")
                            .stop_point_map_
                            .at("DE::StopPointInJourneyPattern:150268219_1_0::")
                            .in_allowed_);
    ASSERT_TRUE(true == sjp_m.at("DE::ServiceJourneyPattern:150268219_0::")
                            .stop_point_map_
                            .at("DE::StopPointInJourneyPattern:150268219_1_0::")
                            .out_allowed_);
  } catch (std::exception& e) {
    std::cout << e.what();
  }
}
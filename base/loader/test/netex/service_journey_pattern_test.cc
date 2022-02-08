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
    ASSERT_TRUE(
        1 ==
            sjp_m.at("DE::ServiceJourneyPattern:105870562_0::").notice_assignments_.size());
    ASSERT_TRUE(
        1 ==
        sjp_m.at("DE::ServiceJourneyPattern:105870562_0::").lines_.size());
    //TODO checken
    ASSERT_TRUE(
        371 ==
        sjp_m.at("DE::ServiceJourneyPattern:105870562_0::").name_);

    ASSERT_TRUE(
        1 ==
        sjp_m.at("DE::ServiceJourneyPattern:105870562_0::").stop_point_map_.size());
    ASSERT_TRUE(
        std::string_view("DE::ScheduledStopPoint:100764207_123_::") ==
        sjp_m.at("DE::ServiceJourneyPattern:105870562_0::").stop_point_map_.at("DE::StopPointInJourneyPattern:105870567_1_0::").id_);
    ASSERT_TRUE(
        true ==
        sjp_m.at("DE::ServiceJourneyPattern:105870562_0::").stop_point_map_.at("DE::StopPointInJourneyPattern:105870567_1_0::").in_allowed_);
    ASSERT_TRUE(
        true ==
        sjp_m.at("DE::ServiceJourneyPattern:105870562_0::").stop_point_map_.at("DE::StopPointInJourneyPattern:105870567_1_0::").out_allowed_);
  } catch (std::exception& e) {
  }
}
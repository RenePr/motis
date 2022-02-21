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

TEST(service_journey_test, service_journey) {
  try {
    const char* file =
        "base/loader/test_resources/netex_schedules/"
        "NX-PI-01_DE_NAP_LINE_123-ERLBUS-371_20211029_line.xml";
    xml::xml_document d;
    auto r = d.load_file(file);
    auto sj_m = std::map<std::string, service_journey>{};
    parse_service_journey(d, sj_m);
    // std::cout << sj_m.size() << "Here?" << std::endl;
    //  ASSERT_TRUE(== sj_m.size());
    ASSERT_TRUE(
        std::string_view("DE::ServiceJourney:1198650792_0::") ==
        std::string_view(sj_m.at("DE::ServiceJourney:1198650792_0::").key_sj_));
    ASSERT_TRUE(std::string_view("DE::ServiceJourneyPattern:105870562_0::") ==
                std::string_view(
                    sj_m.at("DE::ServiceJourney:1198650792_0::").key_sjp_));
    ASSERT_TRUE(1 ==
                sj_m.at("DE::ServiceJourney:1198650792_0::").keys_day_.size());
    ASSERT_TRUE(
        std::string_view("DE::DayType:128678::") ==
        std::string_view(
            sj_m.at("DE::ServiceJourney:1198650792_0::").keys_day_.front()));
    ASSERT_TRUE(12 ==
                sj_m.at("DE::ServiceJourney:1198650792_0::").keys_ttpt_.size());
    ASSERT_TRUE(
        std::string_view("DE::StopPointInJourneyPattern:105870562_1_0::") ==
        std::string_view(sj_m.at("DE::ServiceJourney:1198650792_0::")
                             .keys_ttpt_.front()
                             .stop_point_ref_));
    ASSERT_TRUE(std::string_view("06:35:00") ==
                std::string_view(sj_m.at("DE::ServiceJourney:1198650792_0::")
                                     .keys_ttpt_.front()
                                     .dep_time_));
    ASSERT_TRUE(std::string_view("") ==
                std::string_view(sj_m.at("DE::ServiceJourney:1198650792_0::")
                                     .keys_ttpt_.front()
                                     .arr_time_));

  } catch (std::exception& e) {
  }
}
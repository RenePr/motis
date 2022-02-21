#include "gtest/gtest.h"

#include <functional>

#include "utl/verify.h"

#include "boost/filesystem.hpp"

#include "pugixml.hpp"

#include "motis/loader/netex/builder/helper_builder.h"
#include "motis/loader/netex/builder/main_builder.h"
#include "motis/loader/netex/days_parse.h"
#include "motis/loader/netex/service_frame/service_frame.h"
#include "motis/loader/netex/service_frame/service_frame_parse.h"
#include "motis/loader/netex/service_journey/service_journey.h"
#include "motis/loader/netex/service_journey/service_journey_parse.h"
#include "motis/loader/netex/service_journey_interchange/service_journey_interchange.h"
#include "motis/loader/netex/service_journey_interchange/service_journey_interchange_parse.h"
#include "motis/loader/netex/service_journey_pattern/service_journey_pattern.h"
#include "motis/loader/netex/service_journey_pattern/service_journey_pattern_parse.h"

#include "motis/schedule-format/Schedule_generated.h"

namespace fbs64 = flatbuffers64;
namespace xml = pugi;
namespace fs = boost::filesystem;
using namespace motis::loader::netex;

TEST(main_builder_test, main_builder_empty) {

  try {
    const char* file =
        "base/loader/test_resources/netex_schedules/"
        "NX-PI-01_DE_NAP_LINE_123-ERLBUS-371_20211029_line.xml";
    xml::xml_document d;
    auto fbb = fbs64::FlatBufferBuilder{};
    auto r = d.load_file(file);
    auto l_m = std::map<std::string, line>{};
    auto s_m = std::map<std::string, scheduled_points>{};
    auto d_m = std::map<std::string, direction>{};
    auto p_m = std::map<std::string, passenger_assignments>{};
    parse_frame(d, l_m, s_m, d_m, p_m);
    auto sjp_m = std::map<std::string, service_journey_pattern>{};
    parse_service_journey_pattern(d, sjp_m);
    auto sj_m = std::map<std::string, service_journey>{};
    parse_service_journey(d, sj_m);
    auto sji_v = std::vector<service_journey_interchange>{};
    parse_service_journey_interchange(d, sji_v);
    // ServiceCalendarFrame, so ist ja coby Rückgabe und eher schlecht?
    auto const days_m = parse_daytypes_uicoperation(d);
    auto const season_m = get_season_times(days_m);

    auto b = build{};
    b.l_m_ = l_m;
    b.s_m_ = s_m;
    b.d_m_ = d_m;
    b.p_m_ = p_m;
    b.sjp_m_ = sjp_m;
    b.sj_m_ = sj_m;
    b.days_m_ = days_m;
    b.seasons_m_ = season_m;
    b.file_ = std::string(file);
    auto sjpp = std::vector<service_journey_parse>{};
    build_fbs(b, sjpp, fbb);
    ASSERT_TRUE(112 == sjpp.size());
    ASSERT_TRUE(false == sjpp.front().a_v_.empty());
    std::cout << sjpp.front().a_v_.size() << std::endl;
    ASSERT_TRUE(1 == sjpp.front().a_v_.size());
    ASSERT_TRUE(std::string_view("DE::ServiceJourney:1198650792_0::") ==
                std::string_view(sjpp.front().key_sj_));
    ASSERT_TRUE(-1 == sjpp.front().times_v_.front());
    ASSERT_TRUE(-1 == sjpp.front().times_v_.back());
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }
}
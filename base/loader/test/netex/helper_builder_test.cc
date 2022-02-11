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

TEST(helper_builder_test, time_relative_to_0_season_empty) {
  try {
    auto const time = std::string("");
    auto const min = time_realtive_to_0_season(time);
    ASSERT_TRUE(min == 0);
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }
}

TEST(helper_builder_test, time_relative_to_0_season_same_time) {
  try {
    auto const time = std::string("06:01:01");
    auto const sec = time_realtive_to_0_season(time);
    ASSERT_TRUE(sec == 21661);
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }
}

TEST(helper_builder_test, time_relative_to_0_season_same_wrong_format) {
  try {
    // TODO muss ich das noch anpassen?
    auto const time = std::string("06:01");
    auto const sec = time_realtive_to_0_season(time);
    std::cout << sec;
    ASSERT_TRUE(sec == 0);
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }
}
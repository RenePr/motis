#include "gtest/gtest.h"

#include <functional>

#include "utl/verify.h"

#include "boost/filesystem.hpp"

#include "pugixml.hpp"

#include "motis/loader/netex/builder/helper_builder.h"
#include "motis/loader/netex/builder/main_builder.h"
#include "motis/loader/netex/days/days_parse.h"
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
    auto time = std::string("2022-02-05");
    auto const sec = time_realtive_to_0_season(time);
    std::cout << sec << std::endl;
    ASSERT_TRUE(sec == 400);
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }
}

TEST(helper_builder_test, time_relative_to_0_season_same_wrong_format) {
  try {
    auto const time = std::string("06:00");
    auto const sec = time_realtive_to_0_season(time);
    ASSERT_TRUE(sec == 0);
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }
}

TEST(helper_builder_test, time_relative_to_0) {
  try {
    auto const time = std::string("06:01:00");
    auto const start_time = std::string("06:00:00");
    auto const min = time_realtive_to_0(time, start_time);
    ASSERT_TRUE(min == 1);
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }
}

TEST(helper_builder_test, get_service_times) {
  try {
    auto ttpt = time_table_passing_time{};
    ttpt.dep_time_ = std::string("06:01:00");
    auto ttpt2 = time_table_passing_time{};
    ttpt2.dep_time_ = std::string("06:05:00");
    ttpt2.arr_time_ = std::string("06:05:00");
    auto ttpt3 = time_table_passing_time{};
    ttpt3.arr_time_ = std::string("06:10:00");
    auto times_v = std::vector<int>{};
    get_service_times(ttpt, ttpt.dep_time_, times_v);
    get_service_times(ttpt2, ttpt.dep_time_, times_v);
    get_service_times(ttpt3, ttpt.dep_time_, times_v);

    ASSERT_TRUE(times_v.at(0) == -1);
    ASSERT_TRUE(times_v.at(1) == 0);
    ASSERT_TRUE(times_v.at(2) == 4);
    ASSERT_TRUE(times_v.at(3) == 4);
    ASSERT_TRUE(times_v.at(4) == 9);
    ASSERT_TRUE(times_v.at(5) == -1);
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }
}

TEST(helper_builder_test, t) {}
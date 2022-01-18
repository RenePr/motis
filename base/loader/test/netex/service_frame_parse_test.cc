#include "gtest/gtest.h"

#include <functional>

#include "utl/verify.h"

#include "boost/filesystem.hpp"

#include "motis/loader/netex/service_frame/service_frame.h"
#include "motis/loader/netex/service_frame/service_frame_parse.h"

#include "motis/schedule-format/Schedule_generated.h"

namespace fbs64 = flatbuffers64;
namespace fs = boost::filesystem;
using namespace motis::loader::netex;

TEST(service_frame_parse, line) {
  // TODO how can I start this test, is this even right?
  // Users/reneprinz/Documents/motis

  try {
    const char* file =
        "base/loader/test_resources/netex_schedules/"
        "NX-PI-01_DE_NAP_LINE_123-ERLBUS-371_20211029_line.xml";
    xml::xml_document d;
    auto r = d.load_file(file);
    auto l_m = std::map<std::string, line>{};
    auto s_m = std::map<std::string, scheduled_points>{};
    auto d_m = std::map<std::string, direction>{};
    parse_frame(d, l_m, s_m, d_m);
    std::string_view name = "371";
    std::string_view mode = "bus";
    ASSERT_TRUE(l_m.size() == 1);
    ASSERT_TRUE(l_m.at("DE::Line:138143::").name_ == 371);
    ASSERT_TRUE(l_m.at("DE::Line:138143::").short_name_ == name);
    ASSERT_TRUE(l_m.at("DE::Line:138143::").transport_mode_ == mode);

  } catch (std::exception const& e) {
  }
}

TEST(service_frame_parse, direction) {
  // TODO how can I start this test, is this even right?
  // Users/reneprinz/Documents/motis

  try {
    const char* file =
        "base/loader/test_resources/netex_schedules/"
        "NX-PI-01_DE_NAP_LINE_123-ERLBUS-371_20211029_line.xml";
    xml::xml_document d;
    auto r = d.load_file(file);
    auto l_m = std::map<std::string, line>{};
    auto s_m = std::map<std::string, scheduled_points>{};
    auto d_m = std::map<std::string, direction>{};
    parse_frame(d, l_m, s_m, d_m);
    std::string_view name = "H";
    std::string_view short_name = "H";
    std::string_view name2 = "R";
    std::string_view short_name2 = "R";
    ASSERT_TRUE(d_m.size() == 2);
    ASSERT_TRUE(d_m.at("DE::Direction:H::").name_ == name);
    ASSERT_TRUE(d_m.at("DE::Direction:H::").short_name_ == short_name);
    ASSERT_TRUE(d_m.at("DE::Direction:R::").name_ == name2);
    ASSERT_TRUE(d_m.at("DE::Direction:R::").short_name_ == short_name2);
  } catch (std::exception const& e) {
  }
}

TEST(service_frame_parse, scheduled_points) {
  // TODO how can I start this test, is this even right?
  // Users/reneprinz/Documents/motis

  try {
    const char* file =
        "base/loader/test_resources/netex_schedules/"
        "NX-PI-01_DE_NAP_LINE_123-ERLBUS-371_20211029_line.xml";
    xml::xml_document d;
    auto r = d.load_file(file);
    auto l_m = std::map<std::string, line>{};
    auto s_m = std::map<std::string, scheduled_points>{};
    auto d_m = std::map<std::string, direction>{};
    parse_frame(d, l_m, s_m, d_m);
    std::string_view short_name =
        "HE GI Allendorf Allendorf Treiser Straße Vtreis";
    std::string_view public_code = "---trstr";
    std::string_view stop_type = "busStation";
    ASSERT_TRUE(s_m.size() == 103);
    ASSERT_TRUE(s_m.at("DE::ScheduledStopPoint:100343104_123_::").short_name_ ==
                short_name);
    ASSERT_TRUE(s_m.at("DE::ScheduledStopPoint:100343104_123_::").stop_type_ ==
                stop_type);
    ASSERT_TRUE(
        s_m.at("DE::ScheduledStopPoint:100343104_123_::").public_code_ ==
        public_code);
  } catch (std::exception const& e) {
  }
}
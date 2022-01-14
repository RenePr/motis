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
  //TODO how can I start this test, is this even right?
  //Users/reneprinz/Documents/motis

  try {
    const char* file = "base/loader/test_resources/netex_schedules/NX-PI-01_DE_NAP_LINE_123-ERLBUS-371_20211029_line.xml";
    xml::xml_document d;
    auto r = d.load_file(file);
    auto operator_map = parse_operator(d);
    for(auto const& service_frame : d.select_nodes("//dataObjects/CompositeFrame/frames/ServiceFrame")) {
      auto const& lines =  parse_line(service_frame, operator_map);
      std::string_view name =  "371";
      std::string_view mode = "bus";
      ASSERT_TRUE(lines.size() == 1);
      ASSERT_TRUE(lines.at("DE::Line:138143::").name_ == 371);
      ASSERT_TRUE(lines.at("DE::Line:138143::").short_name_ == name);
      ASSERT_TRUE(lines.at("DE::Line:138143::").transport_mode_ == mode);
    }
  }
  catch(std::exception const& e) {

  }
}

TEST(service_frame_parse, direction) {
  //TODO how can I start this test, is this even right?
  //Users/reneprinz/Documents/motis

  try {
    const char* file = "base/loader/test_resources/netex_schedules/NX-PI-01_DE_NAP_LINE_123-ERLBUS-371_20211029_line.xml";
    xml::xml_document d;
    auto r = d.load_file(file);
    auto operator_map = parse_operator(d);
    for(auto const& service_frame : d.select_nodes("//dataObjects/CompositeFrame/frames/ServiceFrame")) {
      auto const& dir =  parse_direction(service_frame);
      std::string_view name =  "H";
      std::string_view short_name = "H";
      std::string_view name2 =  "R";
      std::string_view short_name2 = "R";
      ASSERT_TRUE(dir.size() == 2);
      ASSERT_TRUE(dir.at("DE::Direction:H::").name_ == name);
      ASSERT_TRUE(dir.at("DE::Direction:H::").short_name_ == short_name);
      ASSERT_TRUE(dir.at("DE::Direction:R::").name_ == name2);
      ASSERT_TRUE(dir.at("DE::Direction:R::").short_name_ == short_name2);
    }
  }
  catch(std::exception const& e) {

  }
}

TEST(service_frame_parse, scheduled_points) {
  //TODO how can I start this test, is this even right?
  //Users/reneprinz/Documents/motis

  try {
    const char* file = "base/loader/test_resources/netex_schedules/NX-PI-01_DE_NAP_LINE_123-ERLBUS-371_20211029_line.xml";
    xml::xml_document d;
    auto r = d.load_file(file);
    for(auto const& service_frame : d.select_nodes("//dataObjects/CompositeFrame/frames/ServiceFrame")) {
      auto const& scheduled_stop =  parse_scheduled_points(service_frame, d);
      std::string_view short_name = "HE GI Allendorf Allendorf Treiser Straße Vtreis";
      std::string_view public_code = "---trstr";
      std::string_view stop_type =  "busStation";
      ASSERT_TRUE(scheduled_stop.size() == 103);
      ASSERT_TRUE(scheduled_stop.at("DE::ScheduledStopPoint:100343104_123_::").short_name_ == short_name);
      ASSERT_TRUE(scheduled_stop.at("DE::ScheduledStopPoint:100343104_123_::").stop_type_ == stop_type);
      ASSERT_TRUE(scheduled_stop.at("DE::ScheduledStopPoint:100343104_123_::").public_code_ == public_code);
    }
  }
  catch(std::exception const& e) {

  }
}
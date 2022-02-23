#include "gtest/gtest.h"

#include <functional>

#include "pugixml.hpp"

#include "motis/loader/netex/service_frame/service_frame.h"
#include "motis/loader/netex/service_frame/service_frame_parse.h"

namespace xml = pugi;
using namespace motis::loader::netex;

TEST(service_frame_parse, line) {
  try {
    const char* file =
        "base/loader/test_resources/netex_schedules/"
        "NX-PI-01_DE_NAP_LINE_126-HEAGTRAM-2_20220104.xml";
    xml::xml_document d;
    auto r = d.load_file(file);
    auto l_m = std::map<std::string, line>{};
    auto s_m = std::map<std::string, scheduled_points>{};
    auto s_p_m = std::map<std::string, stop_point>{};
    auto d_m = std::map<std::string, direction>{};
    auto p_m = std::map<std::string, passenger_assignments>{};
    parse_frame(d, l_m, s_m, s_p_m, d_m, p_m);
    std::string_view name = "2";
    std::string_view mode = "tram";
    ASSERT_TRUE(l_m.size() == 1);
    ASSERT_TRUE(l_m.at("DE::Line:71301::").name_ == 2);
    ASSERT_TRUE(std::string_view(l_m.at("DE::Line:71301::").short_name_) ==
                name);
    ASSERT_TRUE(std::string_view(l_m.at("DE::Line:71301::").transport_mode_) ==
                mode);
  } catch (std::exception const& e) {
    std::cout << e.what();
  }
}

TEST(service_frame_parse, direction) {
  try {
    const char* file =
        "base/loader/test_resources/netex_schedules/"
        "NX-PI-01_DE_NAP_LINE_126-HEAGTRAM-2_20220104.xml";
    xml::xml_document d;
    auto r = d.load_file(file);
    auto l_m = std::map<std::string, line>{};
    auto s_m = std::map<std::string, scheduled_points>{};
    auto s_p_m = std::map<std::string, stop_point>{};
    auto d_m = std::map<std::string, direction>{};
    auto p_m = std::map<std::string, passenger_assignments>{};
    parse_frame(d, l_m, s_m, s_p_m, d_m, p_m);
    std::string_view name = "1";
    std::string_view short_name = "1";
    std::string_view name2 = "2";
    std::string_view short_name2 = "2";
    ASSERT_TRUE(d_m.size() == 2);
    ASSERT_TRUE(std::string_view(d_m.at("DE::Direction:1::").name_) == name);
    ASSERT_TRUE(std::string_view(d_m.at("DE::Direction:1::").short_name_) ==
                short_name);
    ASSERT_TRUE(std::string_view(d_m.at("DE::Direction:2::").name_) == name2);
    ASSERT_TRUE(std::string_view(d_m.at("DE::Direction:2::").short_name_) ==
                short_name2);
  } catch (std::exception const& e) {
    std::cout << e.what();
  }
}

TEST(service_frame_parse, scheduled_points) {
  try {
    const char* file =
        "base/loader/test_resources/netex_schedules/"
        "NX-PI-01_DE_NAP_LINE_126-HEAGTRAM-2_20220104.xml";
    xml::xml_document d;
    auto r = d.load_file(file);
    auto l_m = std::map<std::string, line>{};
    auto s_m = std::map<std::string, scheduled_points>{};
    auto s_p_m = std::map<std::string, stop_point>{};
    auto d_m = std::map<std::string, direction>{};
    auto p_m = std::map<std::string, passenger_assignments>{};
    parse_frame(d, l_m, s_m, s_p_m, d_m, p_m);
    std::string_view short_name =
        "Darmstadt Merck-Stadion | Haltestelle 0 | VBöllenfalltorStra";
    std::string_view public_code = "";
    std::string_view stop_type = "tramStation";
    ASSERT_TRUE(s_m.size() == 24);
    ASSERT_TRUE(
        std::string_view(
            s_m.at("DE::ScheduledStopPoint:12466101_126_::").short_name_) ==
        short_name);
    ASSERT_TRUE(
        std::string_view(
            s_m.at("DE::ScheduledStopPoint:12466101_126_::").stop_type_) ==
        stop_type);
    ASSERT_TRUE(
        std::string_view(
            s_m.at("DE::ScheduledStopPoint:12466101_126_::").public_code_) ==
        public_code);
  } catch (std::exception const& e) {
    std::cout << e.what();
  }
}
TEST(service_frame_parse, stop_points) {
  try {
    const char* file =
        "base/loader/test_resources/netex_schedules/"
        "NX-PI-01_DE_NAP_LINE_126-HEAGTRAM-2_20220104.xml";
    xml::xml_document d;
    auto r = d.load_file(file);
    auto l_m = std::map<std::string, line>{};
    auto s_m = std::map<std::string, scheduled_points>{};
    auto s_p_m = std::map<std::string, stop_point>{};
    auto d_m = std::map<std::string, direction>{};
    auto p_m = std::map<std::string, passenger_assignments>{};
    parse_frame(d, l_m, s_m, s_p_m, d_m, p_m);
    std::string_view key = "GlobalID";
    std::string_view value = "de:06411:4734";
    std::string_view name = "Darmstadt Hauptbahnhof";
    ASSERT_TRUE(s_p_m.size() == 11);
    ASSERT_TRUE(std::string_view(
                    s_p_m.at("DE::StopPlace:300004734_1000::").key_) == key);
    ASSERT_TRUE(
        std::string_view(s_p_m.at("DE::StopPlace:300004734_1000::").value_) ==
        value);
    ASSERT_TRUE(std::string_view(
                    s_p_m.at("DE::StopPlace:300004734_1000::").name_) == name);
  } catch (std::exception const& e) {
    std::cout << e.what();
  }
}
TEST(service_frame_parse, passenger_assignments) {
  try {
    const char* file =
        "base/loader/test_resources/netex_schedules/"
        "NX-PI-01_DE_NAP_LINE_126-HEAGTRAM-2_20220104.xml";
    xml::xml_document d;
    auto r = d.load_file(file);
    auto l_m = std::map<std::string, line>{};
    auto s_m = std::map<std::string, scheduled_points>{};
    auto s_p_m = std::map<std::string, stop_point>{};
    auto d_m = std::map<std::string, direction>{};
    auto p_m = std::map<std::string, passenger_assignments>{};
    parse_frame(d, l_m, s_m, s_p_m, d_m, p_m);
    std::string_view scheduled_stop = "DE::ScheduledStopPoint:12466101_126_::";
    std::string_view quay_id = "";
    std::string_view stop_point = "DE::StopPlace:300024661_1000::";
    ASSERT_TRUE(p_m.size() == 24);
    ASSERT_TRUE(std::string_view(p_m.at("DE::PassengerStopAssignment:965366::")
                                     .stop_point_id_) == stop_point);
    ASSERT_TRUE(std::string_view(
                    p_m.at("DE::PassengerStopAssignment:965366::").quay_id_) ==
                quay_id);
    ASSERT_TRUE(std::string_view(p_m.at("DE::PassengerStopAssignment:965366::")
                                     .scheduled_place_id_) == scheduled_stop);
  } catch (std::exception const& e) {
    std::cout << e.what();
  }
}
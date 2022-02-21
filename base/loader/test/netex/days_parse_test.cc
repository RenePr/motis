#include "gtest/gtest.h"

#include <functional>
#include <iostream>

#include "utl/verify.h"

#include "motis/loader/netex/days_parse.h"

TEST(days_parse, test1) {

  try {
    /// Users/reneprinz/Documents/motis
    const char* file =
        "base/loader/test_resources/netex_schedules/"
        "NX-PI-01_DE_NAP_LINE_123-ERLBUS-371_20211029_daytpye_ref.xml";
    xml::xml_document d;
    auto r = d.load_file(file);
    utl::verify(r, "netex parser: invalid xml in {}", file);
    auto const& days = motis::loader::netex::combine_daytyps_uic_opertions(d);
    std::string_view valid_bits{
        "11111001111100111110011111001111100111110011111"};
    std::string_view from_date{"2021-10-25T00:00:00"};
    std::string_view to_date{"2021-12-10T00:00:00"};
    std::string key = days.at("DE::DayType:128678::").uic_id_;

    std::string_view valid_bits2{
        "100000010000001000000100000010000001000000100000010000001"};
    std::string_view from_date2{"2021-10-15T00:00:00"};
    std::string_view to_date2{"2021-12-10T00:00:00"};
    std::string key2 = days.at("DE::DayType:128682::").uic_id_;

    ASSERT_TRUE(days.at("DE::DayType:128678::").uic_.at(key).valid_day_bits_ ==
                valid_bits);
    ASSERT_TRUE(days.at("DE::DayType:128678::").uic_.at(key).from_date_ ==
                from_date);
    ASSERT_TRUE(days.at("DE::DayType:128678::").uic_.at(key).to_date_ ==
                to_date);

    ASSERT_FALSE(days.at("DE::DayType:128678::").uic_.at(key).to_date_ ==
                 from_date);
    ASSERT_FALSE(days.at("DE::DayType:128678::").uic_.at(key).to_date_ ==
                 valid_bits);

    ASSERT_TRUE(days.at("DE::DayType:128682::").uic_.at(key2).valid_day_bits_ ==
                valid_bits2);
    ASSERT_TRUE(days.at("DE::DayType:128682::").uic_.at(key2).from_date_ ==
                from_date2);
    ASSERT_TRUE(days.at("DE::DayType:128682::").uic_.at(key2).to_date_ ==
                to_date2);

    ASSERT_FALSE(days.at("DE::DayType:128682::").uic_.at(key2).to_date_ ==
                 from_date);
    ASSERT_FALSE(days.at("DE::DayType:128682::").uic_.at(key2).to_date_ ==
                 valid_bits);

    ASSERT_TRUE(days.at("DE::DayType:128682::").uic_.size() ==
                days.at("DE::DayType:128682::").day_.size());

    auto const seasons_m = get_season_times(days);
    ASSERT_TRUE(11 == seasons_m.size());
    ASSERT_TRUE(seasons_m.at("DE::DayType:128682::").day_idx_first_day_ == 0);
    ASSERT_TRUE(seasons_m.at("DE::DayType:128682::").day_idx_last_day_ == 56);
    ASSERT_TRUE(
        seasons_m.at("DE::DayType:128682::").minutes_after_midnight_last_day_ ==
        0);
    ASSERT_TRUE(seasons_m.at("DE::DayType:128682::")
                    .minutes_after_midnight_first_day_ == 0);
  } catch (std::exception const& e) {
  }
}
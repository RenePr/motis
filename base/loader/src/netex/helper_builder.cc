#include "motis/loader/netex/builder/helper_builder.h"

#include <chrono>
#include <iostream>
#include <map>
#include <sstream>
#include "date/date.h"
#include "date/tz.h"
#include "time.h"

#include "motis/schedule-format/Schedule_generated.h"

#include "motis/loader/netex/days.h"
#include "motis/loader/util.h"

namespace fbs64 = flatbuffers64;

namespace motis::loader::netex {
int time_realtive_to_0(std::string const& time, std::string const& start) {
  auto tm_start = std::tm{};
  auto ti_start = std::istringstream(std::string(start));
  ti_start >> std::get_time(&tm_start, "%h%m%s");
  auto tm_0 = std::tm{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  auto tm_t = std::tm{};
  auto ti = std::istringstream(std::string(time));
  ti >> std::get_time(&tm_t, "%h%m%s");
  auto const seconds = std::difftime(mktime(&tm_t), mktime(&tm_0));
  return seconds;
}
int time_realtive_to_0_season(std::string const& time,
                              std::string const& start) {
  auto tm_start = std::tm{};
  auto ti_start = std::istringstream(std::string(start));
  ti_start >> std::get_time(&tm_start, "%Y-%m-%d%Th%m%s");
  auto tm_0 = std::tm{0,
                      0,
                      0,
                      tm_start.tm_mday,
                      tm_start.tm_mon,
                      tm_start.tm_year,
                      tm_start.tm_wday,
                      tm_start.tm_yday,
                      0,
                      0,
                      0};
  auto tm_t = std::tm{};
  auto ti = std::istringstream(std::string(time));
  ti >> std::get_time(&tm_t, "%Y-%m-%d%Th%m%s");
  auto const seconds = std::difftime(mktime(&tm_t), mktime(&tm_0));
  return seconds;
}
std::pair<std::string, std::string> get_valid_day_bits(
    std::map<std::string, ids> const& days_map,
    std::vector<std::string> const& keys) {
  auto pair = std::pair<std::string, std::string>{};
  for (auto const& dt : keys) {
    auto const it = days_map.lower_bound(dt);
    auto const key_uic = std::string(it->second.uic_id_);
    auto const valid_day_bits =
        std::string(it->second.uic_.at(key_uic).valid_day_bits_);
    auto const from_date = std::string(it->second.uic_.at(key_uic).from_date_);
    pair = std::make_pair(valid_day_bits, from_date);
  }
  return pair;
}

void get_provider_operator_fbs(std::vector<std::string> const& lines,
                               std::map<std::string, line> const& l_m,
                               fbs64::Offset<Category>& category,
                               fbs64::Offset<Provider>& provider, int& name,
                               fbs64::FlatBufferBuilder& fbb) {
  for (auto const& l_r : lines) {
    if (l_m.size() != 1) {
      std::cout << "Warning more than one line in one file" << std::endl;
    }
    auto const it = l_m.lower_bound(l_r);
    try {
      utl::verify(it != end(l_m), "missing line: {}", l_r);
      name = it->second.name_;
    } catch (std::runtime_error& e) {
      std::cout << e.what() << std::endl;
      continue;
    }
    auto const test = std::string("1");
    provider = CreateProvider(
        fbb, to_fbs_string(fbb, it->second.operator_.short_name_),
        to_fbs_string(fbb, it->second.operator_.name_),
        to_fbs_string(fbb, it->second.operator_.legal_name_),
        to_fbs_string(fbb, test));
    // TODO output_rule fehlt, linie oder zugnummer
    auto const s = std::string(it->second.transport_mode_);
    category = CreateCategory(fbb, to_fbs_string(fbb, s), 0);
  }
}

void get_attribute_fbs(std::vector<std::string> const& keys_day,
                       std::vector<notice_assignment> const& notice_assignments,
                       std::map<std::string, ids> const& days_map,
                       std::vector<fbs64::Offset<Attribute>>& attribute,
                       fbs64::FlatBufferBuilder& fbb) {
  auto notice_assignment_ve = std::vector<fbs64::Offset<AttributeInfo>>{};
  for (auto const& n_a : notice_assignments) {
    auto const attribute_info =
        CreateAttributeInfo(fbb, to_fbs_string(fbb, n_a.public_code_),
                            to_fbs_string(fbb, n_a.text_));
    notice_assignment_ve.push_back(attribute_info);
  }
  // for attribute
  auto const pair_day = get_valid_day_bits(days_map, keys_day);
  auto const valid_day_bits = pair_day.first;
  attribute = utl::to_vec(
      begin(notice_assignment_ve), end(notice_assignment_ve),
      [&](fbs64::Offset<AttributeInfo> const& ai) {
        return CreateAttribute(fbb, ai, to_fbs_string(fbb, valid_day_bits));
      });
}
void get_service_times(time_table_passing_time const& ttpt,
                       std::string const& start_time,
                       std::vector<int>& times_v) {
  // TODO in seconds?
  if (times_v.size() == 0 && std::string_view(ttpt.arr_time) == "") {
    // first time
    times_v.push_back(-1);
    auto const time = time_realtive_to_0(ttpt.dep_time, start_time);
    times_v.push_back(time);
  } else if (std::string_view(ttpt.dep_time) == "") {
    // berechne arr time to 0
    auto const time = time_realtive_to_0(ttpt.arr_time, start_time);
    times_v.push_back(time);
    times_v.push_back(-1);
  } else {
    auto const time = time_realtive_to_0(ttpt.arr_time, start_time);
    times_v.push_back(time);
    auto const time2 = time_realtive_to_0(ttpt.dep_time, start_time);
    times_v.push_back(time2);
  }
}
void get_section_fbs(build_sec const& sec, fbs64::Offset<Section>& section,
                     fbs64::FlatBufferBuilder& fbb) {
  section = CreateSection(
      fbb, sec.category_, sec.provider_, 0, to_fbs_string(fbb, sec.line_id),
      fbb.CreateVector(
          utl::to_vec(begin(sec.a_v_), end(sec.a_v_),
                      [&](fbs64::Offset<Attribute> const& a) { return a; })),
      sec.direction_);
}

void get_station_dir_fbs(stations_direction const& s_d,
                         fbs64::Offset<Station>& station,
                         fbs64::Offset<Direction>& direction,
                         fbs64::FlatBufferBuilder& fbb) {
  // external ids = leer
  // TODO interchange time
  auto test = std::vector<std::string>{};
  auto const test1 = std::string("1");
  test.push_back(test1);
  station = CreateStation(
      fbb, to_fbs_string(fbb, s_d.name_), to_fbs_string(fbb, s_d.name_),
      s_d.lat_, s_d.lng_, 0,
      fbb.CreateVector(utl::to_vec(
          begin(test), end(test),
          [&](std::string const& s) { return fbb.CreateString(s); })),
      s_d.timezone_, to_fbs_string(fbb, s_d.timezone_name_));
  auto const dir = std::string("1");
  direction = CreateDirection(fbb, station, to_fbs_string(fbb, dir));
  // TODO bitfield fehlt, gibt an welche gleißangebe bei mehreren gilt
}

}  // namespace motis::loader::netex
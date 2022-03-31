#include "motis/loader/netex/builder/helper_builder.h"

#include <iostream>
#include <map>
#include "time.h"

#include "boost/date_time/gregorian/gregorian_types.hpp"

#include "motis/schedule-format/Schedule_generated.h"

#include "motis/core/common/date_time_util.h"
#include "motis/core/common/logging.h"
#include "motis/loader/netex/days/days.h"
#include "motis/loader/util.h"

namespace fbs64 = flatbuffers64;
using namespace motis::logging;

namespace motis::loader::netex {

int time_realtive_to_0_season(std::string const& start) {
  if (std::string_view(start) == "") {
    return 0;
  } else {
    // TODO parse<int>?
    auto const year = std::stoi(start.substr(0, 4));
    auto const mon = std::stoi(start.substr(5, 2));
    auto const day = std::stoi(start.substr(8, 2));
    if (day == 0 || mon == 0 || year == 0) {
      LOG(info) << "Can't parse datum from Uic";
    }
    boost::gregorian::date sec(year, mon, day);
    // TODO Intervall noch manuell eintragen
    boost::gregorian::date invervall_s(2021, 1, 1);
    auto const end = (sec - invervall_s).days();
    return end;
  }
}
int time_realtive_to_0(std::string const& now_time, std::string const& start) {
  // TODO to parse_string
  if (std::string_view(now_time) == std::string_view(std::string("")) ||
      std::string_view(start) == std::string_view(std::string(""))) {
    return 0;
  }
  auto raw = time_t{};
  auto sec = time(&raw);
  auto timeinfo = localtime(&sec);
  auto tm_start = *timeinfo;
  auto ti_start = std::istringstream(std::string(start));
  ti_start >> std::get_time(&tm_start, "%H:%M:%S");
  auto tm_t = *timeinfo;
  auto ti = std::istringstream(std::string(now_time));
  ti >> std::get_time(&tm_t, "%H:%M:%S");
  return (std::difftime(mktime(&tm_t), mktime(&tm_start)) / 60);
}
std::pair<std::string, std::string> get_valid_day_bits(
    std::map<std::string, ids> const& days_map,
    std::vector<std::string> const& keys) {
  auto pair = std::pair<std::string, std::string>{};
  for (auto const& dt : keys) {
    auto const it = days_map.lower_bound(dt);
    utl::verify(it != end(days_map), "missing days_types: {}", dt);
    auto const key_uic = it->second.uic_id_;
    auto const valid_day_bits =
        std::string(it->second.uic_.at(key_uic).valid_day_bits_);
    auto const from_date = std::string(it->second.uic_.at(key_uic).from_date_);
    pair = std::make_pair(valid_day_bits, from_date);
  }
  return pair;
}

void get_ttpts(routes_data const& routes_d, std::vector<routes>& routes_v) {
  for (auto const& ttpt : routes_d.keys_ttpt_) {
    auto route = routes{};
    auto const it_sp =
        routes_d.stop_point_map_.lower_bound(ttpt.stop_point_ref_);
    utl::verify(it_sp != end(routes_d.stop_point_map_),
                "missing time_table_passing_time: {}", ttpt.stop_point_ref_);
    auto const key_sp = std::string(it_sp->second.id_);
    auto const it = routes_d.stations_map_.lower_bound(key_sp);
    utl::verify(it != end(routes_d.stations_map_),
                "missing scheduled_stop_point: {}", key_sp);
    route.schedulep_point_ref_ = it->first;
    auto st_dir = it->second;
    st_dir.timezone_ = routes_d.timezone_;
    st_dir.direction_ = routes_d.direction_;
    route.st_dir_ = st_dir;
    route.in_allowed_ = it_sp->second.in_allowed_ ? 1 : 0;
    route.out_allowed_ = it_sp->second.out_allowed_ ? 1 : 0;
    if (it->second.quays_.size() == 0) {
      route.quay_ = it->second.quays_.front();
    } else {
      // TODO für alles
      route.quay_ = it->second.quays_.front();
    }
    routes_v.push_back(route);
  }
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

void get_attribute_fbs(std::string const& traffic_days,
                       std::vector<notice_assignment> const& notice_assignments,
                       std::vector<fbs64::Offset<Attribute>>& attribute,
                       fbs64::FlatBufferBuilder& fbb) {
  auto notice_assignment_ve = std::vector<fbs64::Offset<AttributeInfo>>{};
  for (auto const& n_a : notice_assignments) {
    auto const attribute_info =
        CreateAttributeInfo(fbb, to_fbs_string(fbb, n_a.public_code_),
                            to_fbs_string(fbb, n_a.text_));
    notice_assignment_ve.push_back(attribute_info);
  }
  // TODO traffic days nicht gleich den aktuellen traffic days
  auto const st1 = std::string("0");
  attribute =
      utl::to_vec(begin(notice_assignment_ve), end(notice_assignment_ve),
                  [&](fbs64::Offset<AttributeInfo> const& ai) {
                    return CreateAttribute(fbb, ai, to_fbs_string(fbb, st1));
                  });
}
void get_service_times(time_table_passing_time const& ttpt,
                       std::string const& start_time,
                       std::vector<int>& times_v) {
  // TODO in seconds?
  if (times_v.size() == 0 && std::string_view(ttpt.arr_time_) == "") {
    times_v.push_back(-1);
    auto const time = time_realtive_to_0(ttpt.dep_time_, start_time);
    times_v.push_back(time);
  } else if (std::string_view(ttpt.dep_time_) == "") {
    auto const time = time_realtive_to_0(ttpt.arr_time_, start_time);
    times_v.push_back(time);
    times_v.push_back(-1);
  } else {
    auto const time = time_realtive_to_0(ttpt.arr_time_, start_time);
    times_v.push_back(time);
    auto const time2 = time_realtive_to_0(ttpt.dep_time_, start_time);
    times_v.push_back(time2);
  }
}
void get_section_fbs(section const& sec, fbs64::Offset<Section>& section,
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
  // TODO interchange time, external ids vorerst leer
  auto test = std::vector<std::string>{};
  auto const test1 = std::string("0");
  test.push_back(test1);
  station = CreateStation(
      fbb, to_fbs_string(fbb, s_d.stop_point_id_),
      to_fbs_string(fbb, s_d.name_), s_d.lat_, s_d.lng_, 0,
      fbb.CreateVector(utl::to_vec(
          begin(test), end(test),
          [&](std::string const& s) { return fbb.CreateString(s); })),
      s_d.timezone_, to_fbs_string(fbb, s_d.timezone_name_));
  direction = CreateDirection(fbb, station, to_fbs_string(fbb, s_d.direction_));
}
std::map<std::string, stations_direction> get_stations(
    std::map<std::string, scheduled_points> const& s_m,
    std::map<std::string, stop_point> const& s_p_m,
    std::map<std::string, passenger_assignments> const& p_m) {
  auto s_d_m = std::map<std::string, stations_direction>{};
  for (auto const& p_a : p_m) {
    auto const it_s = s_m.lower_bound(p_a.second.scheduled_place_id_);
    utl::verify(it_s != end(s_m), "missing scheduled_point: {}",
                p_a.second.scheduled_place_id_);
    auto const it_s_p = s_p_m.lower_bound(p_a.second.stop_point_id_);
    utl::verify(it_s_p != end(s_p_m), "missing stop_point: {}",
                p_a.second.stop_point_id_);
    auto st = stations_direction{};
    st.name_ = it_s_p->second.name_;
    st.lat_ = it_s_p->second.lat_;
    st.lng_ = it_s_p->second.lon_;
    st.timezone_name_ = it_s_p->second.timezone_;
    st.quays_ = it_s_p->second.quay_;
    st.stop_point_id_ = p_a.second.stop_point_id_;
    st.id_ = it_s_p->first;
    s_d_m.emplace(p_a.second.scheduled_place_id_, st);
  }
  return s_d_m;
}
}  // namespace motis::loader::netex
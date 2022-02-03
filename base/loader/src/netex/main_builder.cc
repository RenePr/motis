#include "motis/loader/netex/builder/main_builder.h"

#include <iostream>
#include <map>
#include <vector>

#include "motis/schedule-format/Schedule_generated.h"

#include "motis/loader/netex/builder/helper_builder.h"

namespace fbs64 = flatbuffers64;

namespace motis::loader::netex {

void build_fbs(build const& b, std::vector<fbs64::Offset<Route>>& routes,
               std::vector<fbs64::Offset<Service>>& services,
               std::map<std::string, fbs64::Offset<Station>>& stations,
               std::vector<fbs64::Offset<RuleService>>& rule_services,
               fbs64::FlatBufferBuilder& fbb) {
  // TODO dummyweise hier, das rest ermal geht
  auto const season = CreateSeason(fbb, 0, 0, 0, 0, 0);
  auto const timezone = CreateTimezone(fbb, 0, season);
  auto stations_sji_m = std::map<std::string, fbs64::Offset<Station>>{};
  auto service_sji_m = std::map<std::string, fbs64::Offset<Service>>{};
  for (auto const& sj : b.sj_m_) {
    std::cout << sj.second.key_sjp_ << sj.first << std::endl;
    auto const it_sjp = b.sjp_m_.lower_bound(sj.second.key_sjp_);
    auto a_v = std::vector<fbs64::Offset<Attribute>>{};
    get_attribute_fbs(sj.second.keys_day_, it_sjp->second.notice_assignments_,
                      b.days_m_, a_v, fbb);
    auto category = fbs64::Offset<Category>{};
    auto provider = fbs64::Offset<Provider>{};
    int name;
    auto const lines = it_sjp->second.lines_;
    get_provider_operator_fbs(lines, b.l_m_, category, provider, name, fbb);
    auto const traffic_days =
        get_valid_day_bits(b.days_m_, sj.second.keys_day_);
    auto const minutes_a_m_f_d =
        time_realtive_to_0_season(traffic_days.first, traffic_days.first);
    // TODO change auf last
    auto const minutes_a_m_l_d =
        time_realtive_to_0_season(traffic_days.first, traffic_days.first);
    auto stations_v = std::vector<fbs64::Offset<Station>>{};
    // TODO
    auto in_allowed_v = std::vector<uint8_t>{};
    auto out_allowed_v = std::vector<uint8_t>{};
    auto sections_v = std::vector<fbs64::Offset<Section>>{};
    auto track_v = std::vector<fbs64::Offset<Track>>{};
    auto times_v = std::vector<int>{};
    auto start_time = begin(sj.second.keys_ttpt_)->arr_time;
    for (auto const& ttpt : sj.second.keys_ttpt_) {
      auto s_d = station_dir{
          it_sjp->second.stop_point_map, b.s_m_,          b.l_m_,  traffic_days,
          ttpt.stop_point_ref,           std::string(""), timezone};
      auto station = fbs64::Offset<Station>{};
      auto direction = fbs64::Offset<Direction>{};
      auto track = fbs64::Offset<Track>{};
      // TODO is uint8_t richtig?
      auto in_allowed = uint8_t{};
      auto out_allowed = uint8_t{};
      get_service_times(ttpt, start_time, times_v);
      get_station_dir_fbs(s_d, in_allowed, out_allowed, station, direction,
                          track, fbb);
      stations_v.push_back(station);
      // TODO eventuell ändern
      stations.try_emplace(ttpt.stop_point_ref, station);
      auto sec = build_sec{category, provider, a_v,
                           std::string(begin(b.l_m_)->second.id_), direction};
      auto section = fbs64::Offset<Section>{};
      get_section_fbs(sec, section, fbb);
      sections_v.push_back(section);
      in_allowed_v.push_back(in_allowed);
      out_allowed_v.push_back(out_allowed);
      track_v.push_back(track);

      auto const key = std::string(
          it_sjp->second.stop_point_map.lower_bound(ttpt.stop_point_ref)
              ->second.id_);
      stations_sji_m.try_emplace(key, station);
    }
    auto const route = CreateRoute(
        fbb,
        fbb.CreateVector(
            utl::to_vec(begin(stations_v), end(stations_v),
                        [&](fbs64::Offset<Station> const& s) { return s; })),
        fbb.CreateVector(utl::to_vec(begin(in_allowed_v), end(in_allowed_v),
                                     [](uint8_t const& i) { return i; })),
        fbb.CreateVector(utl::to_vec(begin(out_allowed_v), end(out_allowed_v),
                                     [](uint8_t const& o) { return o; })));
    routes.push_back(route);
    // TODO raussuchen, kein int nur strings...
    auto const rule_service_debug = CreateServiceDebugInfo(
        fbb, to_fbs_string(fbb, std::string(b.file_)), 0, 0);
    // TODO  trackrules noch anpassen in arr und dep
    auto track_rules_v = std::vector<fbs64::Offset<TrackRules>>{};
    auto const track_rules = CreateTrackRules(
        fbb,
        fbb.CreateVector(
            utl::to_vec(begin(track_v), end(track_v),
                        [&](fbs64::Offset<Track> const& t) { return t; })),
        fbb.CreateVector(
            utl::to_vec(begin(track_v), end(track_v),
                        [&](fbs64::Offset<Track> const& t) { return t; })));
    track_rules_v.push_back(track_rules);
    // TODO route_key, inital train nummer, rule_participant
    auto const service = CreateService(
        fbb, route, to_fbs_string(fbb, traffic_days.first),
        fbb.CreateVector(
            utl::to_vec(begin(sections_v), end(sections_v),
                        [&](fbs64::Offset<Section> const& s) { return s; })),
        fbb.CreateVector(
            utl::to_vec(begin(track_rules_v), end(track_rules_v),
                        [&](fbs64::Offset<TrackRules> const& t) { return t; })),
        fbb.CreateVector(utl::to_vec(begin(times_v), end(times_v),
                                     [](int const& t) { return t; })),
        0, rule_service_debug, false, 0);
    services.push_back(service);
    service_sji_m.try_emplace(sj.first, service);
  }
  auto rule_service_v = std::vector<fbs64::Offset<Rule>>{};
  for (auto const& sji : b.sji_v_) {
    auto const from_service =
        service_sji_m.lower_bound(sji.from_journey_)->second;
    auto const to_service = service_sji_m.lower_bound(sji.to_journey_)->second;
    auto const from_stop =
        stations_sji_m.lower_bound(sji.from_station_)->second;
    auto const to_stop = stations_sji_m.lower_bound(sji.to_station_)->second;
    // TODO ruletype fehlt, dayoffset1,dayoffset2, day_switch
    auto const rule = CreateRule(fbb, static_cast<RuleType>(0.0), from_service,
                                 to_service, from_stop, to_stop, 0, 0, false);
    rule_service_v.push_back(rule);
  }
  if (!rule_service_v.empty()) {
  }
  auto const rule_service = CreateRuleService(
      fbb, fbb.CreateVector(
               utl::to_vec(begin(rule_service_v), end(rule_service_v),
                           [&](fbs64::Offset<Rule> const& r) { return r; })));
  rule_services.push_back(rule_service);
}
}  // namespace motis::loader::netex
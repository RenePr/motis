#include "motis/loader/netex/builder/main_builder.h"

#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include "motis/schedule-format/Schedule_generated.h"

#include "motis/loader/netex/builder/helper_builder.h"

namespace fbs64 = flatbuffers64;

namespace motis::loader::netex {

void build_fbs(build const& b, std::vector<section_route>& sjp_m,
               fbs64::FlatBufferBuilder& fbb) {
  // TODO wichtig
  for (auto const& sj : b.sj_m_) {
    auto sjp = section_route{};
    sjp.key_sj_ = sj.second.key_sj_;
    auto const it_sjp = b.sjp_m_.lower_bound(sj.second.key_sjp_);
    utl::verify(it_sjp != end(b.sjp_m_), "missing service_journey_pattern: {}",
                sj.second.key_sjp_);
    auto a_v = std::vector<fbs64::Offset<Attribute>>{};
    get_attribute_fbs(sj.second.keys_day_, it_sjp->second.notice_assignments_,
                      b.days_m_, a_v, fbb);
    sjp.a_v_ = a_v;
    auto category = fbs64::Offset<Category>{};
    auto provider = fbs64::Offset<Provider>{};
    int name;
    auto const lines = it_sjp->second.lines_;
    get_provider_operator_fbs(lines, b.l_m_, category, provider, name, fbb);
    sjp.category_ = category;
    sjp.provider_ = provider;
    auto const traffic_days =
        get_valid_day_bits(b.days_m_, sj.second.keys_day_);
    // TODO mehr als ein key möglich
    auto const day_type = sj.second.keys_day_.front();
    auto const it_sea = b.seasons_m_.lower_bound(day_type);
    utl::verify(it_sea != end(b.seasons_m_), "missing seasons: {}", day_type);
    auto const it_days = b.days_m_.lower_bound(day_type);
    utl::verify(it_days != end(b.days_m_), "missing daytypes: {}", day_type);
    auto const uic_id = it_days->second.uic_id_;
    auto const it_uic = b.days_m_.at(it_days->first).uic_.lower_bound(uic_id);
    utl::verify(it_uic != end(b.days_m_.at(it_days->first).uic_), "missing uic: {}", uic_id);
    auto from_time = std::string(it_uic->second.from_date_);
    auto const day_i_f = time_realtive_to_0_season(from_time, b.intervall_start_);
    auto to_time = std::string(it_uic->second.to_date_);
    auto const day_i_l = time_realtive_to_0_season(to_time, b.intervall_start_);
    std::cout << day_i_l - day_i_f  << std::endl;
    auto s_time = std::string("");
    if(!begin(sj.second.keys_ttpt_)->dep_time_.empty()) {
      s_time =  begin(sj.second.keys_ttpt_)->dep_time_;
    }
    auto const minutes_after_midnight_first_day = time_realtive_to_0(s_time, std::string("00:00:00"));
    auto e_time = std::string("");
    if(!end(sj.second.keys_ttpt_)->arr_time_.empty()){
      e_time = end(sj.second.keys_ttpt_)->arr_time_;
    }
    auto const minutes_after_midnight_last_day = time_realtive_to_0(e_time, std::string("00:00:00"));
    auto const season =
        CreateSeason(fbb, 120, day_i_f, day_i_l,
                     minutes_after_midnight_first_day,
                     minutes_after_midnight_last_day);
    //  TODO generell offset = winterzeit unterschied zu gmt so korrekt
    auto const timezone = CreateTimezone(fbb, 60, season);
    //  TODO times_v und ttpt_v zusammen oder getrennt?
    auto times_v = std::vector<int>{};
    auto start_time = begin(sj.second.keys_ttpt_)->dep_time_;
    for (auto const& ttpt : sj.second.keys_ttpt_) {
      get_service_times(ttpt, start_time, times_v);
    }
    auto routes_v = std::vector<routes>{};
    auto const routes_d = routes_data{sj.second.keys_ttpt_,
                                   it_sjp->second.direction_,
                                   traffic_days.first,
                                   it_sjp->second.stop_point_map_,
                                   timezone,
                                   b.stations_map_};
    get_ttpts(routes_d, routes_v);
    sjp.traffic_days_ = traffic_days.first;
    sjp.times_v_ = times_v;
    sjp.routes_ = routes_v;
    sjp_m.push_back(sjp);
  }
}
void create_stations_routes_services_fbs(
    std::vector<section_route> const& sjpp,
    std::string const& file_name,
    std::map<std::string, fbs64::Offset<Station>>& fbs_stations,
    std::vector<fbs64::Offset<Route>>& fbs_routes,
    std::map<std::string, fbs64::Offset<Service>>& services,
    fbs64::FlatBufferBuilder& fbb) {
  auto counter = 0;
  for (auto const& ele : sjpp) {
    auto in_allowed_v = std::vector<uint8_t>{};
    auto out_allowed_v = std::vector<uint8_t>{};
    auto stations_v = std::vector<fbs64::Offset<Station>>{};
    auto sections_v = std::vector<fbs64::Offset<Section>>{};
    auto tracks_v = std::vector<fbs64::Offset<Track>>{};
    for (auto const& sta : ele.routes_) {
      auto station = fbs64::Offset<Station>{};
      auto direction = fbs64::Offset<Direction>{};
      /*if(fbs_stations.lower_bound(sta.st_dir_.stop_point_id_) != end(fbs_stations)) {
        std::cout << "Here?";
        continue;
      }*/
      get_station_dir_fbs(sta.st_dir_, station, direction, fbb);
      stations_v.push_back(station);
      in_allowed_v.push_back(static_cast<uint8_t>(sta.in_allowed_));
      out_allowed_v.push_back(static_cast<uint8_t>(sta.out_allowed_));
      fbs_stations.emplace(sta.st_dir_.stop_point_id_, station);
      // TODO Line_id
      auto const sec = section{ele.category_, ele.provider_, ele.a_v_,
                                 std::string(""), direction};
      auto section = fbs64::Offset<Section>{};
      get_section_fbs(sec, section, fbb);
      sections_v.push_back(section);
      auto const test = std::string("");
      auto const track = CreateTrack(fbb, to_fbs_string(fbb, test),
                                     to_fbs_string(fbb, test));
      tracks_v.push_back(track);

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
    fbs_routes.push_back(route);
    // TODO line from to
    auto counter2 = counter+1;
    auto const service_debug_info =
        CreateServiceDebugInfo(fbb, to_fbs_string(fbb, file_name), 0, 0);
    counter++;
    // TODO
    auto tracks_rules_v = std::vector<fbs64::Offset<TrackRules>>{};
    auto const tracks = CreateTrackRules(
        fbb,
        fbb.CreateVector(
            utl::to_vec(begin(tracks_v), end(tracks_v),
                        [&](fbs64::Offset<Track> const& t) { return t; })),
        fbb.CreateVector(
            utl::to_vec(begin(tracks_v), end(tracks_v),
                        [&](fbs64::Offset<Track> const& t) { return t; })));
    tracks_rules_v.push_back(tracks);
    //TODO entfehrnen
    tracks_rules_v.push_back(tracks);
    auto const st2 = std::string(ele.traffic_days_);
    auto const st1 = std::string("0");
    // TODO wenn ich das einkommentiere bekomme ich bei schedule: ERROR: bitset
    std::cout << "Länge: " << ele.traffic_days_.length() << std::endl;
    auto const service = CreateService(
        fbb, route, fbb.CreateString(st2),
        fbb.CreateVector(
            utl::to_vec(begin(sections_v), end(sections_v),
                        [&](fbs64::Offset<Section> const& s) { return s; })),
        fbb.CreateVector(
            utl::to_vec(begin(tracks_rules_v), end(tracks_rules_v),
                        [&](fbs64::Offset<TrackRules> const& t) { return t; })),
        fbb.CreateVector(utl::to_vec(begin(ele.times_v_), end(ele.times_v_),
                                     [](int const& t) { return t; })),
        0, service_debug_info, false, 0, fbb.CreateString(st1));
    services.emplace(ele.key_sj_, service);
  }
}
void create_rule_service(
    std::vector<service_journey_interchange> const& sji_v,
    std::map<std::string, fbs64::Offset<Service>> const& services,
    std::map<std::string, fbs64::Offset<Station>> const& fbs_stations,
    std::vector<fbs64::Offset<RuleService>>& rule_services,
    fbs64::FlatBufferBuilder& fbb) {
  auto rule_service_v = std::vector<fbs64::Offset<Rule>>{};
  for (auto const& sji : sji_v) {
    auto const it_from_service = services.lower_bound(sji.from_journey_);
    utl::verify(it_from_service != end(services), "missing from services: {}", services.lower_bound(sji.from_journey_)->second);
    auto const it_to_service = services.lower_bound(sji.to_journey_);
    utl::verify(it_to_service != end(services), "missing to services: {}", services.lower_bound(sji.to_journey_)->second);
    // TODO check if scheduledpointref in sji
    auto const it_from_stop = fbs_stations.lower_bound(sji.from_station_);
    utl::verify(it_from_stop != end(fbs_stations), "missing from stations: {}", services.lower_bound(sji.from_station_)->second);
    auto const it_to_stop = fbs_stations.lower_bound(sji.to_station_);
    utl::verify(it_to_stop != end(fbs_stations), "missing to stations: {}", services.lower_bound(sji.to_station_)->second);
    // TODO dayoffset1,dayoffset2, day_switch
    auto rule = fbs64::Offset<Rule>{};
    if (sji.stay_seated_) {
      rule = CreateRule(fbb, static_cast<RuleType>(0.0), it_from_service->second,
                        it_to_service->second, it_from_stop->second, it_to_stop->second, 0, 0, false);
    } else {
      rule = CreateRule(fbb, static_cast<RuleType>(1.0), it_from_service->second,
                        it_to_service->second, it_from_stop->second, it_to_stop->second, 0, 0, false);
    }
    rule_service_v.push_back(rule);
  }
  if (!rule_service_v.empty()) {
    auto const rule_service = CreateRuleService(
        fbb, fbb.CreateVector(
                 utl::to_vec(begin(rule_service_v), end(rule_service_v),
                             [&](fbs64::Offset<Rule> const& r) { return r; })));
    rule_services.push_back(rule_service);
  }
}
}  // namespace motis::loader::netex
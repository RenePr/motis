#include "motis/loader/netex/builder/main_builder.h"

#include <iostream>
#include <map>
#include <vector>

#include "motis/schedule-format/Schedule_generated.h"

#include "motis/loader/netex/builder/helper_builder.h"

namespace fbs64 = flatbuffers64;

namespace motis::loader::netex {

void build_fbs(build const& b, std::vector<service_journey_parse>& sjp_m,
               fbs64::FlatBufferBuilder& fbb) {
  // search and build .fbs files
  // TODO dummyweise hier, das rest ermal geht
  // day_idx_first_day und last_day = bitfield index?
  // bitfield index =
  // dataObjects/CompositeFrame/frames/ServiceCalendarFrame/ServiceCalendar/operatingPeriods/UicOperatingPeriod/FromDate
  // - dataObjects/CompositeFrame/ValidBetween/FromDate in days
  // gmt_offset winterzeit, offset sommerzeit
  // minutes_after_midnight von local timezone?
  // auto const minutes_a_m_f_d = time_realtive_to_0();
  auto const season = CreateSeason(fbb, 0, 0, 0, 0, 0);
  // general_offset vs offset? Unterschied?
  auto const timezone = CreateTimezone(fbb, 0, season);
  for (auto const& sj : b.sj_m_) {
    std::cout << sj.second.key_sjp_ << sj.first << std::endl;
    auto sjp = service_journey_parse{};
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
    auto const minutes_a_m_f_d =
        time_realtive_to_0_season(traffic_days.first, traffic_days.first);
    // TODO change auf last
    auto const minutes_a_m_l_d =
        time_realtive_to_0_season(traffic_days.first, traffic_days.first);
    // TODO times_v und ttpt_v zusammen oder getrennt?
    auto times_v = std::vector<int>{};
    auto start_time = begin(sj.second.keys_ttpt_)->arr_time;
    for (auto const& ttpt : sj.second.keys_ttpt_) {
      get_service_times(ttpt, start_time, times_v);
    }
    auto ttpt_v = std::vector<ttpt_index>{};
    for (auto const& ttpt : sj.second.keys_ttpt_) {
      auto ttpt_i = ttpt_index{};
      ttpt_i.stop_point_ref_ = ttpt.stop_point_ref;
      auto const it_sp =
          it_sjp->second.stop_point_map.lower_bound(ttpt.stop_point_ref);
      utl::verify(it_sp != end(it_sjp->second.stop_point_map),
                  "missing time_table_passing_time: {}", ttpt.stop_point_ref);
      auto const key_sp = std::string(it_sp->second.id_);
      auto const it = b.s_m_.lower_bound(key_sp);
      // TODO direction
      ttpt_i.st_dir_ =
          stations_direction{std::string(it->second.short_name_),
                             std::string(it->second.short_name_),
                             it->second.stop_point_.lat_,
                             it->second.stop_point_.lon_,
                             timezone,
                             std::string(it->second.stop_point_.timezone_),
                             std::string("")};
      ttpt_i.in_allowed_ = it_sp->second.in_allowed_;
      ttpt_i.out_allowed_ = it_sp->second.out_allowed_;
      //  TODO is uint8_t richtig?
      if (it->second.stop_point_.quay_.size() == 0) {
        ttpt_i.quay_ = traffic_days.first;
      } else {
        ttpt_i.quay_ = begin(it->second.stop_point_.quay_)->data();
      }
      ttpt_v.push_back(ttpt_i);
      // TODO eventuell ändern
    }
    sjp.times_v_ = times_v;
    sjp.ttpt_index_ = ttpt_v;
    sjp_m.push_back(sjp);
  }
}
void create_fbs(std::vector<service_journey_parse> const& sjpp,
                std::string const& file_name,
                std::map<std::string, fbs64::Offset<Station>>& fbs_stations,
                std::vector<fbs64::Offset<Route>>& fbs_routes,
                std::vector<fbs64::Offset<Service>>& output_services,
                fbs64::FlatBufferBuilder& fbb) {
  for (auto const& ele : sjpp) {
    auto in_allowed_v = std::vector<bool>{};
    auto out_allowed_v = std::vector<bool>{};
    auto stations_v = std::vector<fbs64::Offset<Station>>{};
    auto sections_v = std::vector<fbs64::Offset<Section>>{};
    auto tracks_v = std::vector<fbs64::Offset<Track>>{};
    for (auto const& sta : ele.ttpt_index_) {
      auto station = fbs64::Offset<Station>{};
      auto direction = fbs64::Offset<Direction>{};
      get_station_dir_fbs(sta.st_dir_, station, direction, fbb);
      stations_v.push_back(station);
      in_allowed_v.push_back(sta.in_allowed_);
      out_allowed_v.push_back(sta.out_allowed_);
      fbs_stations.try_emplace(sta.stop_point_ref_, station);
      // TODO Line_id
      auto const sec = build_sec{ele.category_, ele.provider_, ele.a_v_,
                                 std::string(""), direction};
      auto section = fbs64::Offset<Section>{};
      get_section_fbs(sec, section, fbb);
      sections_v.push_back(section);
      auto const track = CreateTrack(fbb, to_fbs_string(fbb, std::string("")),
                                     to_fbs_string(fbb, sta.quay_));
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
    auto const service_debug_info =
        CreateServiceDebugInfo(fbb, to_fbs_string(fbb, file_name), 0, 0);
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
    auto const service = CreateService(
        fbb, route, to_fbs_string(fbb, std::string("123")),
        fbb.CreateVector(
            utl::to_vec(begin(sections_v), end(sections_v),
                        [&](fbs64::Offset<Section> const& s) { return s; })),
        fbb.CreateVector(
            utl::to_vec(begin(tracks_rules_v), end(tracks_rules_v),
                        [&](fbs64::Offset<TrackRules> const& t) { return t; })),
        fbb.CreateVector(utl::to_vec(begin(ele.times_v_), end(ele.times_v_),
                                     [](int const& t) { return t; })),
        0, service_debug_info, false, 0);
    output_services.push_back(service);
  }
}
}  // namespace motis::loader::netex
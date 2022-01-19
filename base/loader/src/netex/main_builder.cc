#include "motis/loader/netex/builder/main_builder.h"

#include <iostream>
#include <map>

#include "motis/schedule-format/Schedule_generated.h"

#include "motis/loader/netex/builder/helper_builder.h"

namespace fbs64 = flatbuffers64;

namespace motis::loader::netex {

void build_fbs(build const& b, fbs64::FlatBufferBuilder& fbb) {
  // search and build .fbs files
  // TODO dummyweise hier, das rest ermal geht
  // day_idx_first_day und last_day = bitfield index?
  // bitfield index =
  // dataObjects/CompositeFrame/frames/ServiceCalendarFrame/ServiceCalendar/operatingPeriods/UicOperatingPeriod/FromDate
  // - dataObjects/CompositeFrame/ValidBetween/FromDate in days
  // gmt_offset = offset?
  // minutes_after_midnight von local timezone?
  auto const season = CreateSeason(fbb, 0, 0, 0, 0, 0);
  // general_offset vs offset? Unterschied?
  auto const timezone = CreateTimezone(fbb, 0, season);
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
    auto stations_v = std::vector<fbs64::Offset<Station>>{};
    auto sections_v = std::vector<fbs64::Offset<Section>>{};
    // TODO
    auto in_allowed_v = std::vector<uint8_t>{};
    auto out_allowed_v = std::vector<uint8_t>{};
    auto track_v = std::vector<fbs64::Offset<Track>>{};
    for (auto const& ttpt : sj.second.keys_ttpt_) {
      auto s_d_s = station_dir_section{};
      s_d_s.l_m_ = b.l_m_;
      s_d_s.s_m_ = b.s_m_;
      s_d_s.traffic_days = traffic_days;
      s_d_s.a_v_ = a_v;
      s_d_s.provider_ = provider;
      s_d_s.category_ = category;
      s_d_s.timezone_ = timezone;
      s_d_s.key_ = std::string(ttpt);
      s_d_s.direction_ = it_sjp->second.direction_;
      s_d_s.s_p_m_ = it_sjp->second.stop_point_map;
      auto station = fbs64::Offset<Station>{};
      auto direction = fbs64::Offset<Direction>{};
      auto section = fbs64::Offset<Section>{};
      auto track = fbs64::Offset<Track>{};
      // TODO is uint8_t richtig?
      auto in_allowed = uint8_t{};
      auto out_allowed = uint8_t{};
      get_station_dir_section(s_d_s, in_allowed, out_allowed, station,
                              direction, section, track, fbb);
      stations_v.push_back(station);
      sections_v.push_back(section);
      in_allowed_v.push_back(in_allowed);
      out_allowed_v.push_back(out_allowed);
      track_v.push_back(track);
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
    // TODO raussuchen
    auto start_point = 1;
    auto stop_point = 1;
    auto const rule_service_debug = CreateServiceDebugInfo(
        fbb, to_fbs_string(fbb, std::string(b.file_)), start_point, stop_point);
    // TODO  trackrules noch anpassen in arr und dep
    auto track_rules_v = std::vector<fbs64::Offset<TrackRules>>{};
    auto const track_rules =
        CreateTrackRules(fbb,
                         fbb.CreateVector(utl::to_vec(
                             begin(track_v), end(track_v),
                             [](fbs64::Offset<Track> const& t) { return t; })),
                         fbb.CreateVector(utl::to_vec(
                             begin(track_v), end(track_v),
                             [](fbs64::Offset<Track> const& t) { return t; })));
    track_rules_v.push_back(track_rules);
    // TODO times
    auto const service = CreateService(
        fbb, route, to_fbs_string(fbb, traffic_days),
        fbb.CreateVector(
            utl::to_vec(begin(sections_v), end(sections_v),
                        [&](fbs64::Offset<Section> const& s) { return s; })),
        fbb.CreateVector(
            utl::to_vec(begin(track_rules_v), end(track_rules_v),
                        [&](fbs64::Offset<TrackRules> const& t) { return t; })),
        NULL, 0, rule_service_debug, false, 0);
    // wzl-BUS-1 über passengerassignment,
    // 209-wefra über schedulestoppoint name -> quay
    // gar nicht
    // TODO trackroules= new over passengerassignment -> quay, times in
    // siteconnection WalkTransferDuration?, routekey uint?,
    // rule_participant?, initial_train_nr?, trip id optional ? auto const
    // service = CreateService(fbb, route, to_fbs_string(fbb,
    // valid_day_bits), ,);
  }
}
}  // namespace motis::loader::netex
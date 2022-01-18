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
  auto const season = CreateSeason(fbb, 0, 0, 0, 0);
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
    auto stations_v = std::vector<fbs64::Offset<Station>>{};
    // TODO
    // auto in_allowed_v = std::vector<uint8_t>{};
    // auto out_allowed_v = std::vector<uint8_t>{};
    for (auto const& ttpt : sj.second.keys_ttpt_) {
      auto s_d_s = station_dir_section{};
      s_d_s.l_m_ = b.l_m_;
      s_d_s.s_m_ = b.s_m_;
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
      get_station_dir_section(s_d_s, station, direction, section, fbb);
      stations_v.push_back(station);
      // in_allowed_v.push_back(0u);
      // out_allowed_v.push_back(0u);
    }
    /*auto const route = CreateRoute(
        fbb,
        fbb.CreateVector(utl::to_vec(
            begin(stations_v), end(stations_v),
            [&](fbs64::Offset<Station> const& s) { return s; })),
        fbb.CreateVector(utl::to_vec(begin(in_allowed_v), end(in_allowed_v),
                                     [](uint8_t const& i) { return i; })),
        fbb.CreateVector(utl::to_vec(begin(out_allowed_v),
                                     end(out_allowed_v),
                                     [](uint8_t const& o) { return o;
       })));*/
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
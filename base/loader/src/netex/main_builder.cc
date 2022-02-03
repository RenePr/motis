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
    // TODO
    auto times_v = std::vector<int>{};
    auto ttpt_v = std::vector<ttpt_index>{};
    auto start_time = begin(sj.second.keys_ttpt_)->arr_time;

    // New
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
      auto out_allowed = it_sp->second.out_allowed_;
      ttpt_i.in_allowed_ = it_sp->second.in_allowed_;
      ttpt_i.out_allowed_ = it_sp->second.out_allowed_;
      //  TODO is uint8_t richtig?
      get_service_times(ttpt, start_time, times_v);
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
}  // namespace motis::loader::netex
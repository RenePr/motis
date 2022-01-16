#include "motis/loader/netex/netex_parser.h"

#include <cstring>
#include <iostream>
#include <vector>

#include "boost/filesystem.hpp"

#include "pugixml.hpp"

#include "utl/get_or_create.h"
#include "utl/verify.h"

#include "motis/core/common/logging.h"
#include "motis/core/common/zip_reader.h"
#include "motis/loader/netex/builder/helper_builder.h"
#include "motis/loader/netex/days_parse.h"
#include "motis/loader/netex/get_valid_day_bits_transform.h"
#include "motis/loader/netex/service_frame/service_frame.h"
#include "motis/loader/netex/service_frame/service_frame_parse.h"
#include "motis/loader/netex/service_journey.h"
#include "motis/loader/netex/service_journey/service_journey.h"
#include "motis/loader/netex/service_journey/service_journey_parse.h"
#include "motis/loader/netex/service_journey_pattern/service_journey_pattern.h"
#include "motis/loader/netex/service_journey_pattern/service_journey_pattern_parse.h"
#include "motis/loader/util.h"
#include "motis/schedule-format/Schedule_generated.h"

namespace fbs64 = flatbuffers64;
namespace fs = boost::filesystem;
namespace xml = pugi;
using namespace motis::logging;

namespace motis::loader::netex {

bool netex_parser::applicable(fs::path const& p) {
  return fs::is_regular_file(p) && p.extension().generic_string() == ".zip";
}

std::vector<std::string> netex_parser::missing_files(fs::path const&) const {
  return {};
}

void netex_parser::parse(fs::path const& p,
                         flatbuffers64::FlatBufferBuilder& fbb) {
  utl::verify(fs::is_regular_file(p), "{} is not a zip file", p);
  auto const output_services = std::vector<fbs64::Offset<Service>>{};
  auto fbs_stations = std::map<std::string, fbs64::Offset<Station>>{};
  auto fbs_routes = std::vector<fbs64::Offset<Route>>{};
  auto const interval = Interval{0, 0};
  auto const footpaths = std::vector<fbs64::Offset<Footpath>>{};
  auto const rule_services = std::vector<fbs64::Offset<RuleService>>{};
  auto const meta_stations = std::vector<fbs64::Offset<MetaStation>>{};
  auto const dataset_name = "test";
  auto const hash = 123;

  auto const zeit_messen = false;
  auto const z = zip_reader{p.generic_string().c_str()};
  for (auto file = z.read(); file.has_value(); file = z.read()) {
    std::cout << z.current_file_name() << "\n";
    try {
      auto start = std::chrono::high_resolution_clock::now();
      xml::xml_document d;
      auto const r = d.load_buffer(reinterpret_cast<void const*>(file->data()),
                                   file->size());

      utl::verify(r, "netex parser: invalid xml in {}", z.current_file_name());
      // parse
      auto const days_map = combine_daytyps_uic_opertions(d);
      // 3 noch ok oder in struct?
      auto l_m = std::map<std::string, line>{};
      auto s_m = std::map<std::string, scheduled_points>{};
      auto d_m = std::map<std::string, direction>{};
      parse_frame(d, l_m, s_m, d_m);
      auto sjp_m = std::map<std::string, service_journey_pattern>{};
      parse_service_journey_pattern(d, sjp_m);
      auto sj_m = std::map<std::string, service_journey>{};
      parse_service_journey(d, sj_m);
      // search and build .fbs files
      // TODO dummyweise hier, das rest ermal geht
      auto const season = CreateSeason(fbb, 0, 0, 0, 0);
      auto const timezone = CreateTimezone(fbb, 0, season);
      for (auto const& sj : sj_m) {
        std::cout << sj.second.key_sjp_ << std::endl;
        auto const it_sjp = sjp_m.lower_bound(sj.second.key_sjp_);

        auto a_v = std::vector<fbs64::Offset<Attribute>>{};
        get_attribute_fbs(sj.second.keys_day_,
                          it_sjp->second.notice_assignments_, days_map, a_v,
                          fbb);
        auto category = fbs64::Offset<Category>{};
        auto provider = fbs64::Offset<Provider>{};
        int name;
        auto const lines = it_sjp->second.lines_;
        get_provider_operator_fbs(lines, l_m, category, provider, name, fbb);
        auto stations_v = std::vector<fbs64::Offset<Station>>{};
        // TODO
        auto in_allowed_v = std::vector<uint8_t>{};
        auto out_allowed_v = std::vector<uint8_t>{};
        for (auto const& ttpt : sj.second.keys_ttpt_) {
          auto s_d_s = station_dir_section{};
          s_d_s.l_m_ = l_m;
          s_d_s.s_m_ = s_m;
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
          in_allowed_v.push_back(0u);
          out_allowed_v.push_back(0u);
        }
        auto const route = CreateRoute(
            fbb,
            fbb.CreateVector(utl::to_vec(
                begin(stations_v), end(stations_v),
                [&](fbs64::Offset<Station> const& s) { return s; })),
            fbb.CreateVector(utl::to_vec(begin(in_allowed_v), end(in_allowed_v),
                                         [](uint8_t const& i) { return i; })),
            fbb.CreateVector(utl::to_vec(begin(out_allowed_v),
                                         end(out_allowed_v),
                                         [](uint8_t const& o) { return o; })));
      }

      // TODO create schedule, Interval ulong?, Footpath = Quay?, RuleService?,
      // MetaStations?, name, hash?

    } catch (std::exception const& e) {
      LOG(error) << "unable to parse message: " << e.what();
    } catch (...) {
      LOG(error) << "unable to parse message";
    }
  }
  fbb.Finish(CreateSchedule(
      fbb, fbb.CreateVector(output_services),
      fbb.CreateVector(values(fbs_stations)), fbb.CreateVector(fbs_routes),
      &interval, fbb.CreateVector(footpaths), fbb.CreateVector(rule_services),
      fbb.CreateVector(meta_stations), fbb.CreateString(dataset_name), hash));
}

}  // namespace motis::loader::netex
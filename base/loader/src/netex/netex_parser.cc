#include "motis/loader/netex/netex_parser.h"

#include <cstring>
#include <iostream>
#include <vector>
#include "date/date.h"
#include "date/tz.h"

#include "boost/filesystem.hpp"
#include "pugixml.hpp"

#include "utl/get_or_create.h"
#include "utl/verify.h"

#include "motis/core/common/logging.h"
#include "motis/core/common/zip_reader.h"
#include "motis/loader/netex/builder/main_builder.h"
#include "motis/loader/netex/days_parse.h"
#include "motis/loader/netex/service_frame/service_frame.h"
#include "motis/loader/netex/service_frame/service_frame_parse.h"
#include "motis/loader/netex/service_journey/service_journey.h"
#include "motis/loader/netex/service_journey/service_journey_parse.h"
#include "motis/loader/netex/service_journey_interchange/service_journey_interchange.h"
#include "motis/loader/netex/service_journey_interchange/service_journey_interchange_parse.h"
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
  auto output_services = std::vector<fbs64::Offset<Service>>{};
  auto fbs_stations = std::map<std::string, fbs64::Offset<Station>>{};
  auto fbs_routes = std::vector<fbs64::Offset<Route>>{};
  auto const interval = Interval{0, 0};
  auto const footpaths = std::vector<fbs64::Offset<Footpath>>{};
  auto rule_services = std::vector<fbs64::Offset<RuleService>>{};
  auto const meta_stations = std::vector<fbs64::Offset<MetaStation>>{};
  auto const dataset_name = "test";
  auto const hash = 123;

  auto const z = zip_reader{p.generic_string().c_str()};
  for (auto file = z.read(); file.has_value(); file = z.read()) {
    std::cout << z.current_file_name() << "\n";
    try {
      xml::xml_document d;
      auto const r = d.load_buffer(reinterpret_cast<void const*>(file->data()),
                                   file->size());

      utl::verify(r, "netex parser: invalid xml in {}", z.current_file_name());
      // parse
      // 4 noch ok oder in struct?
      auto l_m = std::map<std::string, line>{};
      auto s_m = std::map<std::string, scheduled_points>{};
      auto d_m = std::map<std::string, direction>{};
      auto p_m = std::map<std::string, passenger_assignments>{};
      parse_frame(d, l_m, s_m, d_m, p_m);
      auto sjp_m = std::map<std::string, service_journey_pattern>{};
      parse_service_journey_pattern(d, sjp_m);
      auto sj_m = std::map<std::string, service_journey>{};
      parse_service_journey(d, sj_m);
      auto sji_v = std::vector<service_journey_interchange>{};
      parse_service_journey_interchange(d, sji_v);
      // ServiceCalendarFrame, so ist ja coby Rückgabe und eher schlecht?
      auto const days_m = combine_daytyps_uic_opertions(d);
      /*for (auto const& ele :
           d.select_nodes("/PublicationDelivery/dataObjects/"
                          "CompositeFrame/FrameDefaults/DefaultLocale")) {
        auto const default_timezone_name =
            std::string(ele.node().child("TimeZone").text().as_string());
        auto current_time = std::chrono::system_clock::now();
        auto utc = date::zoned_time{"America/Los_Angeles", current_time};
        auto local = date::zoned_time{default_timezone_name, current_time};
        auto offset =
            date::format("%T\n", local.get_local_time() - utc.get_local_time());
        auto t = offset.substr(0, 3);
        std::cout << t;
        // in stunden für motis zu minuten transferieren, siehe docu von
        // delfi
        auto const default_offset =
            ele.node().child("SummerTimeZoneOffset").text().as_int() / 60;
        auto const default_general_offset =
            ele.node().child("TimeZoneOffset").text().as_int() / 60;
      }*/

      auto b = build{};
      b.l_m_ = l_m;
      b.s_m_ = s_m;
      b.d_m_ = d_m;
      b.p_m_ = p_m;
      b.sjp_m_ = sjp_m;
      b.sj_m_ = sj_m;
      b.sji_v_ = sji_v;
      b.days_m_ = days_m;
      b.file_ = z.current_file_name();
      auto sjpp = std::vector<service_journey_parse>{};
      build_fbs(b, sjpp, fbb);
      for (auto const& ele : sjpp) {
        auto in_allowed_v = std::vector<bool>{};
        auto out_allowed_v = std::vector<bool>{};
        auto stations_v = std::vector<fbs64::Offset<Station>>{};
        for (auto const& sta : ele.ttpt_index_) {
          auto const station = CreateStation(
              fbb, to_fbs_string(fbb, sta.st_dir_.name_),
              to_fbs_string(fbb, sta.st_dir_.name_), sta.st_dir_.lat_,
              sta.st_dir_.lng_, 0, NULL, sta.st_dir_.timezone_,
              to_fbs_string(fbb, sta.st_dir_.timezone_name_));
          // Für route
          stations_v.push_back(station);
          in_allowed_v.push_back(sta.in_allowed_);
          out_allowed_v.push_back(sta.out_allowed_);
          fbs_stations.try_emplace(sta.stop_point_ref_, station);
          auto const direction = CreateDirection(
              fbb, station, to_fbs_string(fbb, sta.st_dir_.direction_));
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
        fbs_routes.push_back(route);
      }

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

#include "motis/loader/netex/netex_parser.h"

#include <string.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include "date/date.h"
#include "date/tz.h"

#include "boost/filesystem.hpp"
#include "pugixml.hpp"

#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "utl/get_or_create.h"
#include "utl/verify.h"

#include "motis/core/common/date_time_util.h"
#include "motis/core/common/logging.h"
#include "motis/core/common/zip_reader.h"
#include "motis/loader/netex/builder/main_builder.h"
#include "motis/loader/netex/days/days_parse.h"
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
  auto output_services = std::map<std::string, fbs64::Offset<Service>>{};
  auto fbs_stations = std::map<std::string, fbs64::Offset<Station>>{};
  auto fbs_routes = std::vector<fbs64::Offset<Route>>{};
  auto const t1 = to_unix_time(2021, 1, 1);
  auto const t2 = to_unix_time(2023, 1, 1);
  auto const interval = Interval(t1, t2);
  auto const footpaths = std::vector<fbs64::Offset<Footpath>>{};
  auto rule_services = std::vector<fbs64::Offset<RuleService>>{};
  auto const meta_stations = std::vector<fbs64::Offset<MetaStation>>{};
  auto const dataset_name = p.generic_string().c_str();
  auto const hash = 123;
  auto const z = zip_reader{p.generic_string().c_str()};
  for (auto file = z.read(); file.has_value(); file = z.read()) {
    std::cout << z.current_file_name() << "\n";
    try {
      xml::xml_document d;
      auto const r = d.load_buffer(reinterpret_cast<void const*>(file->data()),
                                   file->size());
      // utl::verify(r, "netex parser: invalid xml in {}",
      // z.current_file_name());
      auto l_m = std::map<std::string, line>{};
      auto s_m = std::map<std::string, scheduled_points>{};
      auto s_p_m = std::map<std::string, stop_point>{};
      auto d_m = std::map<std::string, direction>{};
      auto p_m = std::map<std::string, passenger_assignments>{};
      parse_frame(d, l_m, s_m, s_p_m, d_m, p_m);
      auto sjp_m = std::map<std::string, service_journey_pattern>{};
      parse_service_journey_pattern(d, sjp_m);
      auto sj_m = std::map<std::string, service_journey>{};
      parse_service_journey(d, sj_m);
      auto sji_v = std::vector<service_journey_interchange>{};
      parse_service_journey_interchange(d, sji_v);
      auto const days_m = parse_daytypes_uicoperation(d);
      auto const season_m = get_season_times(days_m);
      auto const stations_m = get_stations(s_m, s_p_m, p_m);
      auto const b = build{l_m,      d_m,   stations_m, days_m,
                           season_m, sjp_m, sj_m,       z.current_file_name(),
                           t1};
      auto sjpp = std::vector<section_route>{};
      build_fbs(b, sjpp, fbb);
      auto services = std::map<std::string, fbs64::Offset<Service>>{};
      create_stations_routes_services_fbs(
          sjpp, std::string(z.current_file_name()), fbs_stations, fbs_routes,
          output_services, fbb);
      create_rule_service(sji_v, output_services, fbs_stations, rule_services,
                          fbb);
      std::cout << fbs_routes.size() << std::endl;
      std::cout << fbs_stations.size() << std::endl;
    } catch (std::exception const& e) {
      LOG(error) << "unable to parse message: " << e.what();
      std::cout << e.what();
    } catch (...) {
      LOG(error) << "unable to parse message";
    }
  }

  fbb.Finish(CreateSchedule(
      fbb, fbb.CreateVector(values(output_services)),
      fbb.CreateVector(values(fbs_stations)), fbb.CreateVector(fbs_routes),
      &interval, fbb.CreateVector(footpaths), fbb.CreateVector(rule_services),
      fbb.CreateVector(meta_stations), fbb.CreateString(dataset_name), hash));
}
}  // namespace motis::loader::netex

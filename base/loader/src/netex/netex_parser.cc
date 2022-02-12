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

#include "utl/get_or_create.h"
#include "utl/verify.h"

#include "motis/core/common/date_time_util.h"
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
#include "motis/loader/netex/verfiry_netex_checker.h"
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
  auto interval = Interval{0, 0};
  auto const footpaths = std::vector<fbs64::Offset<Footpath>>{};
  auto rule_services = std::vector<fbs64::Offset<RuleService>>{};
  auto const meta_stations = std::vector<fbs64::Offset<MetaStation>>{};
  auto const dataset_name = "test";
  auto const hash = 123;
  // --import.data_dir /Users/reneprinz/Downloads --import.paths
  // schedule:/Users/reneprinz/Downloads/20211029_fahrplaene_gesamtdeutschland.zip
  // --dataset.write_serialized=false
  //--import.data_dir /Users/reneprinz/Downloads /Applications/motis
  //--import.paths
  // schedule:/Users/reneprinz/Downloads/NX-PI-01_DE_NAP_LINE_123-ERLBUS-371_20211029.zip
  //--dataset.write_serialized=false
  auto const z = zip_reader{p.generic_string().c_str()};
  for (auto file = z.read(); file.has_value(); file = z.read()) {
    std::cout << z.current_file_name() << "\n";
    try {
      xml::xml_document d;
      auto const r = d.load_buffer(reinterpret_cast<void const*>(file->data()),
                                   file->size());
      utl::verify(r, "netex parser: invalid xml in {}", z.current_file_name());
      // TODO noch implementieren
      verfiy_xml_header(d);
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
      auto const season_m = get_season_times(days_m);
      auto b = build{};
      b.l_m_ = l_m;
      b.s_m_ = s_m;
      b.d_m_ = d_m;
      b.p_m_ = p_m;
      b.sjp_m_ = sjp_m;
      b.sj_m_ = sj_m;
      b.days_m_ = days_m;
      b.seasons_m_ = season_m;
      b.file_ = z.current_file_name();
      // std::cout << "After parse" << verfiy_build(b) << std::endl;
      //  if (!verfiy_build(b)) {
      if (false) {
        auto sjpp = std::vector<service_journey_parse>{};
        build_fbs(b, sjpp, fbb);
        std::cout << "After build fbs" << std::endl;
        auto services = std::map<std::string, fbs64::Offset<Service>>{};
        create_stations_routes_services_fbs(
            sjpp, std::string(z.current_file_name()), fbs_stations, fbs_routes,
            output_services, fbb);
        std::cout << "After create s_r_s fbs" << std::endl;
        create_rule_service(sji_v, output_services, fbs_stations, rule_services,
                            fbb);
        std::cout << "After create fbs" << std::endl;
        std::this_thread::sleep_until(std::chrono::system_clock::now() +
                                      std::chrono::seconds(5));
      }
    } catch (std::exception const& e) {
      LOG(error) << "unable to parse message: " << e.what();
    } catch (...) {
      LOG(error) << "unable to parse message";
    }
  }

  /* <FromDate>2021-10-15T00:00:00</FromDate>
 <ToDate>2021-12-11T00:00:00</ToDate>*/
  auto t1 = to_unix_time(2021, 10, 15);
  auto t2 = to_unix_time(2022, 12, 11);
  interval = Interval(t1, t2);

  fbb.Finish(CreateSchedule(
      fbb, fbb.CreateVector(values(output_services)),
      fbb.CreateVector(values(fbs_stations)), fbb.CreateVector(fbs_routes),
      &interval, fbb.CreateVector(footpaths), fbb.CreateVector(rule_services),
      fbb.CreateVector(meta_stations), fbb.CreateString(dataset_name), hash));
}

}  // namespace motis::loader::netex

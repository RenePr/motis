#include "motis/loader/netex/netex_parser.h"

#include <iostream>

#include "boost/filesystem.hpp"

#include "pugixml.hpp"

#include "utl/verify.h"
#include "utl/get_or_create.h"

#include "motis/core/common/logging.h"
#include "motis/core/common/zip_reader.h"
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
  auto const fbs_stations = std::map<std::string, fbs64::Offset<Station>>{};
  auto const fbs_routes = std::map<std::string, fbs64::Offset<Route>>{};
  auto const interval = Interval{0, 0};
  auto const footpaths = std::vector<fbs64::Offset<Footpath>>{};
  auto const rule_services = std::vector<fbs64::Offset<RuleService>>{};
  auto const meta_stations = std::vector<fbs64::Offset<MetaStation>>{};
  auto const dataset_name = "test";
  auto const hash = 123;

  auto const z = zip_reader{p.generic_string().c_str()};
  for (auto file = z.read(); file.has_value(); file = z.read()) {
    std::cout << z.current_file_name() << "\n";
    std::cout << "     "
              << file->substr(0, std::min(file->size(), size_t{100U})) << "\n";

    try {
      xml::xml_document d;
      auto r = d.load_buffer(reinterpret_cast<void const*>(file->data()),
                             file->size());
      utl::verify(r, "netex parser: invalid xml in {}", z.current_file_name());
      //bekomme traffic days
      for (auto const& daytype : d.select_nodes("//DayTypeAssignment")) {

        auto const& operation_period = daytype.node().child("OperatingPeriodRef").attribute("ref").value();
        for(auto const& operating_period : d.select_nodes("//UicOperatingPeriod")) {
          if(operation_period == operating_period.node().attribute("id").value()) {
            //fbs: service traffic days
            //std::cout << operating_period.node().child("ValidDayBits").text().get() << std::endl;
            //std::cout << operating_period.node().child("FromDate").text().get() << std::endl;
            //std::cout << operating_period.node().child("ToDate").text().get() << std::endl;
            //std::cout << "Same operation id, ." << std::endl;
          }
        }
        //journeyPattern
        //d.select_nodes("//ServiceJourneyPattern/pointsInSequence/StopPointInJourneyPattern")
        for(auto const& service_journey : d.select_nodes("//ServiceJourneyPattern")) {

          for(auto const& stop_point : service_journey.node().select_nodes("pointsInSequence")) {
            //fbs route, in_allowed out_allowed
            auto const& for_alighting =  stop_point.node().child("StopPointInJourneyPattern").child("ForAlighting").text().get();
            auto const& for_boarding = stop_point.node().child("StopPointInJourneyPattern").child("ForBoarding").text().get();
            //std::cout << stop_point.node().child("StopPointInJourneyPattern").child("ForBoarding").text().get() << std::endl;
          }
            for(auto const& line_ref : service_journey.node().select_nodes("RouteView")) {
              auto const& line = line_ref.node().child("LineRef").attribute("ref").value();
              //std::cout << line_ref.node().child("LineRef").attribute("ref").value() << std::endl;
            }
            for(auto const& direction_ref : service_journey.node().select_nodes("DirectionRef")) {
              auto const& direction = direction_ref.node().attribute("ref").value();
              //std::cout << direction_ref.node().attribute("ref").value() << std::endl;
            }
            for(auto const& notice_assignment : service_journey.node().select_nodes("noticeAssignments")) {
              auto const& notice_text = notice_assignment.node().child("NoticeAssignment").child("Notice").child("Text").text().get();
              auto const& notice_public_code = notice_assignment.node().child("NoticeAssignment").child("Notice").child("PublicCode").text().get();
              //std::cout << notice_assignment.node().child("NoticeAssignment").child("Notice").child("Text").text().get() << std::endl;
            }


        }

        //day_type.name_ = station.node().child("Name").text().get();
        //st1.lat_ = dynamic_cast<double>(station.node().child("Latitude").text().get());
        //st1.lng_ = static_cast<double>(station.node().child("Longitude").text().get());
        //stations.insert(std::pair<std::string, station_netex>(st1.id_, st1));
        //std::cout << "     " << assignment.operating_period_ref_ << "\n";
        /*auto const new_station =
            utl::get_or_create(stations, station.node().attribute("id").value(), [&]() {
              return std::make_unique<station_netex>();
            });
         //new_station->id_ = station.node().attribute("id").value();
         //new_station->name_ = station.node().child("Name").text().get();
         new_station->name_ = station.node().child("Name").text().get();

        station_map.insert(std::pair<std::string, station_netex>(st1.id_, st1));

        //std::cout << "     " << st1.id_ << st1.name_ << "\n"; */

      }
      //fbs64::Offset<station_netex> test =  stb.get_or_create_station(stations.begin()->first, fbb);
      //TODO get_or_greate  here..

      //parse ..

    } catch (std::exception const& e) {
      LOG(error) << "unable to parse message: " << e.what();
    } catch (...) {
      LOG(error) << "unable to parse message";
    }
  }
  fbb.Finish(CreateSchedule(
      fbb, fbb.CreateVector(output_services),
      fbb.CreateVector(values(fbs_stations)),
      fbb.CreateVector(values(fbs_routes)), &interval,
      fbb.CreateVector(footpaths), fbb.CreateVector(rule_services),
      fbb.CreateVector(meta_stations), fbb.CreateString(dataset_name), hash));
}

}  // namespace motis::loader::netex
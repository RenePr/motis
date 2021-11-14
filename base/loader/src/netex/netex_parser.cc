#include "motis/loader/netex/netex_parser.h"

#include <iostream>
#include <cstring>

#include "boost/filesystem.hpp"

#include "pugixml.hpp"

#include "utl/verify.h"
#include "utl/get_or_create.h"

#include "motis/core/common/logging.h"
#include "motis/core/common/zip_reader.h"
#include "motis/loader/util.h"
#include "motis/schedule-format/Schedule_generated.h"
#include "motis/loader/netex/service_journey.h"

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
      for(auto const& service : d.select_nodes("//ServiceJourney")) {
        service_journey service_jor;
        for(auto const& day_type_ref : service.node().select_nodes("//DayTypeRef")) {
          service_jor.day_type_ref_ = day_type_ref.node().attribute("ref").value();

          std::cout << day_type_ref.node().attribute("ref").value() << std::endl;
          //bekomme traffic days
          for (auto const& daytype : d.select_nodes("//DayTypeAssignment")) {
            auto const& operation_period = daytype.node().child("OperatingPeriodRef").attribute("ref").value();
            auto const& day = daytype.node().child("DayTypeRef").attribute("ref").value();
            //std::cout << day << operation_period << std::endl;
            if(day == service_jor.day_type_ref_) {
              for (auto const& period :
                   d.select_nodes("//UicOperatingPeriod")) {
                if (std::strcmp(operation_period
                    ,period.node().attribute("id").value()) == 0) {
                  // fbs: service traffic days
                  auto const& valid_day_bits = period.node()
                                                   .child("ValidDayBits")
                                                   .text()
                                                   .get();
                  auto const& from_date = period.node()
                                              .child("FromDate")
                                              .text()
                                              .get();
                  auto const& to_date = period.node()
                                            .child("ToDate")
                                            .text()
                                            .get();
                  std::cout << period.node()
                                   .child("ValidDayBits")
                                   .text()
                                   .get()
                            << std::endl;

                }
              }
            }
          }
          for(auto const& service_journey_pattern_ref : service.node().select_nodes("//ServiceJourneyPatternRef")) {
            service_jor.service_journey_pattern_ref_ = service_journey_pattern_ref.node().attribute("ref").value();
            //std::cout << service_journey_pattern_ref.node().attribute("ref").value() << std::endl;
            //journeyPattern
            //d.select_nodes("//ServiceJourneyPattern/pointsInSequence/StopPointInJourneyPattern")
            for(auto const& service_journey : d.select_nodes("//ServiceJourneyPattern")) {
              if(service_jor.service_journey_pattern_ref_ == service_journey.node().attribute("id").value() ) {

                for (auto const& stop_point :
                   service_journey.node().select_nodes("pointsInSequence")) {
                  // fbs route, in_allowed out_allowed
                  auto const& for_alighting =
                    stop_point.node()
                        .child("StopPointInJourneyPattern")
                        .child("ForAlighting")
                        .text()
                        .get();
                  auto const& for_boarding =
                    stop_point.node()
                        .child("StopPointInJourneyPattern")
                        .child("ForBoarding")
                        .text()
                        .get();
                  // std::cout << stop_point.node().child("StopPointInJourneyPattern").child("ForBoarding").text().get() << std::endl;
                }
                for (auto const& line_ref :
                   service_journey.node().select_nodes("RouteView")) {
                    auto const& line =
                      line_ref.node().child("LineRef").attribute("ref").value();
                    // std::cout << line_ref.node().child("LineRef").attribute("ref").value() << std::endl;
                }
                for (auto const& direction_ref :
                   service_journey.node().select_nodes("DirectionRef")) {
                  auto const& direction =
                    direction_ref.node().attribute("ref").value();
                  // std::cout << direction_ref.node().attribute("ref").value() << std::endl;
                }
                for (auto const& notice_assignment :
                   service_journey.node().select_nodes("noticeAssignments")) {
                  auto const& notice_text = notice_assignment.node()
                                              .child("NoticeAssignment")
                                              .child("Notice")
                                              .child("Text")
                                              .text()
                                              .get();
                  auto const& notice_public_code = notice_assignment.node()
                                                     .child("NoticeAssignment")
                                                     .child("Notice")
                                                     .child("PublicCode")
                                                     .text()
                                                     .get();
                  // std::cout << notice_assignment.node().child("NoticeAssignment").child("Notice").child("Text").text().get() << std::endl;
                }
              }
            }
          }
          //service_list.push_back(service_jor);
        }
      }
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
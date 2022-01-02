#include "motis/loader/netex/netex_parser.h"

#include <iostream>
#include <cstring>
#include <vector>

#include "boost/filesystem.hpp"

#include "pugixml.hpp"

#include "utl/verify.h"
#include "utl/get_or_create.h"

#include "motis/core/common/logging.h"
#include "motis/core/common/zip_reader.h"
#include "motis/loader/netex/service_frame_parse.h"
#include "motis/loader/netex/service_journey.h"
#include "motis/loader/util.h"
#include "motis/schedule-format/Schedule_generated.h"
#include "motis/loader/netex/service_frame.h"
#include "motis/loader/netex/get_valid_day_bits_transform.h"
#include "motis/loader/netex/days_parse.h"

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
  utl::verify(fs::is_regular_file(p), "{} is not a zip file",p);
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
    //std::cout << "     "
    //          << file->substr(0, std::min(file->size(), size_t{100U})) << "\n";

    try {
      auto start = std::chrono::high_resolution_clock::now();
      xml::xml_document d;
      auto r = d.load_buffer(reinterpret_cast<void const*>(file->data()),
                             file->size());

      utl::verify(r, "netex parser: invalid xml in {}", z.current_file_name());

      auto const& days_map = combine_daytyps_uic_opertions(d);
      auto operator_map = parse_operator(d);
      //std::map<std::string, Operator_Authority> operator_map = parse_operator(d);
      std::map<std::string, direction> direction;
      std::map<std::string, line> line;
      std::map<std::string, scheduled_points> scheduled_points;

      for(auto const& service_frame : d.select_nodes("//dataObjects/CompositeFrame/frames/ServiceFrame")) {
        direction = parse_direction(service_frame);
        line = parse_line(service_frame, operator_map);
        scheduled_points = parse_scheduled_points(service_frame, d);
      }
      std::map<std::string, std::string> vehicle_type;
      for(auto const& timetable_frame : d.select_nodes("//dataObjects/CompositeFrame/frames/TimetableFrame/vehicleTypes/VehicleType")) {
        auto const key = timetable_frame.node().attribute("id").as_string();
        auto const type = timetable_frame.node().child("Name").text().as_string();
        //auto const pair = std::make_pair(timetable_frame.node().attribute("id").as_string(), type);
        //vehicle_type.insert(pair);
        vehicle_type.try_emplace(key, type);
      }

      //std::map<std::string ,fbs64::Offset<Provider>> provider;
      //std::map<std::string ,fbs64::Offset<Category>> category;
      std::map<std::string, fbs64::Offset<Service>> service;

      for(auto const& service_journey_pattern : d.select_nodes("//dataObjects/CompositeFrame/frames/ServiceFrame/journeyPatterns/ServiceJourneyPattern")) {
        for(auto const& line_ref : service_journey_pattern.node().select_nodes("//RouteView/LineRef")) {
          auto const key = line_ref.node().attribute("ref").as_string();
          auto const it = line.find(key);
          utl::verify(it != end(line), "missing category: {}",
                      key);
          //TODO timezone_name fehlt!
          auto const prov = CreateProvider(fbb, to_fbs_string(fbb, it->second.operator_.short_name_), to_fbs_string(fbb, it->second.operator_.name_), to_fbs_string(fbb, it->second.operator_.legal_name_), to_fbs_string(fbb, it->second.operator_.name_));

          //TODO output_rule fehlt
          //auto cate = CreateCategory(fbb, to_fbs_string(fbb, it->second.transport_mode_), 0.0);

        }

        for(auto const& notice_ref : service_journey_pattern.node().select_nodes("//noticeAssignments/NoticeAssignment")) {

        }
        //auto const& op = line.operator_;
      }

      for(auto const& service_journey : d.select_nodes("//dataObjects/CompositeFrame/frames/TimetableFrame/vehicleJourneys/ServiceJourney")) {
        // service_journey service_jor;
        for(auto const& day_type_ref : service_journey.node().select_nodes("//dayTypes/DayTypeRef")) {
         auto const& key = day_type_ref.node().attribute("ref").as_string();
            if(days_map.count(key) > 0) {
            auto uic_key = days_map.at(key).uic_id_;

             //std::cout << "Key from DayTypeRef exists" <<  days.at(key).uic_id_ <<days.at(key).uic_.at(uic_key).valid_day_bits_ << std::endl;
          }
        }
        for(auto const& vehicle : service_journey.node().select_nodes("//VehicleTypeRef")) {
          auto key = vehicle.node().attribute("ref").as_string();
          if(vehicle_type.find(key) != vehicle_type.end()) {

          }
        }
          //journeyPattern
          //d.select_nodes("//ServiceJourneyPattern/pointsInSequence/StopPointInJourneyPattern")
        /*for(auto const& service_journey : d.select_nodes("//ServiceJourneyPattern")) {

          if(service_jor.service_journey_pattern_ref_ == service_journey.node().attribute("id").value() ) {
            for (auto const& stop_point :
                   service_journey.node().select_nodes("pointsInSequence")) {
                  // fbs route, in_allowed out_allowed
                  service_jor.for_alighting_ =
                    stop_point.node()
                        .child("StopPointInJourneyPattern")
                        .child("ForAlighting")
                        .text()
                        .as_bool();
                  service_jor.for_boarding_ =
                    stop_point.node()
                        .child("StopPointInJourneyPattern")
                        .child("ForBoarding")
                        .text()
                        .as_bool();
                  //TODO
                  // Wie vergleiche ich attribute ref mit id im ganzen xml file
                  // gibt es hier eine fkt dafür?
                  // d.select_nodes()
                  // find_attribute gibt nur eins zurück und stoppt dann...
                  // iterator? vermutlich deutlich langsamer
                  // neue fkt speziell dafür programmieren?
                }
                //std::cout << "Here? " <<std::endl;
                //pugi::xpatSh_node_set & line = service_journey.node().select_nodes("RouteView");

                std::vector<line> line_test = parse_line(service_journey , d);

                for (auto const& direction_ref :
                   service_journey.node().select_nodes("DirectionRef")) {
                  auto const& direction =
                    direction_ref.node().attribute("ref").value();
                  // std::cout << direction_ref.node().attribute("ref").value() << std::endl;
                  for(auto const& dir : d.select_nodes("//Direction")) {
                   // std::cout << dir.node().attribute("id").value() << direction << std::endl;
                    if(std::strcmp(dir.node().attribute("id").value(), direction) == 0) {
                      //childs: Name, ShortName
                      //std::cout << "here?" << std::endl;
                      service_jor.name_direction_ = dir.node().child("Name").text().get();
                      service_jor.short_name_direction_ = dir.node().child("ShortName").text().get();
                    }
                  }
                }
                for (auto const& notice_assignment :
                   service_journey.node().select_nodes("noticeAssignments")) {
                  service_jor.notice_text_= notice_assignment.node()
                                              .child("NoticeAssignment")
                                              .child("Notice")
                                              .child("Text")
                                              .text()
                                              .get();
                  service_jor.public_code_= notice_assignment.node()
                                                     .child("NoticeAssignment")
                                                     .child("Notice")
                                                     .child("PublicCode")
                                                     .text()
                                                     .get();
                  auto const& start_point_ref = notice_assignment.node()
                                                    .child("NoticeAssignment")
                                                    .child("StartPointInPatternRef")
                                                    .attribute("id")
                                                    .value();

                  auto const& stop_point_ref = notice_assignment.node()
                                                    .child("NoticeAssignment")
                                                    .child("StopPointInPatternRef")
                                                    .attribute("id")
                                                    .value();

                  // std::cout << notice_assignment.node().child("NoticeAssignment").child("Notice").child("Text").text().get() << std::endl;
                }
              }
             }*/
          //service_list.push_back(service_jor);
        }
        //for(auto const& at : service_list) {
         // if(std::strcmp(at.name_direction_.c_str(), "1") == 1 || std::strcmp(at.name_direction_.c_str(), "2") == 1 || std::strcmp(at.name_direction_.c_str(), "H") == 1 || std::strcmp(at.name_direction_.c_str(), "R") == 1 ){
            //std::cout << at.name_direction_ << std::endl;
         // }
            //std::cout << "Line: " << at.name_ << " Dir: " << at.name_direction_ << " Auth: " << at.authority_.name_ << " Op: " << at.operator_.name_ << std::endl;
        //}
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
        std::cout << duration.count() << std::endl;
    } catch (std::exception const& e) {
      LOG(error) << "unable to parse message: " << e.what();
    } catch (...) {
      LOG(error) << "unable to parse message";
    }

    std::cout << "finish" << std::endl;
  }
  fbb.Finish(CreateSchedule(
      fbb, fbb.CreateVector(output_services),
      fbb.CreateVector(values(fbs_stations)),
      fbb.CreateVector(values(fbs_routes)), &interval,
      fbb.CreateVector(footpaths), fbb.CreateVector(rule_services),
      fbb.CreateVector(meta_stations), fbb.CreateString(dataset_name), hash));
}

}  // namespace motis::loader::netex
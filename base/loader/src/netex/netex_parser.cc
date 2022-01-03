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

      for(auto const& s : d.select_nodes("//dataObjects/CompositeFrame/frames/ServiceFrame")) {
        direction = parse_direction(s);
        line = parse_line(s, operator_map);
        scheduled_points = parse_scheduled_points(s, d);
      }
      std::map<std::string, std::string> vehicle_type;
      for(auto const& t : d.select_nodes("//dataObjects/CompositeFrame/frames/TimetableFrame/vehicleTypes/VehicleType")) {
        auto const key = std::string(t.node().attribute("id").as_string());
        auto const type = std::string(t.node().child("Name").text().as_string());
        vehicle_type.try_emplace(key, type);
      }

      std::map<std::string, fbs64::Offset<Service>> service;

      for(auto const& s : d.select_nodes("//dataObjects/CompositeFrame/frames/ServiceFrame/journeyPatterns/ServiceJourneyPattern")) {
        std::vector<fbs64::Offset<Station>> stations_map;
        for(auto const& s : s.node().select_nodes("//pointsInSequence/StopPointInJourneyPattern")) {
          auto const key = std::string(s.node().child("ScheduledStopPointRef").attribute("ref").as_string());
          auto const it = scheduled_points.find(key);
          utl::verify(it != end(scheduled_points), "missing category: {}",
                      key);
          //TODO dummyweise hier
          auto season = CreateSeason(fbb, 0, 0, 0, 0, 0);
          //TODO dummyweise hier
          auto timezone = CreateTimezone(fbb, 0, season);
          //TODO station id == name?, interchange_name ?, external ids?
          std::vector<std::string> test;
          test.push_back("test");

          auto const st = CreateStation(fbb, to_fbs_string(fbb, key), to_fbs_string(fbb, key), it->second.stop_point_.lat_, it->second.stop_point_.lon_, 0, fbb.CreateVector(utl::to_vec(
                                                                                                                                                                begin(test), end(test),
                                                                                                                                                                [&](std::string const& s) { return fbb.CreateString(s); })) ,timezone, to_fbs_string(fbb, std::string(it->second.stop_point_.timezone_)));
          stations_map.push_back(st);
        }
        std::vector<fbs64::Offset<Attribute>> attribute_vec;
        for(auto const& n : s.node().select_nodes("//noticeAssignments/NoticeAssignment")) {
          auto const key = std::string(n.node().child("Notice").attribute("id").as_string());
          auto const text = std::string(n.node().child("Notice").child("Text").text().as_string());
          auto const public_code = std::string(n.node().child("Notice").child("PublicCode").text().as_string());
          auto attribute_info = CreateAttributeInfo(fbb, to_fbs_string(fbb, text), to_fbs_string(fbb, public_code));
          //TODO traffic days hier
          std::string test = "1";
          auto attribute = CreateAttribute(fbb, attribute_info ,to_fbs_string(fbb, test));
          //attribute_map.try_emplace(key, attribute);
          attribute_vec.push_back(attribute);
        }

        for(auto const& l : s.node().select_nodes("//RouteView/LineRef")) {
          auto const key = std::string(l.node().attribute("ref").as_string());
          auto const it = line.find(key);
          utl::verify(it != end(line), "missing category: {}",
                      key);
          //TODO timezone_name fehlt, keine timezone bei operator und provider und die timezone ist ja nicht gleich der timezone der Stationen oder ?!?
          auto prov = CreateProvider(fbb, to_fbs_string(fbb, it->second.operator_.short_name_), to_fbs_string(fbb, it->second.operator_.name_), to_fbs_string(fbb, it->second.operator_.legal_name_), to_fbs_string(fbb, it->second.operator_.name_));
          //TODO output_rule fehlt
          auto category = CreateCategory(fbb, to_fbs_string(fbb, std::string(it->second.transport_mode_)), 0.0);

          auto dir = CreateDirection(fbb, stations_map.emplace_back() );

          auto const section = CreateSection(fbb, category ,prov, 0, to_fbs_string(fbb, key), fbb.CreateVector(utl::to_vec(
                                                                              begin(attribute_vec), end(attribute_vec),
                                                                              [&](fbs64::Offset<Attribute> const& a) { return a; })),  dir);

        }
        //auto const& op = line.operator_;
      }

      for(auto const& s : d.select_nodes("//dataObjects/CompositeFrame/frames/TimetableFrame/vehicleJourneys/ServiceJourney")) {
        // service_journey service_jor;
        for(auto const& d : s.node().select_nodes("//dayTypes/DayTypeRef")) {
         auto const& k = d.node().attribute("ref").as_string();
            if(days_map.count(k) > 0) {
            auto uic_key = days_map.at(k).uic_id_;

             //std::cout << "Key from DayTypeRef exists" <<  days.at(key).uic_id_ <<days.at(key).uic_.at(uic_key).valid_day_bits_ << std::endl;
          }
        }
        for(auto const& vehicle : s.node().select_nodes("//VehicleTypeRef")) {
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
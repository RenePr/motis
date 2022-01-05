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
#include "motis/loader/netex/service_journey_pattern.h"
#include "motis/loader/netex/service_journey_pattern_parse.h"

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
    auto size = fs::file_size(p);
    std::cout << size << std::endl;

    try {
      auto start = std::chrono::high_resolution_clock::now();
      xml::xml_document d;
      auto r = d.load_buffer(reinterpret_cast<void const*>(file->data()),
                             file->size());

      utl::verify(r, "netex parser: invalid xml in {}", z.current_file_name());
      std::cout << "Here?2" << std::endl;
      auto days_map = combine_daytyps_uic_opertions(d);
      auto operator_map = parse_operator(d);
      //TODO noch auslagern
      std::map<std::string, std::string> vehicle_type;
      for(auto const& v : d.select_nodes("//PublicationDelivery/dataObjects/CompositeFrame/frames/TimetableFrame/vehicleTypes/VehicleType")) {
        auto const key = std::string(v.node().attribute("id").as_string());
        auto const type = std::string(v.node().child("Name").text().as_string());
        vehicle_type.try_emplace(key, type);
      }

      std::map<std::string, direction> direction;
      std::map<std::string, line> line;
      std::map<std::string, scheduled_points> scheduled_points;
      for(auto const& s : d.select_nodes("/PublicationDelivery/dataObjects/CompositeFrame/frames/ServiceFrame")) {
        //std::cout << "Here" << std::endl;
        direction = parse_direction(s);
        //std::cout << "h0" << std::endl;
        line = parse_line(s, operator_map);
        //std::cout << "h1" << std::endl;
        scheduled_points = parse_scheduled_points(s, d);
        //std::cout << "h2" << std::endl;
      }
      //std::cout << "h3" << std::endl;
        std::map<std::string, service_journey_pattern> service_journey_pattern_map;
        //ServiceJourneyPattern
        for (auto const& s : d.select_nodes("/PublicationDelivery/dataObjects/CompositeFrame/frames/ServiceFrame/journeyPatterns/ServiceJourneyPattern")) {
          auto const key_service =
              std::string(s.node().attribute("id").as_string());
          service_journey_pattern sjp;
          sjp.direction_ = std::string(
              s.node().child("DirectionRef").attribute("ref").as_string());

          std::vector<fbs64::Offset<Station>> stations_vec;
          for (auto const& s : s.node().select_nodes("//pointsInSequence/StopPointInJourneyPattern")) {
            auto const key = std::string(s.node()
                                             .child("ScheduledStopPointRef")
                                             .attribute("ref")
                                             .as_string());
            auto const it = scheduled_points.find(key);
            utl::verify(it != end(scheduled_points), "missing category: {}",
                        key);
            // TODO station id == name?, interchange_name ?, external ids?
            std::vector<std::string> test;
            test.push_back("test");
            /*auto const st = CreateStation(
                fbb, to_fbs_string(fbb, key), to_fbs_string(fbb, key),
                it->second.stop_point_.lat_, it->second.stop_point_.lon_, 0,
                fbb.CreateVector(utl::to_vec(
                    begin(test), end(test),
                    [&](std::string const& s) { return fbb.CreateString(s); })),
                timezone,
                to_fbs_string(fbb,
                              std::string(it->second.stop_point_.timezone_)));
            stations_vec.push_back(st);*/
          }
          sjp.stations_vec_ = stations_vec;

          std::vector<fbs64::Offset<AttributeInfo>> attribute_vec;
          for (auto const& n :
               s.node().select_nodes("//noticeAssignments/NoticeAssignment")) {
            auto const key = std::string(
                n.node().child("Notice").attribute("id").as_string());
            auto const text = std::string(
                n.node().child("Notice").child("Text").text().as_string());
            auto const public_code = std::string(n.node()
                                                     .child("Notice")
                                                     .child("PublicCode")
                                                     .text()
                                                     .as_string());
            auto attribute_info = CreateAttributeInfo(
                fbb, to_fbs_string(fbb, text), to_fbs_string(fbb, public_code));
            attribute_vec.push_back(attribute_info);
          }
          sjp.attributeinfo_vec_ = attribute_vec;

          for (auto const& l : s.node().select_nodes("//RouteView/LineRef")) {
            auto const key = std::string(l.node().attribute("ref").as_string());
            if (line.size() != 1) {
              std::cout << "Warning more than one line in one file"
                        << std::endl;
            }
            auto const it = line.lower_bound(key);
            try {
              utl::verify(it != end(line), "missing line: {}", key);
              sjp.name_ = it->second.name_;
            } catch (std::runtime_error& e) {
              std::cout << e.what() << std::endl;
              continue;
            }
            // TODO timezone_name fehlt, keine timezone bei operator und provider und die timezone ist ja nicht gleich der timezone der Stationen oder ?!?
            /*auto prov = CreateProvider(
                fbb, to_fbs_string(fbb, it->second.operator_.short_name_),
                to_fbs_string(fbb, it->second.operator_.name_),
                to_fbs_string(fbb, it->second.operator_.legal_name_),
                to_fbs_string(fbb, it->second.operator_.name_));
            // TODO output_rule fehlt
            auto category = CreateCategory(
                fbb,
                to_fbs_string(fbb, std::string(it->second.transport_mode_)),
                0.0);

            sjp.provider_ = prov;
            sjp.category_ = category;*/
            //std::cout << sjp.name_ << std::endl;
            // sjp.name_ = key;

            // auto const section = CreateSection(fbb, category ,prov, 0, to_fbs_string(fbb, key), fbb.CreateVector(utl::to_vec(
            //                                                                     begin(attribute_vec), end(attribute_vec),
            //                                                                     [&](fbs64::Offset<Attribute> const& a) { return a; })),  dir);
          }
          service_journey_pattern_map.try_emplace(key_service, sjp);
          //std::cout << "sjp" << std::endl;
        }


      for(auto const& s : d.select_nodes("//dataObjects/CompositeFrame/frames/TimetableFrame/vehicleJourneys/ServiceJourney")) {
        auto key_sjp = std::string(s.node()
                                       .child("ServiceJourneyPatternRef")
                                       .attribute("ref")
                                       .as_string());
        auto it_sjp = service_journey_pattern_map.lower_bound(key_sjp);
        try {
          utl::verify(it_sjp != end(service_journey_pattern_map),
                      "missing service_journey_pattern: {}", key_sjp);
        } catch (std::runtime_error& e) {
          std::cout << e.what() << std::endl;
          continue;
        }
        std::string valid_day_bits;
        // TODO auslagern
        for (auto const& d : s.node().select_nodes("//dayTypes/DayTypeRef")) {
          auto const key = std::string(d.node().attribute("ref").as_string());
          //days_map.lower_bound(key);
          auto it = days_map.lower_bound(key);
          try {
            utl::verify(it != end(days_map), "missing day_types: {}", key);
            std::string uic_id = it->second.uic_id_;
            try {
              auto it_uic_id = it->second.uic_.lower_bound(uic_id);
              utl::verify(it_uic_id != end(it->second.uic_),
                          "missing service_journey_pattern: {}", uic_id);
              valid_day_bits =
                  std::string(it->second.uic_.at(uic_id).valid_day_bits_);
            } catch (std::runtime_error& e) {
              std::cout << e.what() << std::endl;
              continue;
            }
          } catch (std::runtime_error& e) {
            std::cout << e.what() << std::endl;
            continue;
          }
        }
        auto attribute =
            utl::to_vec(begin(it_sjp->second.attributeinfo_vec_),
                        end(it_sjp->second.attributeinfo_vec_),
                        [&](fbs64::Offset<AttributeInfo> const& ai) {
                          return CreateAttribute(
                              fbb, ai, to_fbs_string(fbb, valid_day_bits));
                        });
        // TODO stations anpassen
        /*auto const dir =
            CreateDirection(fbb, it_sjp->second.stations_vec_.emplace_back(),
                            to_fbs_string(fbb, it_sjp->second.direction_));
        auto const section = CreateSection(
            fbb, it_sjp->second.category_, it_sjp->second.provider_, 0,
            to_fbs_string(fbb, key),
            fbb.CreateVector(utl::to_vec(
                begin(attribute), end(attribute),
                [&](fbs64::Offset<Attribute> const& a) { return a; })),
            dir);*/
        for (auto const& vehicle : s.node().select_nodes("//VehicleTypeRef")) {
          auto key = vehicle.node().attribute("ref").as_string();
          /*if (vehicle_type.lower_bound(key) != vehicle_type.end()) {
          }*/
        }
        }
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
        std::cout << duration.count() << std::endl;
    } catch (std::exception const& e) {
      LOG(error) << "unable to parse message: " << e.what();
    } catch (...) {
      LOG(error) << "unable to parse message";
    }

    //std::cout << "finish" << std::endl;

  }
  fbb.Finish(CreateSchedule(
      fbb, fbb.CreateVector(output_services),
      fbb.CreateVector(values(fbs_stations)),
      fbb.CreateVector(values(fbs_routes)), &interval,
      fbb.CreateVector(footpaths), fbb.CreateVector(rule_services),
      fbb.CreateVector(meta_stations), fbb.CreateString(dataset_name), hash));
}

}  // namespace motis::loader::netex
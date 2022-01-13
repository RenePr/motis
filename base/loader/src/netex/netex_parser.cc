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
  auto fbs_stations = std::map<std::string, fbs64::Offset<Station>>{};
  auto const fbs_routes = std::map<std::string, fbs64::Offset<Route>>{};
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
    //std::cout << "     "
    //          << file->substr(0, std::min(file->size(), size_t{100U})) << "\n";
    //auto size = fs::file_size(p);
    //std::cout << size << std::endl;

    try {
      auto start = std::chrono::high_resolution_clock::now();
      xml::xml_document d;
      auto r = d.load_buffer(reinterpret_cast<void const*>(file->data()),
                             file->size());

      utl::verify(r, "netex parser: invalid xml in {}", z.current_file_name());
      //std::cout << "Here?2" << std::endl;
      auto days_map = combine_daytyps_uic_opertions(d);
      auto operator_map = parse_operator(d);
      //TODO dummyweise hier, das rest ermal geht
      auto const season = CreateSeason(fbb, 0, 0, 0 , 0);
      auto const timezone = CreateTimezone(fbb, 0, season);
      //TODO noch auslagern
      auto vehicle_type = std::map<std::string, std::string>{};
      for(auto const& v : d.select_nodes("//PublicationDelivery/dataObjects/CompositeFrame/frames/TimetableFrame/vehicleTypes/VehicleType")) {
        auto const key = std::string(v.node().attribute("id").as_string());
        auto const type = std::string(v.node().child("Name").text().as_string());
        vehicle_type.try_emplace(key, type);
      }
      auto direction_map = std::map<std::string, direction>{};
      auto line_map = std::map<std::string, line>{};
      auto scheduled_point_map = std::map<std::string, scheduled_points>{};
      for(auto const& s : d.select_nodes("/PublicationDelivery/dataObjects/CompositeFrame/frames/ServiceFrame")) {
        //std::cout << "Here" << std::endl;
        direction_map = parse_direction(s);
        //std::cout << "h0" << std::endl;
        line_map = parse_line(s, operator_map);
        //std::cout << "h1" << std::endl;
        scheduled_point_map = parse_scheduled_points(s, d);
        //std::cout << "h2" << std::endl;
      }//ServiceFrame
      if(zeit_messen) {
        auto stop_sf = std::chrono::high_resolution_clock::now();
        auto duration_sf =
            std::chrono::duration_cast<std::chrono::milliseconds>(stop_sf -
                                                                  start);
        std::cout << "sf " << duration_sf.count() << std::endl;
      }
      auto service_journey_pattern_map = std::map<std::string, service_journey_pattern>{};
      //ServiceJourneyPattern
      for (auto const& s : d.select_nodes("/PublicationDelivery/dataObjects/CompositeFrame/frames/ServiceFrame/journeyPatterns/ServiceJourneyPattern")) {
          auto const key_service =
              std::string(s.node().attribute("id").as_string());
          auto sjp = service_journey_pattern{};
          sjp.direction_ = std::string(
              s.node().child("DirectionRef").attribute("ref").as_string());
          auto attribute_vec = std::vector<fbs64::Offset<AttributeInfo>>{};
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
            sjp.start_point_in_journey_pattern_ = std::string(
                n.node().child("StartPointInPatternRef").attribute("ref").as_string());
            sjp.stop_point_in_journey_pattern_ = std::string(
                n.node().child("StopPointInPatternRef").attribute("ref").as_string());

          }//NoticeAssignment
          sjp.attributeinfo_vec_ = attribute_vec;

          for (auto const& l : s.node().select_nodes("//RouteView/LineRef")) {
            auto const key = std::string(l.node().attribute("ref").as_string());
            if (line_map.size() != 1) {
              std::cout << "Warning more than one line in one file"
                        << std::endl;
            }
            auto const it = line_map.lower_bound(key);
            try {
              utl::verify(it != end(line_map), "missing line: {}", key);
              sjp.name_ = it->second.name_;
            } catch (std::runtime_error& e) {
              std::cout << e.what() << std::endl;
              continue;
            }
            // TODO timezone_name fehlt, keine timezone bei operator und provider und die timezone ist ja nicht gleich der timezone der Stationen oder ?!?
            auto prov = CreateProvider(
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
            sjp.category_ = category;
          }//LineRef
          auto stop_point_map = std::map<std::string, stop_point_in_journey_pattern>{};
          for(auto const& sp : s.node().select_nodes("//pointsInSequence/StopPointInJourneyPattern")) {
            auto const key = std::string(sp.node().attribute("id").as_string());
            auto stopPointInJourneyPattern = stop_point_in_journey_pattern{};
            stopPointInJourneyPattern.id_ = std::string_view(sp.node().child("ScheduledStopPointRef").attribute("ref").as_string());
            stopPointInJourneyPattern.in_allowed_ = sp.node().child("ForBoarding").text().as_bool();
            stopPointInJourneyPattern.out_allowed_ = sp.node().child("ForAlighting").text().as_bool();
            stop_point_map.try_emplace(key,stopPointInJourneyPattern);
          }//StopPointInJourneyPattern
          sjp.stop_point_map = stop_point_map;
          service_journey_pattern_map.try_emplace(key_service, sjp);
      }//ServiceJourneyPattern
      if(zeit_messen) {
        auto stop_sjp = std::chrono::high_resolution_clock::now();
        auto duration_sjp =
            std::chrono::duration_cast<std::chrono::milliseconds>(stop_sjp -
                                                                  start);
        std::cout << "sjp " << duration_sjp.count() << std::endl;
      }

      int start_point, stop_point;
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
                          "missing uic_id: {}", uic_id);
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
        }//DayTypes
        if(zeit_messen) {
          auto stop_day = std::chrono::high_resolution_clock::now();
          auto duration_day =
              std::chrono::duration_cast<std::chrono::milliseconds>(stop_day -
                                                                    start);
          std::cout << "day " << duration_day.count() << std::endl;
        }
        auto const attribute =
            utl::to_vec(begin(it_sjp->second.attributeinfo_vec_),
                        end(it_sjp->second.attributeinfo_vec_),
                        [&](fbs64::Offset<AttributeInfo> const& ai) {
                          return CreateAttribute(
                              fbb, ai, to_fbs_string(fbb, valid_day_bits));
                        });
        if(zeit_messen) {
          auto stop_att = std::chrono::high_resolution_clock::now();
          auto duration_att =
              std::chrono::duration_cast<std::chrono::milliseconds>(stop_att -
                                                                    start);
          std::cout << "attribute " << duration_att.count() << std::endl;
        }
        auto in_allowed_vec = std::vector<uint8_t>{};
        auto out_allowed_vec = std::vector<uint8_t>{};
        auto stations_vec = std::vector<fbs64::Offset<Station>>{};
        auto section_vec = std::vector<fbs64::Offset<Section>>{};
        for(auto const& s_p : s.node().select_nodes("//passingTimes/TimetabledPassingTime")) {
          auto const key = std::string(s_p.node()
                                           .child("StopPointInJourneyPatternRef")
                                           .attribute("ref")
                                           .as_string());

          auto const it_stop_point_pattern = it_sjp->second.stop_point_map.lower_bound(key);
          utl::verify(it_stop_point_pattern != end(it_sjp->second.stop_point_map), "missing stop_point_in_journey_pattern: {}",
                      key);

          auto const key_scheduled_points = std::string(it_stop_point_pattern->second.id_);
          auto const it_scheduled_point = scheduled_point_map.lower_bound(key_scheduled_points);
          utl::verify(it_scheduled_point != end(scheduled_point_map), "missing schelduled_point: {}",
                      key);
          auto const key_sv = std::string_view(key);
          auto const key_start = std::string_view(it_sjp->second.start_point_in_journey_pattern_);
          auto const key_stop = std::string_view(it_sjp->second.stop_point_in_journey_pattern_);
          if(key_sv == key_start) {
            //start_point = static_cast<int>(std::string(it_scheduled_point->second.short_name_));
            start_point = 0;
          } else if(key_sv == key_stop) {
            //stop_point = static_cast<int>(std::string(it_scheduled_point->second.short_name_));
            stop_point = 0;
          }
          //TODO in_allowed_vec as bool und anschließend in uin8_t umwandeln. Dummyweise nur so hier
          auto const out_allowed = static_cast<uint>(it_stop_point_pattern->second.out_allowed_);
          out_allowed_vec.push_back(0);
          auto const in_allowed = static_cast<uint>(it_stop_point_pattern->second.in_allowed_);
          in_allowed_vec.push_back(0);

          auto test = std::vector<std::string>{};
          test.push_back(std::string("test"));
          // TODO station id == name?, interchange_time =  ?, external ids?
          auto st = CreateStation(
              fbb, to_fbs_string(fbb, key), to_fbs_string(fbb, key),
              it_scheduled_point->second.stop_point_.lat_, it_scheduled_point->second.stop_point_.lon_, 0,
              fbb.CreateVector(utl::to_vec(
                  begin(test), end(test),
                  [&](std::string const& s) { return fbb.CreateString(s); })),
              timezone,
              to_fbs_string(fbb,
                            std::string(it_scheduled_point->second.stop_point_.timezone_)));
          stations_vec.push_back(st);
          fbs_stations.try_emplace(key, st);
          //TODO check if direction exists
          // TODO stations anpassen
          auto const dir =
              CreateDirection(fbb, st,
                              to_fbs_string(fbb, it_sjp->second.direction_));
          //TODO which name=line id?, train nummer=name as_int() geht?
          auto const section = CreateSection(
              fbb, it_sjp->second.category_, it_sjp->second.provider_, it_sjp->second.name_,
              to_fbs_string(fbb, std::string(begin(line_map)->second.id_)),
              fbb.CreateVector(utl::to_vec(
                  begin(attribute), end(attribute),
                  [&](fbs64::Offset<Attribute> const& a) { return a; })),
              dir);
          section_vec.push_back(section);
          //std::cout << "Here?" << std::endl;
        }//TimetablePassingTime
        if(zeit_messen) {
          auto stop_tpt = std::chrono::high_resolution_clock::now();
          auto duration_tpt =
              std::chrono::duration_cast<std::chrono::milliseconds>(stop_tpt -
                                                                    start);
          std::cout << "tpt " << duration_tpt.count() << std::endl;
        }
        //falsch aber überhaupt notwendig?
        /*if(stations_vec.size() != in_allowed_vec.size() != out_allowed_vec.size()) {
          std::cout << "Wrong size" << std::endl;
          continue;
        }*/
        auto const route = CreateRoute(fbb, fbb.CreateVector(utl::to_vec(begin(stations_vec), end(stations_vec), [&](fbs64::Offset<Station> const& s) {return s;})),
                                       fbb.CreateVector(utl::to_vec(begin(in_allowed_vec), end(in_allowed_vec), [](uint8_t const& i)  {return i;})),
                                       fbb.CreateVector(utl::to_vec(begin(out_allowed_vec), end(out_allowed_vec), [](uint8_t const& o) {return o;})));
        if(zeit_messen) {
          auto stop_r = std::chrono::high_resolution_clock::now();
          auto duration_r =
              std::chrono::duration_cast<std::chrono::milliseconds>(stop_r -
                                                                    start);
          std::cout << "r " << duration_r.count() << std::endl;
        }
        auto const service_debug = CreateServiceDebugInfo(fbb, to_fbs_string(fbb, std::string(z.current_file_name())) ,start_point, stop_point);
        // wzl-BUS-1 über passengerassignment,
        // 209-wefra über schedulestoppoint name -> quay
        // gar nicht
        //TODO trackroules= new over passengerassignment -> quay, times in siteconnection WalkTransferDuration?, routekey uint?, rule_participant?, initial_train_nr?, trip id optional ?
        //auto const service = CreateService(fbb, route, to_fbs_string(fbb, valid_day_bits), ,);

        for (auto const& vehicle : s.node().select_nodes("//VehicleTypeRef")) {
          auto const key = vehicle.node().attribute("ref").as_string();
          /*if (vehicle_type.lower_bound(key) != vehicle_type.end()) {
          }*/
        }
      }//ServiceJourney

      //TODO create schedule, Interval?, Footpath, RuleService?, MetaStations?, name, hash?
      //auto const schedule = CreateSchedule();
      if(zeit_messen) {
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        std::cout << duration.count() << std::endl;
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
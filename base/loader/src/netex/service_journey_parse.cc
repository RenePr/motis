#include "motis/loader/netex/service_journey/service_journey_parse.h"

#include <map>

#include "pugixml.hpp"

#include "motis/loader/netex/service_journey/service_journey.h"

namespace xml = pugi;

namespace motis::loader::netex {

void parse_service_journey(xml::xml_document& d,
                           std::map<std::string, service_journey>& sj_m) {
  for (auto const& sj : d.select_nodes("/PublicationDelivery/dataObjects/"
                                       "CompositeFrame/frames/TimetableFrame/"
                                       "vehicleJourneys/ServiceJourney")) {
    auto const key_sj = std::string(sj.node().attribute("id").as_string());
    auto service_j = service_journey{};
    service_j.key_sj_ = key_sj;
    service_j.key_sjp_ = sj.node()
                             .child("ServiceJourneyPatternRef")
                             .attribute("ref")
                             .as_string();

    std::vector<std::string> keys_days;
    // TODO auslagern
    for (auto const& d : sj.node().select_nodes(".//dayTypes/DayTypeRef")) {
      auto const key = std::string(d.node().attribute("ref").as_string());
      keys_days.push_back(key);
    }  // DayTypes
    service_j.keys_day_ = keys_days;
    auto ttpt_v = std::vector<time_table_passing_time>{};
    for (auto const& tpt :
         sj.node().select_nodes(".//passingTimes/TimetabledPassingTime")) {
      auto ttpt = time_table_passing_time{};
      ttpt.stop_point_ref_ = tpt.node()
                                 .child("StopPointInJourneyPatternRef")
                                 .attribute("ref")
                                 .as_string();
      ttpt.arr_time_ = tpt.node().child("ArrivalTime").text().as_string();
      ttpt.dep_time_ = tpt.node().child("DepartureTime").text().as_string();
      ttpt_v.push_back(ttpt);

    }  // TimetablePassingTime
    service_j.keys_ttpt_ = ttpt_v;
    sj_m.try_emplace(key_sj, service_j);
  }
}

}  // namespace motis::loader::netex
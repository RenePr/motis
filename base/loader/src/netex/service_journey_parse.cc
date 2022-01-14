#include "motis/loader/netex/service_journey/service_journey_parse.h"

#include <iostream>
#include <map>
#include <string_view>

#include "pugixml.hpp"

#include "motis/loader/netex/service_journey.h"
#include "motis/loader/netex/service_journey_pattern/service_journey_pattern.h"

namespace xml = pugi;

namespace motis::loader::netex {

void parse_service_journey(xml::xml_document& d,
                           std::map<std::string, service_journey>& sj_m) {
  for (auto const& sj :
       d.select_nodes("//dataObjects/CompositeFrame/frames/TimetableFrame/"
                      "vehicleJourneys/ServiceJourney")) {
    auto const key_sj = std::string(sj.node().attribute("id").as_string());
    auto service_j = service_journey{};
    service_j.key_sjp_ = std::string(sj.node()
                                         .child("ServiceJourneyPatternRef")
                                         .attribute("ref")
                                         .as_string());

    std::vector<std::string> keys_days;
    // TODO auslagern
    for (auto const& d : sj.node().select_nodes("//dayTypes/DayTypeRef")) {
      auto const key = std::string(d.node().attribute("ref").as_string());
      // days_map.lower_bound(key);
      keys_days.push_back(key);
    }  // DayTypes
    service_j.keys_day_ = keys_days;
    std::vector<std::string> ttpt_v;
    for (auto const& tpt :
         sj.node().select_nodes("//passingTimes/TimetabledPassingTime")) {
      auto key = std::string(tpt.node()
                                 .child("StopPointInJourneyPatternRef")
                                 .attribute("ref")
                                 .as_string());
      ttpt_v.push_back(key);

    }  // TimetablePassingTime

    service_j.keys_ttpt_ = ttpt_v;
    sj_m.try_emplace(key_sj, service_j);
  }
}

}  // namespace motis::loader::netex
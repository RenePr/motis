#include "motis/loader/netex/service_journey_pattern/service_journey_pattern_parse.h"

#include <map>

#include "pugixml.hpp"

#include "motis/loader/netex/service_journey_pattern/service_journey_pattern.h"

namespace xml = pugi;

namespace motis::loader::netex {

void parse_service_journey_pattern(
    xml::xml_document& d,
    std::map<std::string, service_journey_pattern>& sjp_m) {
  for (auto const& sjp :
       d.select_nodes("/PublicationDelivery/dataObjects/CompositeFrame/frames/"
                      "ServiceFrame/journeyPatterns/ServiceJourneyPattern")) {
    auto const key_sjp = std::string(sjp.node().attribute("id").as_string());
    auto sjps = service_journey_pattern{};
    sjps.direction_ =
        sjp.node().child("DirectionRef").attribute("ref").as_string();
    auto attribute_vec = std::vector<notice_assignment>{};
    for (auto const& n :
         sjp.node().select_nodes(".//noticeAssignments/NoticeAssignment")) {
      auto n_a = notice_assignment{};
      n_a.text_ = n.node().child("Notice").child("Text").text().as_string();
      n_a.public_code_ =
          n.node().child("Notice").child("PublicCode").text().as_string();
      n_a.start_point_in_journey_pattern_ =
          n.node().child("StartPointInPatternRef").attribute("ref").as_string();
      n_a.stop_point_in_journey_pattern_ =
          n.node().child("EndPointInPatternRef").attribute("ref").as_string();
      attribute_vec.push_back(n_a);
    }  // NoticeAssignment
    sjps.notice_assignments_ = attribute_vec;
    auto l_m = std::vector<std::string>{};
    for (auto const& l : sjp.node().select_nodes(".//RouteView/LineRef")) {
      auto const key_l = std::string(l.node().attribute("ref").as_string());
      l_m.push_back(key_l);
    }
    sjps.lines_ = l_m;
    auto stop_point_map =
        std::map<std::string, stop_point_in_journey_pattern>{};
    for (auto const& sp : sjp.node().select_nodes(
             ".//pointsInSequence/StopPointInJourneyPattern")) {
      auto const key = std::string(sp.node().attribute("id").as_string());
      auto stopPointInJourneyPattern = stop_point_in_journey_pattern{};
      stopPointInJourneyPattern.id_ =
          sp.node().child("ScheduledStopPointRef").attribute("ref").as_string();
      stopPointInJourneyPattern.in_allowed_ =
          sp.node().child("ForBoarding").text().as_bool();
      stopPointInJourneyPattern.out_allowed_ =
          sp.node().child("ForAlighting").text().as_bool();
      stop_point_map.try_emplace(key, stopPointInJourneyPattern);
    }  // StopPointInJourneyPattern
    sjps.stop_point_map_ = stop_point_map;
    sjp_m.try_emplace(key_sjp, sjps);
  }  // ServiceJourneyPattern
}
}  // namespace motis::loader::netex
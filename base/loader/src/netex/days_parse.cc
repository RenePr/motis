#include "motis/loader/netex/days/days_parse.h"

#include "motis/loader/netex/days/days.h"
#include <map>
#include <vector>
#include "utl/verify.h"

#include "pugixml.hpp"

namespace xml = pugi;

namespace motis::loader::netex {
std::map<std::string, daytype> transform_to_map_daytype(xml::xml_document& d) {
  std::map<std::string, daytype> daytypes_map;
  for (auto const& days :
       d.select_nodes("/PublicationDelivery/dataObjects/CompositeFrame/frames/"
                      "ServiceCalendarFrame/ServiceCalendar")) {
    daytype day_type;
    day_type.from_date_ =
        std::string_view{days.node().child("FromDate").text().as_string()};
    day_type.to_date_ =
        std::string_view{days.node().child("ToDate").text().as_string()};
    for (auto const& day : days.node().select_nodes(".//dayTypes/DayType")) {
      day_type.id_ = std::string_view{day.node().attribute("id").as_string()};
      day_type.version_ =
          std::string_view{day.node().attribute("version").as_string()};
      std::pair<std::string, daytype> pair =
          std::make_pair(day.node().attribute("id").as_string(), day_type);
      daytypes_map.insert(pair);
    }
  }
  return daytypes_map;
}

std::map<std::string, uic_opertion_period> transform_to_map_uic_opertion(
    xml::xml_document& d) {
  std::map<std::string, uic_opertion_period> daytypes_map;
  for (auto const& days :
       d.select_nodes("/PublicationDelivery/dataObjects/CompositeFrame/frames/"
                      "ServiceCalendarFrame/"
                      "ServiceCalendar/operatingPeriods/UicOperatingPeriod")) {
    uic_opertion_period uic;
    uic.from_date_ =
        std::string_view{days.node().child("FromDate").text().as_string()};
    uic.to_date_ =
        std::string_view{days.node().child("ToDate").text().as_string()};
    uic.valid_day_bits_ =
        std::string_view{days.node().child("ValidDayBits").text().as_string()};
    std::pair<std::string, uic_opertion_period> pair =
        std::make_pair(days.node().attribute("id").as_string(), uic);
    daytypes_map.insert(pair);
  }
  return daytypes_map;
}

std::map<std::string, ids> parse_daytypes_uicoperation(xml::xml_document& d) {
  auto const& daytype_map = transform_to_map_daytype(d);
  auto const& uic_map = transform_to_map_uic_opertion(d);
  std::map<std::string, ids> ret_unmap;
  ids ids;
  ids.uic_ = uic_map;
  ids.day_ = daytype_map;
  for (auto const& assignment :
       d.select_nodes("/PublicationDelivery/dataObjects/CompositeFrame/frames/"
                      "ServiceCalendarFrame/"
                      "ServiceCalendar/dayTypeAssignments/DayTypeAssignment")) {
    if (uic_map.find(assignment.node()
                         .child("OperatingPeriodRef")
                         .attribute("ref")
                         .as_string()) != uic_map.end()) {
      if (daytype_map.find(assignment.node()
                               .child("DayTypeRef")
                               .attribute("ref")
                               .as_string()) != daytype_map.end()) {
        ids.uic_id_ = assignment.node()
                          .child("OperatingPeriodRef")
                          .attribute("ref")
                          .as_string();
        auto const& pair = std::make_pair(
            assignment.node().child("DayTypeRef").attribute("ref").as_string(),
            ids);
        ret_unmap.insert(pair);
      }
    }
  }
  return ret_unmap;
}
}  // namespace motis::loader::netex
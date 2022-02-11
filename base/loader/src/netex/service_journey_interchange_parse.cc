#include "motis/loader/netex/service_frame/service_frame_parse.h"

#include <map>
#include <string_view>
#include <vector>

#include "pugixml.hpp"

#include "motis/loader/netex/service_journey_interchange/service_journey_interchange.h"
#include "motis/loader/netex/service_journey_interchange/service_journey_interchange_parse.h"

namespace xml = pugi;

namespace motis::loader::netex {
void parse_service_journey_interchange(
    xml::xml_document& d, std::vector<service_journey_interchange>& sji_m) {
  for (auto const sji :
       d.select_nodes("/PublicationDelivery/dataObjects/CompositeFrame/frames/"
                      "TimetableFrame/"
                      "journeyInterchanges/ServiceJourneyInterchange")) {
    auto sji_this = service_journey_interchange{};
    // TODO is_seated?
    sji_this.from_journey_ =
        std::string(sji.node().child("FromJourneyRef").text().as_string());
    sji_this.to_journey_ =
        std::string(sji.node().child("ToJourneyRef").text().as_string());
    sji_this.from_station_ =
        std::string(sji.node().child("FromPointRef").text().as_string());
    sji_this.to_station_ =
        std::string(sji.node().child("ToPointRef").text().as_string());
    sji_m.push_back(sji_this);
  }
}
}  // namespace motis::loader::netex
#include "motis/loader/netex/service_journey_interchange/service_journey_interchange.h"

#include <vector>

#include "pugixml.hpp"

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
    sji_this.from_journey_ =
        sji.node().child("FromJourneyRef").attribute("ref").as_string();
    sji_this.to_journey_ =
        sji.node().child("ToJourneyRef").attribute("ref").as_string();
    sji_this.from_station_ =
        sji.node().child("FromPointRef").attribute("ref").as_string();
    sji_this.to_station_ =
        sji.node().child("ToPointRef").attribute("ref").as_string();
    sji_this.stay_seated_ = sji.node().child("StaySeated").text().as_bool();
    sji_m.push_back(sji_this);
  }
}
}  // namespace motis::loader::netex
#include "motis/loader/netex/verfiry_netex_checker.h"

#include <iostream>
#include <map>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "pugixml.hpp"

#include "motis/core/common/logging.h"
#include "motis/loader/netex/builder/main_builder.h"
#include "motis/loader/netex/service_frame/service_frame.h"
#include "motis/schedule-format/Schedule_generated.h"

using namespace motis::logging;

namespace fbs64 = flatbuffers64;
namespace xml = pugi;

namespace motis::loader::netex {
void verfiy_xml_header(xml::xml_document& d) {}
void verfiy_build(build const& b) {
  if (b.seasons_m_.empty()) {
    LOG(error) << "daytype empty: ";
    // throw std::exception("daytype empty: ");
  }
  if (b.days_m_.empty()) {
    LOG(error) << "daytype empty: ";
    // throw std::exception("daytype empty: ");
  }
  if (b.sj_m_.empty()) {
    LOG(error) << "no service journey: ";
    // throw std::exception("no service journey: ");
  }
  if (b.sjp_m_.empty()) {
    LOG(error) << "no service journey pattern ";
    // throw std::exception("no service journey pattern ");
  }
  if (b.l_m_.empty()) {
    LOG(error) << "no lines ";
    // throw std::exception("no lines ");
  }
  if (b.s_m_.empty()) {
    LOG(error) << "no scheduled point ";
    // throw std::exception("no scheduled point ");
  }
  if (b.p_m_.empty()) {
    LOG(error) << "no service journey pattern ";
    // throw std::exception("no service journey pattern ");
  }
  if (b.d_m_.empty()) {
    LOG(error) << "no direction ";
    // throw "no direction ";
  }
}
}  // namespace motis::loader::netex
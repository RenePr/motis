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
bool verfiy_build(build const& b) {
  if (b.seasons_m_.empty()) {
    LOG(error) << "daytype empty: ";
    // throw std::exception("daytype empty: ");
    return true;
  }
  if (b.days_m_.empty()) {
    LOG(error) << "daytype empty: ";
    // throw std::exception("daytype empty: ");
    return true;
  }
  if (b.sj_m_.empty()) {
    LOG(error) << "no service journey: ";
    // throw std::exception("no service journey: ");
    return true;
  }
  if (b.sjp_m_.empty()) {
    LOG(error) << "no service journey pattern ";
    // throw std::exception("no service journey pattern ");
    return true;
  }
  if (b.l_m_.empty()) {
    LOG(error) << "no lines ";
    // throw std::exception("no lines ");
    return true;
  }
  if (b.s_m_.empty()) {
    LOG(error) << "no scheduled point ";
    // throw std::exception("no scheduled point ");
    return true;
  }
  if (b.p_m_.empty()) {
    LOG(error) << "no service journey pattern ";
    // throw std::exception("no service journey pattern ");
    return true;
  }
  if (b.d_m_.empty()) {
    LOG(error) << "no direction ";
    // throw "no direction ";
    return true;
  }
  return false;
}
}  // namespace motis::loader::netex
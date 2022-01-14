#include "motis/loader/netex/builder/helper_builder.h"

#include <iostream>
#include <map>
#include <string_view>

#include "pugixml.hpp"

#include "motis/schedule-format/Schedule_generated.h"

#include "motis/loader/netex/days.h"
#include "motis/loader/util.h"

namespace xml = pugi;
namespace fbs64 = flatbuffers64;

namespace motis::loader::netex {

std::string get_valid_day_bits(std::map<std::string, ids> const& days_map,
                               std::vector<std::string> keys) {
  auto valid_day_bits = std::string{};
  for (auto const& dt : keys) {
    auto const it = days_map.lower_bound(dt);
    auto key_uic = std::string(it->second.uic_id_);
    valid_day_bits = std::string(it->second.uic_.at(key_uic).valid_day_bits_);
  }
  return valid_day_bits;
}

void get_provider_operator_fbs(std::vector<std::string> const& lines,
                               std::map<std::string, line> const& l_m,
                               fbs64::Offset<Category>& category,
                               fbs64::Offset<Provider>& provider, int& name,
                               fbs64::FlatBufferBuilder& fbb) {
  for (auto const& l_r : lines) {
    if (l_m.size() != 1) {
      std::cout << "Warning more than one line in one file" << std::endl;
    }
    auto const it = l_m.lower_bound(l_r);
    try {
      utl::verify(it != end(l_m), "missing line: {}", l_r);
      name = it->second.name_;
    } catch (std::runtime_error& e) {
      std::cout << e.what() << std::endl;
      continue;
    }
    // TODO timezone_name fehlt, keine timezone bei operator und provider
    // und die timezone ist ja nicht gleich der timezone der Stationen
    // oder ?!?
    provider = CreateProvider(
        fbb, to_fbs_string(fbb, it->second.operator_.short_name_),
        to_fbs_string(fbb, it->second.operator_.name_),
        to_fbs_string(fbb, it->second.operator_.legal_name_),
        to_fbs_string(fbb, it->second.operator_.name_));
    // TODO output_rule fehlt
    auto s = std::string(it->second.transport_mode_);
    category = CreateCategory(fbb, to_fbs_string(fbb, s), 0.0);
  }
}

void get_attribute_fbs(std::vector<std::string> const& keys_day,
                       std::vector<notice_assignment> const& notice_assignments,
                       std::map<std::string, ids> const& days_map,
                       std::vector<fbs64::Offset<Attribute>>& attribute,
                       fbs64::FlatBufferBuilder& fbb) {
  auto notice_assignment_ve = std::vector<fbs64::Offset<AttributeInfo>>{};
  for (auto const& n_a : notice_assignments) {
    auto const attribute_info =
        CreateAttributeInfo(fbb, to_fbs_string(fbb, n_a.public_code_),
                            to_fbs_string(fbb, n_a.text_));
    notice_assignment_ve.push_back(attribute_info);
  }
  // for attribute
  auto valid_day_bits = get_valid_day_bits(days_map, keys_day);

  attribute = utl::to_vec(
      begin(notice_assignment_ve), end(notice_assignment_ve),
      [&](fbs64::Offset<AttributeInfo> const& ai) {
        return CreateAttribute(fbb, ai, to_fbs_string(fbb, valid_day_bits));
      });
}

}  // namespace motis::loader::netex
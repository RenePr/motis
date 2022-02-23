#pragma once

#include <iostream>
#include <map>

namespace motis::loader::netex {
struct daytype {
  std::string_view from_date_;
  std::string_view to_date_;
  std::string_view id_;
  std::string_view version_;
};
struct uic_opertion_period {
  std::string_view from_date_;
  std::string_view to_date_;
  std::string_view valid_day_bits_;
};

struct ids {
  std::string uic_id_;
  std::map<std::string, uic_opertion_period> uic_;
  std::map<std::string, daytype> day_;
};
}  // namespace motis::loader::netex
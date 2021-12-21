#pragma once

#include <iostream>
#include <map>

namespace motis::loader::netex {

struct Operator_Authority {
  std::string public_code_;
  std::string name_;
  std::string short_name_;
  std::string legal_name_;
};
struct line {
  std::string_view name_;
  std::string_view short_name_;
  //Todo Mabye enum?
  std::string_view transport_mode_;
  Operator_Authority operator_;
};

struct direction {
  std::string_view name_;
  std::string_view short_name_;
};

struct scheduled_points {
  std::string_view short_name_;
  std::string_view public_code_;
  std::string_view stop_type_;
};

} //motis::loader::netex
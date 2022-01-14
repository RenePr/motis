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
  int name_;
  std::string_view short_name_;
  //Todo Mabye enum?
  std::string_view transport_mode_;
  Operator_Authority operator_;

  std::string_view id_;
};

struct direction {
  std::string_view name_;
  std::string_view short_name_;
};

struct stop_point {
  std::string_view key_;
  std::string_view value_;
  std::string_view name_;
  double lon_;
  double lat_;
  std::string_view timezone_;

};

struct scheduled_points {
  std::string_view short_name_;
  std::string_view public_code_;
  std::string_view stop_type_;

  stop_point stop_point_;

};

struct notice {
  std::string_view text_;
  std::string_view code_;
};

} //motis::loader::netex
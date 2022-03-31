#pragma once

#include <iostream>
#include <map>
#include <vector>

namespace motis::loader::netex {

struct Operator_Authority {
  std::string public_code_;
  std::string name_;
  std::string short_name_;
  std::string legal_name_;
};
struct line {
  int name_;
  std::string short_name_;
  // Todo Mabye enum?
  std::string transport_mode_;
  Operator_Authority operator_;
  std::string id_;
};
struct direction {
  std::string name_;
  std::string short_name_;
};
struct passenger_assignments {
  std::string scheduled_place_id_;
  std::string stop_point_id_;
  std::string quay_id_;
};
struct stop_point {
  std::string key_;
  std::string value_;
  std::string name_;
  double lon_;
  double lat_;
  std::string timezone_;

  std::vector<std::string> quay_;
};
struct scheduled_points {
  std::string short_name_;
  std::string public_code_;
  std::string stop_type_;
  stop_point stop_point_;
};
struct notice {
  std::string text_;
  std::string code_;
};

}  // namespace motis::loader::netex
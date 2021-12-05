#pragma once

#include <iostream>
#include <vector>

namespace motis::loader::netex {

struct Operator_Authority {
  std::string public_code_;
  std::string name_;
  std::string short_name_;
  std::string legal_name_;
};
struct line {
  std::string name_;
  std::string short_name_;
  //Todo Mabye enum?
  std::string transport_mode_;
  std::vector<Operator_Authority> operator_;
  std::vector<Operator_Authority> authority_;
};

} //motis::loader::netex
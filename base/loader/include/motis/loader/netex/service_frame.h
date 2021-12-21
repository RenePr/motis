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
  std::string name_;
  std::string short_name_;
  //Todo Mabye enum?
  std::string transport_mode_;
  Operator_Authority operator_;
};

struct direction {
  std::string name_;
  std::string short_name_;
};

} //motis::loader::netex
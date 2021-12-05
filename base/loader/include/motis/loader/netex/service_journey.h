#pragma once

#include <iostream>

namespace motis::loader::netex {

//which name is a good name?
struct test {
  std::string public_code_;
  std::string name_;
  std::string short_name_;
  std::string legal_name_;
};

struct service_journey {
  //UicOperationPeriod
  std::string uic_operation_period_;
  std::string from_date_;
  std::string to_date_;
  //brauch ich das hier noch
  std::string service_journey_pattern_ref_;

  bool for_alighting_;
  bool for_boarding_;

  //Line
  std::string name_;
  std::string short_name_;
  std::string transport_mode_;

  //Authority
  test authority_;

  //Operator
  test operator_;

  //Direction
  std::string name_direction_;
  std::string short_name_direction_;

  //Attributes
  std::string public_code_;
  std::string notice_text_;


  //StartPoint

  //StopPoint


};

} //motis::loader::netex
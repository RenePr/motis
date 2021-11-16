#pragma once

#include <iostream>

namespace motis::loader::netex {

struct service_journey {
  //UicOperationPeriod
  std::string uic_operation_period_;
  std::string from_date_;
  std::string to_date_;
  //brauch ich das hier noch
  std::string service_journey_pattern_ref_;

  std::string for_alighting_;
  std::string for_boarding_;

  //Line
  std::string name_;
  std::string short_name_;
  std::string transport_mode_;
  //Authority
  std::string public_code_authority_;
  std::string name_authority_;
  std::string short_name_authority_;
  std::string legal_name_authority_;
  //Operator
  std::string public_code_operator_;
  std::string name_operator_;
  std::string short_name_operator_;
  std::string legal_name_operator_;
  //Direction
  std::string name_direction_;
  std::string short_name_direction_;


};

} //motis::loader::netex
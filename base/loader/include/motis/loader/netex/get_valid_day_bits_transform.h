# pragma once

#include <iostream>

namespace motis::loader::netex {

void get_valid_day_bits_transform(std::string_view valid_day_bits, std::string_view from_date, std::string_view to_date);

double get_days(std::string_view from_date, std::string_view to_date);

std::time_t get_time_t(std::string_view date);

} //motis::loader::netex
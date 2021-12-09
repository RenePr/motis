# pragma once

#include <iostream>

namespace motis::loader::netex {

void get_valid_day_bits_transform(const char * valid_day_bits, const char * from_date, const char * to_date);

double get_days(const char * from_date, const char * to_date);

std::time_t get_time_t(const char * date);

void parse_days(const char * valid_day_bits, std::time_t t1, std::time_t t2);

} //motis::loader::netex
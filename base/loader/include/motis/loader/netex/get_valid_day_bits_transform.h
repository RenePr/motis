# pragma once

#include <iostream>

namespace motis::loader::netex {
void get_valid_day_bits_transform(const char *, const char *, const char *);

double get_days(const char *, const char *);

std::time_t get_time_t(const char *);

void parse_days(const char *, std::time_t, std::time_t);

} //motis::loader::netex
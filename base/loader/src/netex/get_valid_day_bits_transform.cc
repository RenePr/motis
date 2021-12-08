#include "motis/loader/netex/get_valid_day_bits_transform.h"

#include <iostream>


namespace motis::loader::netex {


void get_valid_day_bits_transform(const char * valid_day_bits, const char * from_date, const char * to_date) {
  auto days  = get_days(to_date, from_date);
  std::cout << " Tage: " << days << " From date: " << from_date << " To Date: " << to_date << " Valid bits: " << valid_day_bits << std::endl;

}

double get_days(const char * from_date, const char * to_date) {
  //ctime, difftime time_t
  std::time_t t1 = get_time_t(from_date);
  std::time_t t2 = get_time_t(to_date);
  double sec =  std::difftime(t1, t2);
  return  sec / 86400;
}

std::time_t get_time_t(const char * date) {
  //muss leider so habe bisher keinen besseren weg gefunden
  //const char * test = std::string(date).c_str();
  int y, m, d, H, M, S;
  int ret = std::sscanf(date,  "%d-%d-%dT%d:%d:%d", &y, &m, &d, &H, &M, &S);
  std::tm time = {};
  time.tm_year = y;
  time.tm_mon = m;
  time.tm_mday = d;
  time.tm_hour = H;
  time.tm_min = M;
  time.tm_sec = S;

  std::cout << time.tm_year  << time.tm_mon  << time.tm_mday  << time.tm_hour  << time.tm_min  << time.tm_sec
             << std::endl;

  return std::mktime(&time);
}

} //motis::loader::netex
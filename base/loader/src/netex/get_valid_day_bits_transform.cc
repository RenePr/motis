#include "motis/loader/netex/get_valid_day_bits_transform.h"

#include <iostream>


namespace motis::loader::netex {

//wegen std::stringview keine &, da std::string_view speicheroptimiert ist und cheap-to-copy ist. siehe https://quuxplusone.github.io/blog/2021/11/09/pass-string-view-by-value/
void get_valid_day_bits_transform(std::string_view valid_day_bits, std::string_view from_date, std::string_view to_date) {
  auto days  = get_days(to_date, from_date);
  std::cout << " Tage: " << days << " From date: " << from_date << " To Date: " << to_date << " Valid bits: " << valid_day_bits <<  " Size: " << valid_day_bits.size() << std::endl;
  if(days - valid_day_bits.size() - 1 > 0) {
    //korrekt;
    bool test = true;
  }
}

double get_days(std::string_view from_date, std::string_view to_date) {
  //ctime, difftime time_t
  std::time_t t1 = get_time_t(from_date);
  std::time_t t2 = get_time_t(to_date);
  double sec =  std::difftime(t1, t2);
  return  sec / 86400;
}

std::time_t get_time_t(std::string_view date) {
  //muss leider so habe bisher keinen besseren weg gefunden
  const char * test = std::string(date).c_str();
  int y, m, d, H, M, S;
  int ret = std::sscanf(test,  "%d-%d-%dT%d:%d:%d", &y, &m, &d, &H, &M, &S);
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
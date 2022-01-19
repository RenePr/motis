#include "motis/loader/netex/builder/helper_builder.h"

#include <iostream>
#include <map>

#include "motis/schedule-format/Schedule_generated.h"

#include "motis/loader/netex/days.h"
#include "motis/loader/util.h"

namespace fbs64 = flatbuffers64;

namespace motis::loader::netex {

std::string get_valid_day_bits(std::map<std::string, ids> const& days_map,
                               std::vector<std::string> const& keys) {
  auto valid_day_bits = std::string{};
  for (auto const& dt : keys) {
    auto const it = days_map.lower_bound(dt);
    auto key_uic = std::string(it->second.uic_id_);
    valid_day_bits = std::string(it->second.uic_.at(key_uic).valid_day_bits_);
  }
  return valid_day_bits;
}

void get_provider_operator_fbs(std::vector<std::string> const& lines,
                               std::map<std::string, line> const& l_m,
                               fbs64::Offset<Category>& category,
                               fbs64::Offset<Provider>& provider, int& name,
                               fbs64::FlatBufferBuilder& fbb) {
  for (auto const& l_r : lines) {
    if (l_m.size() != 1) {
      std::cout << "Warning more than one line in one file" << std::endl;
    }
    auto const it = l_m.lower_bound(l_r);
    try {
      utl::verify(it != end(l_m), "missing line: {}", l_r);
      name = it->second.name_;
    } catch (std::runtime_error& e) {
      std::cout << e.what() << std::endl;
      continue;
    }
    provider = CreateProvider(
        fbb, to_fbs_string(fbb, it->second.operator_.short_name_),
        to_fbs_string(fbb, it->second.operator_.name_),
        to_fbs_string(fbb, it->second.operator_.legal_name_), NULL);
    // TODO output_rule fehlt
    auto s = std::string(it->second.transport_mode_);
    category = CreateCategory(fbb, to_fbs_string(fbb, s), NULL);
  }
}

void get_attribute_fbs(std::vector<std::string> const& keys_day,
                       std::vector<notice_assignment> const& notice_assignments,
                       std::map<std::string, ids> const& days_map,
                       std::vector<fbs64::Offset<Attribute>>& attribute,
                       fbs64::FlatBufferBuilder& fbb) {
  auto notice_assignment_ve = std::vector<fbs64::Offset<AttributeInfo>>{};
  for (auto const& n_a : notice_assignments) {
    auto const attribute_info =
        CreateAttributeInfo(fbb, to_fbs_string(fbb, n_a.public_code_),
                            to_fbs_string(fbb, n_a.text_));
    notice_assignment_ve.push_back(attribute_info);
  }
  // for attribute
  auto valid_day_bits = get_valid_day_bits(days_map, keys_day);

  attribute = utl::to_vec(
      begin(notice_assignment_ve), end(notice_assignment_ve),
      [&](fbs64::Offset<AttributeInfo> const& ai) {
        return CreateAttribute(fbb, ai, to_fbs_string(fbb, valid_day_bits));
      });
}
void get_station_dir_section(station_dir_section const& s_d_s,
                             uint8_t& in_allowed, uint8_t& out_allowed,
                             fbs64::Offset<Station>& station,
                             fbs64::Offset<Direction>& direction,
                             fbs64::Offset<Section>& section,
                             fbs64::Offset<Track>& track,
                             fbs64::FlatBufferBuilder& fbb) {
  auto const it_sp = s_d_s.s_p_m_.lower_bound(s_d_s.key_);
  utl::verify(it_sp != end(s_d_s.s_p_m_), "missing time_table_passing_time: {}",
              s_d_s.key_);
  auto const key_sp = std::string(it_sp->second.id_);
  in_allowed = it_sp->second.in_allowed_;
  out_allowed = it_sp->second.out_allowed_;
  auto const it = s_d_s.s_m_.lower_bound(key_sp);
  // external ids = leer
  // TODO interchange time
  station = CreateStation(
      fbb, to_fbs_string(fbb, std::string(it->second.short_name_)),
      to_fbs_string(fbb, std::string(it->second.short_name_)),
      it->second.stop_point_.lat_, it->second.stop_point_.lon_, 0, NULL,
      s_d_s.timezone_,
      to_fbs_string(fbb, std::string(it->second.stop_point_.timezone_)));

  direction =
      CreateDirection(fbb, station, to_fbs_string(fbb, s_d_s.direction_));
  section = CreateSection(
      fbb, s_d_s.category_, s_d_s.provider_, NULL,
      to_fbs_string(fbb, std::string(begin(s_d_s.l_m_)->second.id_)),
      fbb.CreateVector(
          utl::to_vec(begin(s_d_s.a_v_), end(s_d_s.a_v_),
                      [&](fbs64::Offset<Attribute> const& a) { return a; })),
      direction);
  track = CreateTrack(fbb, to_fbs_string(fbb, s_d_s.traffic_days), NULL);
}

}  // namespace motis::loader::netex
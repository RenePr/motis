#pragma once

#include "motis/loader/parser.h"

namespace motis::loader::netex {

struct netex_parser : public format_parser {
  ~netex_parser() = default;
  bool applicable(boost::filesystem::path const&) override;
  std::vector<std::string> missing_files(
      boost::filesystem::path const&) const override;
  void parse(boost::filesystem::path const&,
             flatbuffers64::FlatBufferBuilder&) override;
};

}  // namespace motis::loader::netex
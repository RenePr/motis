#include "motis/loader/netex/netex_parser.h"

#include "motis/loader/netex/station_netex_parser.h"

#include <iostream>

#include "boost/filesystem.hpp"

#include "pugixml.hpp"

#include "utl/verify.h"
#include "utl/get_or_create.h"

#include "motis/core/common/logging.h"
#include "motis/core/common/zip_reader.h"

#include "motis/loader/util.h"

#include "motis/schedule-format/Schedule_generated.h"

namespace fbs64 = flatbuffers64;
namespace fs = boost::filesystem;
namespace xml = pugi;
using namespace motis::logging;

namespace motis::loader::netex {

bool netex_parser::applicable(fs::path const& p) {
  return fs::is_regular_file(p) && p.extension().generic_string() == ".zip";
}

std::vector<std::string> netex_parser::missing_files(fs::path const&) const {
  return {};
}

void netex_parser::parse(fs::path const& p,
                         flatbuffers64::FlatBufferBuilder& fbb) {
  utl::verify(fs::is_regular_file(p), "{} is not a zip file", p);
  auto const output_services = std::vector<fbs64::Offset<Service>>{};
  auto const fbs_stations = std::map<std::string, fbs64::Offset<Station>>{};
  auto const fbs_routes = std::map<std::string, fbs64::Offset<Route>>{};
  auto const interval = Interval{0, 0};
  auto const footpaths = std::vector<fbs64::Offset<Footpath>>{};
  auto const rule_services = std::vector<fbs64::Offset<RuleService>>{};
  auto const meta_stations = std::vector<fbs64::Offset<MetaStation>>{};
  auto const dataset_name = "test";
  auto const hash = 123;

  auto const z = zip_reader{p.generic_string().c_str()};
  for (auto file = z.read(); file.has_value(); file = z.read()) {
    std::cout << z.current_file_name() << "\n";
    std::cout << "     "
              << file->substr(0, std::min(file->size(), size_t{100U})) << "\n";

    try {
      xml::xml_document d;
      auto r = d.load_buffer(reinterpret_cast<void const*>(file->data()),
                             file->size());
      utl::verify(r, "netex parser: invalid xml in {}", z.current_file_name());
      xml::xpath_node_set stop_node = d.select_nodes("//StopPlace");
      //std::cout << "Here? size: " << stop_node.size() <<  " First_node: " << stop_node.first().node().text() << std::endl;
      std::map<std::string, stop> stops =  parse_xml_stops(stop_node);
      /*for (auto const& stop : d.select_nodes("//StopPlace")) {
        //std::cout << "     " << stop.node().child("Name").child_value() << "\n";
        //Stoplace id
        std::cout << "     " << stop.node().attribute("id").value() << "\n";
        auto get_or_create_stop = [&](stop const* s) {
          return utl::get_or_create(fbs_stations, s, [&]() {
            return CreateStation(
                fbb, fbb.CreateString(s->id_), fbb.CreateString(s->name_);
                //s->coord_.lat_, s->coord_.lng_, 2,
                //fbb.CreateVector(std::vector<Offset<String>>()), 0,
                //s->timezone_.empty() ? 0 : fbb.CreateString(s->timezone_));
          });
        };*/

        //utl::get_or_create();
     // }
    } catch (std::exception const& e) {
      LOG(error) << "unable to parse message: " << e.what();
    } catch (...) {
      LOG(error) << "unable to parse message";
    }
  }
  fbb.Finish(CreateSchedule(
      fbb, fbb.CreateVector(output_services),
      fbb.CreateVector(values(fbs_stations)),
      fbb.CreateVector(values(fbs_routes)), &interval,
      fbb.CreateVector(footpaths), fbb.CreateVector(rule_services),
      fbb.CreateVector(meta_stations), fbb.CreateString(dataset_name), hash));
}

}  // namespace motis::loader::netex
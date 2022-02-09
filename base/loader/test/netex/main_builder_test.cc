#include "gtest/gtest.h"

#include <functional>

#include "utl/verify.h"

#include "boost/filesystem.hpp"

#include "pugixml.hpp"

#include "motis/loader/netex/builder/main_builder.h"
#include "motis/loader/netex/builder/helper_builder.h"
#include "motis/loader/netex/service_journey/service_journey.h"

#include "motis/schedule-format/Schedule_generated.h"

namespace fbs64 = flatbuffers64;
namespace xml = pugi;
namespace fs = boost::filesystem;
using namespace motis::loader::netex;

TEST(main_builder_test, main_builder_empty) {

  try {
    const char* file =
        "base/loader/test_resources/netex_schedules/"
        "NX-PI-01_DE_NAP_LINE_123-ERLBUS-371_20211029_line.xml";
    xml::xml_document d;
    auto fbb =  fbs64::FlatBufferBuilder{};
    auto r = d.load_file(file);
    auto sjp_m = std::map<std::string, service_journey_pattern>{};
    auto sjp = service_journey_pattern{};
    sjp_m.try_emplace(std::string(""), sjp);
    auto sj_m = std::map<std::string, service_journey>{};
    auto sj = service_journey{};
    sj_m.try_emplace(std::string(""), sj);
    auto b = build{};
    b.sjp_m_ = sjp_m;
    b.sj_m_ = sj_m;
    auto sjpp = std::vector<service_journey_parse>{};
    build_fbs(b, sjpp, fbb);
    ASSERT_TRUE(1 ==
                    sjpp.size());


  } catch (std::exception& e) {
  }
}
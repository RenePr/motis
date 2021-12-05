#include "gtest/gtest.h"

#include <functional>

#include "utl/verify.h"

#include "motis/loader/netex/line.h"
#include "motis/loader/netex/netex_parser.h"
#include "motis/loader/netex/line_parse.h"

#include "motis/schedule-format/Schedule_generated.h"

namespace fbs64 = flatbuffers64;

TEST(line_parse, test1) {
  //TODO how can I start this test, is this even right?
  //Users/reneprinz/Documents/motis
  std::string file = "//base/loader/test_resources/netex_schedules/NX-PI-01_DE_NAP_LINE_123-ERLBUS-371_20211029_line.xml";
  try {
    xml::xml_document d;
    auto r = d.load_buffer(reinterpret_cast<void const*>(file.data()),
                           file.size());

    utl::verify(r, "netex parser: invalid xml in {}", file);

    for(auto const& service_journey : d.select_nodes("//ServiceJourneyPattern")) {
      std::cout << "Here?" << std::endl;
      std::vector<motis::loader::netex::line> test =  motis::loader::netex::parse_line(service_journey, d);
      std::string_view test3 {"371"};
      std::cout << "1" << std::endl;
      EXPECT_EQ(test3, test[0].name_ );
    }
  }
  catch(std::exception const& e) {

  }
  //Nur als test erstmal
  std::string_view test4 {"1"};
  std::string_view test5 {"1"};
  EXPECT_EQ(test4, test5);
  //ASSERT_TRUE("1" == "!");
}
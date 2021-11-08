#include "gtest/gtest.h"

#include "motis/loader/netex/station_netex_parser.h"
#include "motis/loader/netex/netex_parser.h"

#include "motis/schedule-format/Schedule_generated.h"

namespace fbs64 = flatbuffers64;

TEST(station_netex_parser, test) {
  //TODO how can I start this test, is this even right?
  fbs64 fbs = new fbs64();
  netex_parser("../motis/base/loader/test_resources/netex_schedules/NX-PI-01_DE_NAP_LINE_123-ERLBUS-371_20211029.xml",
               fbs);

  //ASSERT_EQUAL();
}
#include "Arduino.h"
#include "common_functions.h"
#include "test_framework.h"

int main() {
  CASE("IntFormat zero-pads below 10");
  CHECK_EQ_STR(IntFormat(0).c_str(), "00");
  CHECK_EQ_STR(IntFormat(5).c_str(), "05");
  CHECK_EQ_STR(IntFormat(9).c_str(), "09");
  CHECK_EQ_STR(IntFormat(10).c_str(), "10");
  CHECK_EQ_STR(IntFormat(59).c_str(), "59");

  CASE("GetRSSIasQuality maps RSSI to 0-100%");
  CHECK(GetRSSIasQuality(-100) == 0);
  CHECK(GetRSSIasQuality(-110) == 0);
  CHECK(GetRSSIasQuality(-50) == 100);
  CHECK(GetRSSIasQuality(-30) == 100);
  CHECK(GetRSSIasQuality(-75) == 50);
  CHECK(GetRSSIasQuality(-60) == 80);

  CASE("join concatenates with the separator");
  int days[7] = {0, 1, 1, 1, 1, 1, 0};
  CHECK_EQ_STR(join(days, ",", 7).c_str(), "0,1,1,1,1,1,0");
  int one[1] = {4};
  CHECK_EQ_STR(join(one, ",", 1).c_str(), "4");
  // regression: join once ignored its separator argument and hardcoded ","
  int pair[2] = {1, 2};
  CHECK_EQ_STR(join(pair, " - ", 2).c_str(), "1 - 2");

  return SUMMARY();
}

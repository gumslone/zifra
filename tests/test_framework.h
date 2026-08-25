#pragma once
// Minimal header-only test framework: CASE / CHECK / CHECK_EQ_STR / SUMMARY.
// Each test suite is one .cpp with its own main() returning SUMMARY().
#include <cstdio>
#include <cstring>

inline int g_checks = 0;
inline int g_failures = 0;

#define CASE(name) std::printf("\n-- %s\n", name)

#define CHECK(cond)                                                       \
  do {                                                                    \
    ++g_checks;                                                           \
    if (!(cond)) {                                                        \
      ++g_failures;                                                       \
      std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);         \
    }                                                                     \
  } while (0)

#define CHECK_EQ_STR(a, b)                                                \
  do {                                                                    \
    ++g_checks;                                                           \
    if (std::strcmp((a), (b)) != 0) {                                     \
      ++g_failures;                                                       \
      std::printf("FAIL %s:%d: \"%s\" != \"%s\"\n", __FILE__, __LINE__,   \
                  (a), (b));                                              \
    }                                                                     \
  } while (0)

#define SUMMARY()                                                         \
  (std::printf("\n%d checks, %d failure%s\n", g_checks, g_failures,       \
               g_failures == 1 ? "" : "s"),                               \
   g_failures ? 1 : 0)

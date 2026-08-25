#pragma once
// Host-test shim for <Arduino.h>: just enough of the Arduino API for the
// pure-logic firmware headers to compile on a desktop compiler. The String
// class is backed by std::string and mirrors Arduino String semantics for
// the methods the firmware uses.
#include <cstdint>
#include <cstdlib>
#include <string>

class String {
 public:
  String() {}
  String(const char *s) : v(s ? s : "") {}
  String(const std::string &s) : v(s) {}
  String(char c) : v(1, c) {}
  String(int n) : v(std::to_string(n)) {}
  String(unsigned int n) : v(std::to_string(n)) {}
  String(long n) : v(std::to_string(n)) {}
  String(unsigned long n) : v(std::to_string(n)) {}

  unsigned int length() const { return (unsigned int)v.size(); }
  const char *c_str() const { return v.c_str(); }

  // Arduino substring: `to` is exclusive; out-of-range values are clamped.
  String substring(unsigned int from) const {
    if (from >= v.size()) return String();
    return String(v.substr(from));
  }
  String substring(unsigned int from, unsigned int to) const {
    if (from > to) { unsigned int t = from; from = to; to = t; }
    if (from >= v.size()) return String();
    if (to > v.size()) to = (unsigned int)v.size();
    return String(v.substr(from, to - from));
  }

  long toInt() const { return std::atol(v.c_str()); }

  bool startsWith(const String &prefix) const {
    return v.rfind(prefix.v, 0) == 0;
  }
  int indexOf(char c, unsigned int from = 0) const {
    const auto pos = v.find(c, from);
    return pos == std::string::npos ? -1 : (int)pos;
  }
  int indexOf(const String &s, unsigned int from = 0) const {
    const auto pos = v.find(s.v, from);
    return pos == std::string::npos ? -1 : (int)pos;
  }
  void reserve(unsigned int) {}

  String &operator+=(const String &o) { v += o.v; return *this; }
  String &operator+=(const char *o) { v += (o ? o : ""); return *this; }
  String &operator+=(char c) { v += c; return *this; }
  friend String operator+(String a, const String &b) { a += b; return a; }
  friend String operator+(String a, const char *b) { a += b; return a; }
  friend String operator+(const char *a, const String &b) {
    return String(a) + b;
  }
  bool operator==(const String &o) const { return v == o.v; }
  bool operator==(const char *o) const { return v == (o ? o : ""); }
  bool operator!=(const String &o) const { return v != o.v; }
  bool operator!=(const char *o) const { return v != (o ? o : ""); }

 private:
  std::string v;
};

// Test-controllable clock: set g_millis in a test, millis() returns it.
inline unsigned long g_millis = 0;
inline unsigned long millis() { return g_millis; }
inline void delay(unsigned long) {}
inline void yield() {}

#define HEX 16
#define DEC 10

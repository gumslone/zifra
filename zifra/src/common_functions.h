#pragma once
#ifndef ZIFRA_COMMON_FUNCTIONS_H
#define ZIFRA_COMMON_FUNCTIONS_H

// Small shared helpers. No hardware access — host-testable
// (tests/test_common_functions.cpp).

// 7 -> "07", 12 -> "12"
inline String IntFormat(int value) {
  if (value < 10) {
    return "0" + String(value);
  }
  return String(value);
}

// RSSI (dBm) -> link quality 0-100%
inline int GetRSSIasQuality(int rssi) {
  if (rssi <= -100) {
    return 0;
  }
  if (rssi >= -50) {
    return 100;
  }
  return 2 * (rssi + 100);
}

// Escapes text for use inside a JSON string literal: quotes, backslashes
// and control characters can't appear raw there.
inline String jsonEscape(const String &in) {
  String out;
  out.reserve(in.length() + 8);
  const char *s = in.c_str();
  for (unsigned int i = 0; i < in.length(); i++) {
    const char c = s[i];
    switch (c) {
      case '"': out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default:
        out += ((unsigned char)c < 0x20) ? ' ' : c;
    }
  }
  return out;
}

// {1,0,1} -> "1,0,1"
inline String join(const int *arr, const String &separator, int len) {
  String out = "";
  for (int i = 0; i < len; i++) {
    if (i > 0) {
      out += separator;
    }
    out += String(arr[i]);
  }
  return out;
}

#endif

#pragma once

/// <summary>
/// Adds a leading 0 to a number if it is smaller than 10
/// </summary>
String IntFormat(int _int) {
  if (_int < 10) {
    return "0" + String(_int);
  }

  return String(_int);
}

/// <summary>
/// Convert RSSI to percentage quality
/// </summary>
int GetRSSIasQuality(int rssi) {
  int quality = 0;

  if (rssi <= -100) {
    quality = 0;
  } else if (rssi >= -50) {
    quality = 100;
  } else {
    quality = 2 * (rssi + 100);
  }
  return quality;
}

/// <summary>
/// Joins an int array into a separated string, e.g. {1,0,1} -> "1,0,1"
/// </summary>
String join(int *arr, String separator, int len) {
  String out = "";
  for (int i = 0; i < len; i++) {
    if (i > 0) {
      out += separator;
    }
    out += String(arr[i]);
  }
  return out;
}

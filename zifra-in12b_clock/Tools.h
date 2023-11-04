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
/// Returns 1 hour in daylight saving time and outside 0
/// </summary>
int DSToffset(time_t _date, int _clockTimeZone) {
  bool _summerTime;
  /*
          if (month(_date) < 3 || month(_date) > 10)
          {
                  _summerTime = false; // no summertime in Jan, Feb, Nov, Dec
          }
          else if (month(_date) > 3 && month(_date) < 10) {
                  _summerTime = true; // Summertime in Apr, May, Jun, Jul, Aug,
     Sep
          }
          else if (month(_date) == 3 && (hour(_date) + 24 * day(_date)) >= (1 +
     String(_clockTimeZone).toInt() + 24 * (31 - (5 * year(_date) / 4 + 4) % 7))
     || month(_date) == 10 && (hour(_date) + 24 * day(_date)) < (1 +
     String(_clockTimeZone).toInt() + 24 * (31 - (5 * year(_date) / 4 + 1) %
     7)))
          {
                  _summerTime = true;
          }
          else
          {
                  _summerTime = false;
          }*/
  return _summerTime ? 1 : 0;
}

/// <summary>
/// Checks if it is a valid IP address
/// </summary>
boolean isIP(String _str) {
  for (char i = 0; i < _str.length(); i++) {
    if (!(isDigit(_str.charAt(i)) || _str.charAt(i) == '.')) {
      return false;
    }
  }
  return true;
}

/// <summary>
/// Convert UTF8 byte to ASCII
/// </summary>
byte Utf8ToAscii(byte _ascii) {
  static byte _thisByte;

  if (_ascii < 128) {
    _thisByte = 0;
    return (_ascii);
  }

  byte last = _thisByte;
  _thisByte = _ascii;
  byte _result = 0;

  switch (last) {
  case 0xC2:
    _result = _ascii - 34;
    break;
  case 0xC3:
    _result = (_ascii | 0xC0) - 34;
    break;
  case 0x82:
    if (_ascii == 0xAC) {
      _result = (0xEA);
    }
    break;
  }
  return _result;
}

/// <summary>
/// Convert UTF8 Chars to ASCII
/// </summary>
String Utf8ToAscii(String _str) {
  String _result = "";
  char _thisChar;

  for (int i = 0; i < _str.length(); i++) {
    _thisChar = Utf8ToAscii(_str.charAt(i));

    if (_thisChar != 0) {
      _result += _thisChar;
    }
  }
  return _result;
}

/// <summary>
/// Returns the chip id
/// </summary>
String GetChipID() { return String(ESP.getChipId()); }

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
// find in string
uint8_t strContains(const char *string, char *toFind) {
  uint8_t slen = strlen(string);
  uint8_t tFlen = strlen(toFind);
  uint8_t found = 0;

  if (slen >= tFlen) {
    for (uint8_t s = 0, t = 0; s < slen; s++) {
      do {

        if (string[s] == toFind[t]) {
          if (++found == tFlen)
            return 1;
          s++;
          t++;
        } else {
          s -= found;
          found = 0;
          t = 0;
        }

      } while (found);
    }
    return 0;
  } else
    return -1;
}

String join(int *arr, String separator, int len) {
  int i;
  String out = "";
  for (i = 0; i < len; i++) {
    if (i > 0) /* You do need this separate check, unfortunately. */
      out = out + ",";
    out = out + String(arr[i]);
  }
  return out;
}

bool leapYear(int yyyy)
{
  bool leap = false;
  // Leap Year Calculation
  if ((fmod(yyyy, 4) == 0 && fmod(yyyy, 100) != 0) || (fmod(yyyy, 400) == 0)) {
    leap = 1;
  } else {
    leap = 0;
  }
  return leap;
}
//================================================================================
// Begin calcdayofweek( D, M, Y)
//================================================================================
int dayOfWeek(int yyyy, int m, int d) {

  // int Year, Month, Day, Hour, Minute, Second ;
  uint8_t DoW;        // Day of the week value (0-6)
  uint8_t yy;         // Last 2 digits of the year (ie 2016 would be 16)
  uint8_t c;          // Century (ie 2016 would be 20)
  uint8_t mTable;     // Month value based on calculation table
  bool leap = leapYear(yyyy);
  int SummedDate; // Add values combined in prep for Mod7 calc
  int cTable;     // Century value based on calculation table
  // Limit results to year 1900-2299 (to save memory)
  while (yyyy > 2299) {
    yyyy = yyyy - 400;
  }
  while (yyyy < 1900) {
    yyyy = yyyy + 400;
  }

  // Calculating century
  c = yyyy / 100;

  // Calculating two digit year
  yy = fmod(yyyy, 100);

  // Century value based on Table
  if (c == 19) {
    cTable = 1;
  }
  if (c == 20) {
    cTable = 0;
  }
  if (c == 21) {
    cTable = 5;
  }
  if (c == 22) {
    cTable = 3;
  }

  // Jan and Feb calculations affected by leap years
  if (m == 1) {
    if (leap == 1) {
      mTable = 6;
    } else {
      mTable = 0;
    }
  }
  if (m == 2) {
    if (leap == 1) {
      mTable = 2;
    } else {
      mTable = 3;
    }
  }
  // Other months not affected and have set values
  if (m == 10) {
    mTable = 0;
  }
  if (m == 8) {
    mTable = 2;
  }
  if (m == 3 || m == 11) {
    mTable = 3;
  }
  if (m == 4 || m == 7) {
    mTable = 6;
  }
  if (m == 5) {
    mTable = 1;
  }
  if (m == 6) {
    mTable = 4;
  }
  if (m == 9 || m == 12) {
    mTable = 5;
  }

  // Enter the data into the formula
  SummedDate = d + mTable + yy + (yy / 4) + cTable;

  // Find remainder
  DoW = fmod(SummedDate, 7);
  return DoW;
}
//================================================================================
// End calcdayofweek( D, M, Y)
//================================================================================

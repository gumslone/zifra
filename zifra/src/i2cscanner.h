#pragma once
#ifndef ZIFRA_I2CSCANNER_H
#define ZIFRA_I2CSCANNER_H

// One-shot I2C bus scan at boot; the found addresses decide whether the
// optional DS3231 RTC (0x68) is present.
namespace i2cScanner {

String addresses;

void scan() {
  D_println(F("i2c: scanning..."));
  for (uint8_t address = 1; address < 127; address++) {
    // a device acknowledges its address with a clean endTransmission
    Wire.beginTransmission(address);
    const uint8_t error = Wire.endTransmission();
    if (error == 0) {
      addresses += "0x";
      if (address < 16) {
        addresses += "0";
      }
      addresses += String(address, HEX) + ",";
      D_println("i2c: device at 0x" + String(address, HEX));
    }
  }
  D_println("i2c: found " + (addresses.length() ? addresses : String("none")));
}

bool addressExists(const char *addr) {
  return addresses.indexOf(addr) >= 0;
}

} // namespace i2cScanner

#endif

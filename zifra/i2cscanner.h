#pragma once

namespace i2cScanner {

String addresses;

// scan i2c devices
void scan() {
  D_println("Scanning...");
  int nDevices = 0;
  for (uint8_t address = 1; address < 127; address++) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    const uint8_t error = Wire.endTransmission();
    if (error == 0) {
      D_println("I2C device found at address 0x");
      addresses += "0x";
      if (address < 16) {
        D_print("0");
        addresses += "0";
      }

      D_println(address, HEX);
      addresses += String(address, HEX) + ",";
      nDevices++;
    } else if (error == 4) {
      D_print("Unknown error at address 0x");
      if (address < 16)
        D_print("0");
      D_println(address, HEX);
    }
  }
  if (nDevices == 0)
    D_println("No I2C devices found\n");
  else
    D_println("I2C scan is finished\n");
}

bool addressExists(const char *addr) {
  return addresses.indexOf(addr) >= 0;
}

} // namespace i2cScanner

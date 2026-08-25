#pragma once

#ifndef _pf595_
#define _pf595_

class PF595{
public:

void startUp()
{
  allOff();
  // iterate over digits:
  for (uint8_t digit = 0; digit < 11; digit++) {
    allOff();
    delay(10);
    showNumber(digit);
    delay(100);
  }
  allOff();
}

void allOff() {
  // turn all off
  m_a = B00000000;
  m_b = B00000000;
  write(word(m_a, m_b));
}

void on() {
  // turn on
  write(word(m_a, m_b));
}

void showNumber(uint8_t num, bool dot = false) {
  if (dot) {
    bitSet(m_a, 2); // dot pin
  }
  if (num < 8) {
    bitSet(m_b, num); // 0-7 Pins
  } else {
    num -= 8;
    bitSet(m_a, num); // 8-9 + dot Pins
  }
  on();
}

private:
int m_address{0x20};
uint8_t m_a{B00000000};
uint8_t m_b{B00000000};

//  Function for writing two Bytes to the I2C expander device
void write(uint16_t data) {
  Wire.beginTransmission(m_address);
  Wire.write(lowByte(data));
  Wire.write(highByte(data));
  Wire.endTransmission();
}

uint16_t read() {
  Wire.beginTransmission(m_address);
  Wire.endTransmission();
  Wire.requestFrom(m_address, 2);
  if (Wire.available()) {
    uint8_t lo = Wire.read();
    uint8_t hi = Wire.read();
    return (word(hi, lo)); // joing bytes
  }
  return 0;
}
};

#endif

#pragma once
#ifndef ZIFRA_PF595_H
#define ZIFRA_PF595_H

// The I2C port expander driving the IN-12B tube: one output pin per
// cathode digit 0-9 plus the decimal dot.
class PF595 {
  public:
    // Cycles once through every digit as a power-on test pattern.
    void startUp() {
      allOff();
      for (uint8_t digit = 0; digit < 11; digit++) {
        allOff();
        delay(10);
        showNumber(digit);
        delay(100);
      }
      allOff();
    }

    void allOff() {
      m_a = 0;
      m_b = 0;
      write(word(m_a, m_b));
    }

    void showNumber(uint8_t num, bool dot = false) {
      if (dot) {
        bitSet(m_a, 2); // dot pin
      }
      if (num < 8) {
        bitSet(m_b, num); // digits 0-7
      } else {
        bitSet(m_a, num - 8); // digits 8-9 share the byte with the dot
      }
      write(word(m_a, m_b));
    }

  private:
    static constexpr int ADDRESS = 0x20;
    uint8_t m_a{0};
    uint8_t m_b{0};

    // Writes both output bytes to the expander
    void write(uint16_t data) {
      Wire.beginTransmission(ADDRESS);
      Wire.write(lowByte(data));
      Wire.write(highByte(data));
      Wire.endTransmission();
    }
};

#endif

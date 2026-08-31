#pragma once
#ifndef ZIFRA_CLOCK_TIMER_H
#define ZIFRA_CLOCK_TIMER_H

// Sequences the single tube through the time's digits: each slot is either
// a digit to show or a dark pause, timed by DIGIT_TIMES_MS. Pure millis()
// logic — host-testable (tests/test_clock_timer.cpp).
class ClockTimer {
  public:
    bool showDigit(uint8_t digit) {
      return getCurrentDigit() == digit && !expired();
    }

    bool expired() {
      const uint16_t interval = digitTimesMs()[getCurrentDigit()];
      if (millis() - m_lastTime >= interval) {
        nextDigit();
        updateLast();
        return true;
      }
      return false;
    }

    void reset() {
      m_currentDigit = 0;
      updateLast();
    }

    void updateLast() {
      m_lastTime = millis();
    }

    uint8_t getCurrentDigit() {
      if (m_currentDigit >= SLOT_COUNT) {
        m_currentDigit = 0;
      }
      return m_currentDigit;
    }

    void nextDigit() {
      m_currentDigit++;
    }

  private:
    static constexpr uint8_t SLOT_COUNT = 8;

    static const uint16_t *digitTimesMs() {
      static const uint16_t times[SLOT_COUNT] = {
        800,  // hours, first digit
        200,  // pause between two digits
        800,  // hours, second digit
        700,  // pause between hours and minutes
        800,  // minutes, first digit
        200,  // pause
        800,  // minutes, second digit
        1600  // all off before the next round
      };
      return times;
    }

    uint8_t m_currentDigit{0};
    uint32_t m_lastTime{0};
};

#endif

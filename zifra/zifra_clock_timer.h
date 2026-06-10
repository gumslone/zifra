#pragma once
class ClockTimer{
  public:
  bool showDigit(uint8_t digit)
  {
    return getCurrentDigit() == digit && !expired();
  }
  bool expired()
  {
    uint16_t interval = m_digitTimesInMillis[getCurrentDigit()];
    if (millis() - m_lastTime >= interval) {
      nextDigit();
      updateLast();
      return true;
    } 
    return false;
  }
  void reset()
  {
    m_currentDigit = 0;
    updateLast();
  }
  void updateLast()
  {
    m_lastTime = millis();
  }
  uint8_t getCurrentDigit()
  {
    if(m_currentDigit >= m_digitTimesLength)
    {
      m_currentDigit = 0;
    }
    return m_currentDigit;
  }
  void nextDigit()
  {
    m_currentDigit++;
  }
  private:
  uint8_t m_currentDigit{0};
  uint16_t m_digitTimesInMillis[8] = {
  800, // display digit time
  200, // pause between two digits
  800, 
  700, // pause between hours and minutes
  800, 
  200, 
  800,
  1600 // turn all off
  };
  uint8_t m_digitTimesLength = 8;
  uint32_t m_lastTime{0};
};

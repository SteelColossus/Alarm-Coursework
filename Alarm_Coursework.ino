#include <Adafruit_RGBLCDShield.h>
#include <EEPROM.h>

class Time
{
  private:
    unsigned int hours;
    unsigned int minutes;
    unsigned int seconds;

  public:
    enum Part { HOUR, MINUTE, SECOND };

    Time()
    {
      Time(0, 0, 0);
    }
  
    Time(unsigned int h, unsigned int m, unsigned int s)
    {
      setTime(h, m, s);
    }

    unsigned int getTimePart(Part part)
    {
      switch (part)
      {
        case HOUR:
          return hours;
        case MINUTE:
          return minutes;
        case SECOND:
          return seconds;
      }
    }
  
    void setTimePart(Part part, unsigned int v)
    {
      switch (part)
      {
        case HOUR:
          hours = v % 24;
          break;
        case MINUTE:
          minutes = v % 60;
          break;
        case SECOND:
          seconds = v % 60;
          break;
      }
    }
  
    void setTime(unsigned int h, unsigned int m, unsigned int s)
    {
      m += (s / 60);
      h += (m / 60);

      setTimePart(HOUR, h);
      setTimePart(MINUTE, m);
      setTimePart(SECOND, s);
    }
  
    void setTime(unsigned long ms)
    {
      setTime(0, 0, (ms / 1000));
    }

    void addTimePart(Part part, unsigned int v)
    {
      switch (part)
      {
        case HOUR:
          setTimePart(part, hours + v);
          break;
        case MINUTE:
          setTimePart(part, minutes + v);
          break;
        case SECOND:
          setTimePart(part, seconds + v);
          break;
      }
    }

    void subtractTimePart(Part part, unsigned int v)
    {
      int newTime;
      
      switch (part)
      {
        case HOUR:
          newTime = (int)hours - (v % 24);
          if (newTime < 0) newTime += 24;
          setTimePart(part, newTime);
          break;
        case MINUTE:
          newTime = (int)minutes - (v % 60);
          if (newTime < 0) newTime += 60;
          setTimePart(part, newTime);
          break;
        case SECOND:
          newTime = (int)seconds - (v % 60);
          if (newTime < 0) newTime += 60;
          setTimePart(part, newTime);
          break;
      }
    }

    void addTime(unsigned int h, unsigned int m, unsigned int s)
    {
      setTime(hours + h, minutes + m, seconds + s);
    }

    void addTime(unsigned long ms)
    {
      addTime(0, 0, (ms / 1000));
    }

    String getReadableShort()
    {
      return ((hours < 10) ? "0" : "") + String(hours) + ":" + ((minutes < 10) ? "0" : "") + String(minutes);
    }

    String getReadable()
    {
      return getReadableShort() + ":" + ((seconds < 10) ? "0" : "") + String(seconds);
    }

    bool areApproxEqual(const Time& other)
    {
      return hours == other.hours && minutes == other.minutes;
    }

    bool areEqual(const Time& other)
    {
      return areApproxEqual(other) && seconds == other.seconds;
    }

    bool operator== (const Time& other)
    {
      return areEqual(other);
    }
};

Adafruit_RGBLCDShield lcd;

const int screenWidth = 16;
const int screenHeight = 2;

// Defining custom characters
uint8_t backslash[8] = {0, 16, 8, 4, 2, 1, 0, 0};
uint8_t bltotrdiag[8] = {0, 1, 2, 4, 8, 16, 15, 0};
uint8_t trtobldiag[8] = {0, 30, 1, 2, 4, 8, 16, 0};
uint8_t topline[8] = {0, 31, 0, 0, 0, 0, 0, 0};
uint8_t upperbracket[8] = {16, 16, 8, 7, 0, 0, 0, 0};
uint8_t sixstem[8] = {0, 3, 4, 8, 8, 8, 8, 8};

enum class CustomChars : uint8_t { BACKSLASH = 0, BLTOTRDIAG = 1, TRTOBLDIAG = 2, TOPLINE = 3, UPPERBRACKET = 4, SIXSTEM = 5 };

uint8_t digits[][4] = { {'/', (uint8_t)CustomChars::BACKSLASH, (uint8_t)CustomChars::BACKSLASH, '/'},
                        {'/', '|', ' ', '|'},
                        {'/', (uint8_t)CustomChars::TRTOBLDIAG, (uint8_t)CustomChars::BLTOTRDIAG, '_'},
                        {(uint8_t)CustomChars::TOPLINE, (uint8_t)CustomChars::TRTOBLDIAG, 164, ')'},
                        {'/', '|', (uint8_t)CustomChars::UPPERBRACKET, '+'},
                        {(uint8_t)CustomChars::BLTOTRDIAG, (uint8_t)CustomChars::TOPLINE, 164, ')'},
                        {(uint8_t)CustomChars::SIXSTEM, (uint8_t)CustomChars::TOPLINE, '(', ')'},
                        {(uint8_t)CustomChars::TOPLINE, (uint8_t)CustomChars::TRTOBLDIAG, '/', ' '},
                        {'(', ')', '(', ')'},
                        {'(', ')', ' ', '|'}};

uint8_t colon[2] = {165, 165};
uint8_t space[2] = {' ', ' '};
 
int cursorX = 0;
int cursorY = 0;

String currentText = "";

Time screenTime;
Time alarmTime;

unsigned long currentTime = 0;
unsigned long previousClockTime = 0;
unsigned long previousBlinkTime = 0;
unsigned int previousAlarmBlinkTime = 0;
unsigned int clockTimer = 0;
unsigned int blinkTimer = 0;
unsigned int alarmBlinkTimer = 0;
unsigned int lostClockTime = 0;

bool alarmActive = false;
const int defaultBacklightColor = 0x7;
unsigned int backlightColor;

const unsigned int clockWaitTime = 1000;
const unsigned int blinkTime = 500;
const unsigned int alarmBlinkTime = 500;
const unsigned int shortPressTime = 400;
const unsigned int shortHoldTime = 200;
const unsigned int longHoldTime = 1500;

enum class Mode { CLOCK, CLOCKSET, ALARMSET };

Mode currentMode;
unsigned long timeChangedMode = 0;
Time::Part selectedEditPart;
bool blinking = false;

uint8_t buttons;
unsigned long upButtonStartTimePressed = 0;
unsigned long downButtonStartTimePressed = 0;
unsigned long leftButtonStartTimePressed = 0;
unsigned long rightButtonStartTimePressed = 0;
unsigned long selectButtonStartTimePressed = 0;

void setCursorPos(int x, int y)
{
  cursorX = x;
  cursorY = y;
  lcd.setCursor(cursorX, cursorY);
}

int getBigCharWidth(char c)
{
  if (c >= '0' && c <= '9')
  {
    return 2;
  }
  else
  {
    return 1;
  }
}

void writeBigChar(uint8_t * c, int width=2)
{
  setCursorPos(cursorX, 0);
  
  for (int i = 0; i < width*2; i++)
  {    
    lcd.write((char)c[i]);
    
    if (i == width - 1)
    {
      setCursorPos(cursorX - (width - 1), 1);
    }
    else
    {
      cursorX++;
    }
  }
}

void printChar(char c)
{
  if (c >= '0' && c <= '9')
  {
    writeBigChar(digits[(int)c - (int)'0']);
  }
  else if (c == ':')
  {
    writeBigChar(colon, 1);
  }
  else
  {
    writeBigChar(space, 1);
  }
}

void printToScreen(String str, int x=cursorX)
{
  cursorX = x;
  
  for (int i = 0; i < str.length(); i++)
  {
    printChar(str.charAt(i));
  }
  
  currentText = str;
}

void updateScreen(String str, int x=cursorX)
{  
  if (getWidthOnScreen(str) != getWidthOnScreen(currentText))
  {
    printToScreen(str, x);
  }
  else if (str != currentText)
  {    
    cursorX = x;

    int i = 0;
    int j = 0;
    int overlap = 0;
    
    while (i < str.length())
    {
      char c1 = str.charAt(i);
      char c2 = currentText.charAt(j);

      int c1w = getBigCharWidth(c1);
      int c2w = getBigCharWidth(c2);
      
      if (c1 != c2)
      {
        printChar(c1);
      }
      else
      {        
        cursorX += c1w;
      }

      if (c2w > c1w)
      {
        i += 1;
        printChar(str.charAt(i));
      }

      i += 1;
      j += (c1w > c2w ? (c1w - c2w) + 1 : 1);
    }
  }
  
  currentText = str;
}

int getWidthOnScreen(String str)
{
  int len = 0;

  for (int i = 0; i < str.length(); i++)
  {
    char c = str.charAt(i);

    len += getBigCharWidth(c);
  }

  return len;
}

int getCentrePos(String str)
{  
  return (screenWidth / 2) - ((float)getWidthOnScreen(str) / 2);
}

void updateScreenTime()
{
  String output = screenTime.getReadable();
  
  if (currentMode == Mode::CLOCKSET)
  {
    if (blinkTimer >= blinkTime) blinking = !blinking;
    
    if (blinking)
    {
      int sp = (int)selectedEditPart;
      output = output.substring(0, sp*3) + "    " + output.substring(sp*3 + 2);
    }
  }
  
  updateScreen(output, getCentrePos(output));
}

void resetBlink(bool b=true)
{
    blinkTimer = 0;
    blinking = b;
    updateScreenTime();
    previousBlinkTime = millis();
}

void updateMode(Mode mode)
{
  Mode previousMode = currentMode;
  currentMode = mode;
  
  switch (mode)
  {
    case Mode::CLOCK:
      updateScreenTime();
      if (previousMode == Mode::CLOCKSET) updateEEPROMClock();
      break;
    case Mode::CLOCKSET:
      resetBlink();
      break;
  }
  
  timeChangedMode = currentTime;
}

void updateEEPROMClock()
{
  EEPROM.write(0, screenTime.getTimePart(Time::HOUR));
  EEPROM.write(1, screenTime.getTimePart(Time::MINUTE));
  
  //Serial.println("EEPROM values " + String(EEPROM.read(0)) + " and " + String(EEPROM.read(1)) + " written.");
}

void setupCustomChars()
{
  lcd.createChar((int)CustomChars::BACKSLASH, backslash);
  lcd.createChar((int)CustomChars::BLTOTRDIAG, bltotrdiag);
  lcd.createChar((int)CustomChars::TRTOBLDIAG, trtobldiag);
  lcd.createChar((int)CustomChars::TOPLINE, topline);
  lcd.createChar((int)CustomChars::UPPERBRACKET, upperbracket);
  lcd.createChar((int)CustomChars::SIXSTEM, sixstem);
}

void setup() {
  Serial.begin(9600);
  lcd.begin(screenWidth, screenHeight);
  setupCustomChars();

  backlightColor = defaultBacklightColor;
  lcd.setBacklight(backlightColor);

  currentMode = Mode::CLOCK;
  selectedEditPart = Time::HOUR;

  int h = EEPROM.read(0);
  int m = EEPROM.read(1);

  screenTime.setTime(h, m, 0);

  int ah = EEPROM.read(2);
  int am = EEPROM.read(3);

  alarmTime.setTime(ah, am, 0);
  
  Serial.println("Alarm time: " + alarmTime.getReadableShort());

  updateScreenTime();

  currentTime = millis();
  previousClockTime = currentTime;
}

void loop() {  
  currentTime = millis();
  clockTimer = currentTime - previousClockTime + lostClockTime;

  if (currentMode == Mode::CLOCKSET) blinkTimer = currentTime - previousBlinkTime;

  if (alarmActive) alarmBlinkTimer = currentTime - previousAlarmBlinkTime;

  if (clockTimer >= clockWaitTime)
  {
    int prevMins = screenTime.getTimePart(Time::MINUTE);
    previousClockTime = currentTime;
    screenTime.addTime(1000);

    if (screenTime.getTimePart(Time::MINUTE) != prevMins) updateEEPROMClock();

    if (screenTime == alarmTime)
    {
      Serial.println("Alarm started!");
      alarmActive = true;
    }

    updateScreenTime();

    clockTimer -= clockWaitTime;
    lostClockTime = clockTimer;
  }

  if (blinkTimer >= blinkTime && currentMode == Mode::CLOCKSET)
  {
    previousBlinkTime = currentTime;
    
    if (clockTimer > lostClockTime) updateScreenTime();
    
    blinkTimer = 0;
  }

  if (alarmBlinkTimer >= alarmBlinkTime && alarmActive)
  {
    previousAlarmBlinkTime = currentTime;
    
    if (backlightColor == defaultBacklightColor)
    {
      backlightColor = 0x4;
    }
    else
    {
      backlightColor = defaultBacklightColor;
    }

    lcd.setBacklight(backlightColor);
  }

  buttons = lcd.readButtons();

  if (buttons & BUTTON_UP)
  {
    if ((currentTime - upButtonStartTimePressed) >= shortPressTime && currentMode == Mode::CLOCKSET)
    {
      if (selectedEditPart == Time::SECOND)
      {
        screenTime.setTimePart(Time::SECOND, 0);
      }
      else
      {
        screenTime.addTimePart(selectedEditPart, 1);
      }
      
      resetBlink(false);

      upButtonStartTimePressed = currentTime;
    }
  }
  else
  {
    upButtonStartTimePressed = currentTime;
  }
  
  if (buttons & BUTTON_DOWN)
  {
    if ((currentTime - downButtonStartTimePressed) >= shortPressTime && currentMode == Mode::CLOCKSET)
    {
      if (selectedEditPart == Time::SECOND)
      {
        screenTime.setTimePart(Time::SECOND, 0);
      }
      else
      {
        screenTime.subtractTimePart(selectedEditPart, 1);
      }
      
      resetBlink(false);
      
      downButtonStartTimePressed = currentTime;
    }
  }
  else
  {
    downButtonStartTimePressed = currentTime;
  }
  
  if (buttons & BUTTON_LEFT)
  {
    if ((currentTime - leftButtonStartTimePressed) >= shortPressTime && currentMode == Mode::CLOCKSET)
    {
      switch (selectedEditPart)
      {
        case Time::MINUTE:
          selectedEditPart = Time::HOUR;
          resetBlink();
          break;
        case Time::SECOND:
          selectedEditPart = Time::MINUTE;
          resetBlink();
          break;
      }

      leftButtonStartTimePressed = currentTime;
    }
  }
  else
  {
    leftButtonStartTimePressed = currentTime;
  }
  
  if (buttons & BUTTON_RIGHT)
  {
    if ((currentTime - rightButtonStartTimePressed) >= shortPressTime && currentMode == Mode::CLOCKSET)
    {
      switch (selectedEditPart)
      {
        case Time::HOUR:
          selectedEditPart = Time::MINUTE;
          resetBlink();
          break;
        case Time::MINUTE:
          selectedEditPart = Time::SECOND;
          resetBlink();
          break;
      }
      
      rightButtonStartTimePressed = currentTime;
    }
  }
  else
  {
    rightButtonStartTimePressed = currentTime;
  }
  
  if (buttons & BUTTON_SELECT)
  {
    if ((currentTime - selectButtonStartTimePressed) >= longHoldTime && selectButtonStartTimePressed >= timeChangedMode)
    {
      if (alarmActive)
      {
        alarmActive = false;
        Serial.println("Alarm stopped!");
        
        if (backlightColor != defaultBacklightColor)
        {
          backlightColor = defaultBacklightColor;
          lcd.setBacklight(backlightColor);
        }
      }
      
      switch (currentMode)
      {
        case Mode::CLOCK:
          updateMode(Mode::CLOCKSET);
          break;
        case Mode::CLOCKSET:
          updateMode(Mode::CLOCK);
          break;
      }
    }
  }
  else
  {
    selectButtonStartTimePressed = currentTime;
  }
}

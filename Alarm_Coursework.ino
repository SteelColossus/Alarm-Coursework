#include <Adafruit_RGBLCDShield.h>
#include <EEPROM.h>

typedef void (* func)();

// Class for storing a time in hours, minutes and seconds
class Time
{
  private:
    unsigned int hours;
    unsigned int minutes;
    unsigned int seconds;

  public:
    // The different parts that the time is composed of
    enum Part { HOUR, MINUTE, SECOND };

    Time()
    {
      Time(0, 0, 0);
    }
  
    Time(unsigned int h, unsigned int m, unsigned int s)
    {
      setTime(h, m, s);
    }

    // Gets the value of the given time part
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

    // Sets the value of the given time part
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

    // Sets the value of the whole time in hours, minutes and seconds
    void setTime(unsigned int h, unsigned int m, unsigned int s)
    {
      m += (s / 60);
      h += (m / 60);

      setTimePart(HOUR, h);
      setTimePart(MINUTE, m);
      setTimePart(SECOND, s);
    }
  
    // Sets the value of the whole time in milliseconds
    void setTime(unsigned long ms)
    {
      unsigned int h = (ms / 1000) / 3600;
      unsigned int m = ((ms / 1000) / 60) % 60;
      unsigned int s = (ms / 1000) % 60; 
      
      setTime(h, m, s);
    }

    // Adds on an amount of time to a given time part
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

    // Subtracts an amount of time from a given time part
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

    // Adds an amount of time to the whole time in hours, minutes and seconds
    void addTime(unsigned int h, unsigned int m, unsigned int s)
    {
      setTime(hours + h, minutes + m, seconds + s);
    }

    // Adds an amount of time to the whole time in milliseconds
    void addTime(unsigned long ms)
    {
      setTime(getTotalMillis() + ms);
    }

    // Gets the total number of milliseconds from the time
    unsigned long getTotalMillis()
    {
      return (((unsigned long)hours * 3600) + (minutes * 60) + seconds) * 1000;
    }

    // Returns the readable string of the time's hours and minutes
    String getReadableShort()
    {
      return ((hours < 10) ? "0" : "") + String(hours) + ":" + ((minutes < 10) ? "0" : "") + String(minutes);
    }

    // Returns the readable string of the time's hours and minutes and seconds
    String getReadable()
    {
      return getReadableShort() + ":" + ((seconds < 10) ? "0" : "") + String(seconds);
    }

    // Checks if the two times have the same number of hours and minutes
    bool areApproxEqual(const Time& other)
    {
      return hours == other.hours && minutes == other.minutes;
    }

    // Checks if the two times are exactly equal
    bool areEqual(const Time& other)
    {
      return areApproxEqual(other) && seconds == other.seconds;
    }

    bool operator== (const Time& other)
    {
      return areEqual(other);
    }
};

// LCD object
Adafruit_RGBLCDShield lcd;

// Width of the screen in number of characters
const int screenWidth = 16;
// Height of the screen in number of characters
const int screenHeight = 2;

// Defining custom characters as an array of integers. These represent a set of 5x8 pixels in a single character space.
// Each integer represents the binary representation of the pixels on that row. The index of that integer in the array is the same as the row number in the pixel grid.
uint8_t backslash[8] = {0, 16, 8, 4, 2, 1, 0, 0};
uint8_t bltotrdiag[8] = {0, 1, 2, 4, 8, 16, 15, 0};
uint8_t trtobldiag[8] = {0, 30, 1, 2, 4, 8, 16, 0};
uint8_t topline[8] = {0, 31, 0, 0, 0, 0, 0, 0};
uint8_t upperbracket[8] = {16, 16, 8, 7, 0, 0, 0, 0};
uint8_t sixstem[8] = {0, 3, 4, 8, 8, 8, 8, 8};
uint8_t alarm[8] = {0, 31, 14, 21, 23, 17, 14, 0};

// Enum with all the names of the custom characters (same as those above) with their custom character numbers
enum class CustomChars : uint8_t { BACKSLASH = 0, BLTOTRDIAG = 1, TRTOBLDIAG = 2, TOPLINE = 3, UPPERBRACKET = 4, SIXSTEM = 5, ALARM = 6 };

// An array containing the block of 2x2 characters to be displayed for each digit 0-9 in order. Each number is composed of 4 individual characters stored as unsigned integers in ASCII form.
// Some numbers are given by the custom characters, and represent that that custom character should be printed out at that point (e.g. printing backlash which is custom character 0). Others list the ASCII code as they are not commonly used ASCII characters on most computers.
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

// Similar to the above for colon and space, except these characters occupy a 1x2 space.
uint8_t colon[2] = {165, 165};
uint8_t space[2] = {' ', ' '};

// The X position of the LCD cursor
int cursorX = 0;
// The Y position of the LCD cursor
int cursorY = 0;

// The text that is currently being displayed on the screen
String currentText = "";

// The stored time that is displayed on the screen
Time screenTime;
// The set alarm time
Time alarmTime;

// The current amount of time the program has been running for in milliseconds
unsigned long currentTime = 0;
// The current amount of time used as the time displayed on the screen in milliseconds. This is offset from currentTime by the value clockOffset.
unsigned long currentScreenTime = 0;
// The value used to offset the current time to the time displayed on the screen
long clockOffset = 0;

// A timer used to update the screen time around every second
unsigned int clockTimer = 0;
// The last time the clock was 'reset' (i.e. started or got to midnight)
unsigned long clockResetTime = 0;
// A timer used to update whether a time part on the screen is present for the blinking 'animation'
unsigned int blinkTimer = 0;
// The last time the blink status was updated
unsigned long previousBlinkTime = 0;
// A timer used to update what colour the screen backlight should be for the alarm flashing 'animation'
unsigned int alarmFlashTimer = 0;
// The last time the flashing status of the alarm was updated
unsigned long previousAlarmFlashTime = 0;

// Whether a part on the screen is currently blinking (part is not visible) or not blinking (whole time is visible)
bool blinking = false;

// Whether the alarm is currently active
bool alarmOn = false;
// Whether the alarm is currently active
bool alarmActive = false;
// The default colour of the backlight
const int defaultBacklightColor = 0x7;
// The current colour of the backlight
int backlightColor;

// The amount of time it takes for the screen to switch between blinking and not blinking for the blinking 'animation'
const unsigned int blinkTime = 500;
// The amount of time it takes for the screen to switch between different backlight colours for the alarm flashing 'animation'
const unsigned int alarmFlashTime = 500;

// The amount of time required for a short press of a button
const unsigned int shortPressTime = 300;
// The amount of time required for each short press of a button to be registered when the button has been held down for an amount of time
const unsigned int shortHoldTime = 150;
// The amount of time required for a long press of a button
const unsigned int longPressTime = 1000;

// Enum that contains all the modes that the alarm clock can be in
enum class Mode { CLOCK, CLOCKSET, ALARMSET };

// The current mode that the alarm clock is in
Mode currentMode;
// The currently selected part of the time that is being edited
Time::Part selectedEditPart;

// An integer of the current buttons that are being pressed
uint8_t buttons;

// A class for handling button inputs
class ButtonHandler
{
  private:
    // The byte used for checking if the button is pressed down
    uint8_t buttonByte;
    // The last time the button started to be held down
    unsigned long lastTimeHeld;
    // The current internal state of whether the button is being held down
    bool currentButtonState;
    // The last time the handler registered a press
    unsigned long lastTimePressed;
    // The number of presses the handler has registered while the button was continuously held down
    int timesPressedWhileHeld;

    func onShortPress;
    func onLongPress;

  public:
    ButtonHandler(byte b) : buttonByte(b) { }

    void setOnShortPressHandler(func f)
    {
      onShortPress = f;
    }

    void setOnLongPressHandler(func f)
    {
      onLongPress = f;
    }

    // Checks whether the button is currently being held down
    bool isHeldDown()
    {
      return buttons & buttonByte;
    }

    // Gets the amount of time that the button has been held down for
    unsigned long getTimeHeld()
    {
      return (!isHeldDown() ? 0 : currentTime - lastTimeHeld);
    }

    // Gets the amount of time since the handler has registered a press
    unsigned long getTimeSincePress()
    {
      return (!isHeldDown() ? 0 : currentTime - lastTimePressed);
    }

    // Getter function for the times pressed while held
    int getTimesPressedWhileHeld()
    {
      return timesPressedWhileHeld;
    }

    // Updates the internal state of the handler
    void update()
    {
      unsigned long previousTimeHeldFor = 0;
      
      if (isHeldDown() && !currentButtonState)
      {
        currentButtonState = true;
        lastTimeHeld = currentTime;
        lastTimePressed = currentTime;
      }
      else if (!isHeldDown() && currentButtonState)
      {
        currentButtonState = false;
        timesPressedWhileHeld = 0;
        previousTimeHeldFor = currentTime - lastTimeHeld;
      }

      if (onLongPress != NULL)
      {
        if (getTimeHeld() >= longPressTime)
        {          
          if (timesPressedWhileHeld == 0)
          {            
            registerPress();
            onLongPress();
          }
        }
      }
      
      if (onShortPress != NULL)
      {
        if (getTimeSincePress() >= shortPressTime && onLongPress == NULL)
        {
          registerPress();
          onShortPress();
        }
        else if (!isHeldDown())
        {
          if (previousTimeHeldFor >= shortPressTime && previousTimeHeldFor < longPressTime)
          {
            registerPress();
            timesPressedWhileHeld = 0;
            onShortPress();
          }
        }
      }      
    }

    // Causes the handler to register a press at the current time
    void registerPress()
    {
      lastTimePressed = currentTime;
      timesPressedWhileHeld++;
    }
};

// A selection of button handlers for all of the buttons on the Arduino
ButtonHandler upButtonHandler (BUTTON_UP);
ButtonHandler downButtonHandler (BUTTON_DOWN);
ButtonHandler leftButtonHandler (BUTTON_LEFT);
ButtonHandler rightButtonHandler (BUTTON_RIGHT);
ButtonHandler selectButtonHandler (BUTTON_SELECT);

// Sets the cursor position to the given x and y co-ordinates
void setCursorPos(int x, int y)
{
  cursorX = x;
  cursorY = y;
  lcd.setCursor(cursorX, cursorY);
}

// Gets the width in number of characters of a so-called 'big' character (character that spans two lines)
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

// Writes a 'big' character to the screen with a given width
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

// Prints a 'big' character to the screen given by a regular character
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

// Prints a string of characters at a specified cursor position
void printToScreen(String str, int x=cursorX)
{
  cursorX = x;
  
  for (int i = 0; i < str.length(); i++)
  {
    printChar(str.charAt(i));
  }
  
  currentText = str;
}

// Updates the text on the screen with a new string of characters at a specified cursor position
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

// Gets the width in number of characters that a string of characters will take up on the screen
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

// Gets the position of a string of characters so that it will be printed centred
int getCentrePos(String str)
{  
  return (screenWidth / 2) - ((float)getWidthOnScreen(str) / 2);
}

// Updates the time on the screen
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

// Resets the blinking of the screen and updates the screen
void resetBlink(bool b=true)
{
    blinkTimer = 0;
    blinking = b;
    updateScreenTime();
    previousBlinkTime = millis();
}

void updateOtherScreenChars()
{
  setCursorPos(screenWidth - 1, 0);

  switch (currentMode)
  {
    case Mode::CLOCK:
      lcd.write(' ');
      break;
    case Mode::CLOCKSET:
      lcd.write('C');
      break;
    case Mode::ALARMSET:
      lcd.write('A');
      break;
  }
  
  setCursorPos(screenWidth - 1, 1);
  
  lcd.write((alarmOn) ? (char)CustomChars::ALARM : ' ');
}

// Changes the current mode of the alarm clock
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

  updateOtherScreenChars();
}

// Updates the Arduino's flash memory to set the current hour and minute displayed on screen
void updateEEPROMClock()
{
  EEPROM.write(0, screenTime.getTimePart(Time::HOUR));
  EEPROM.write(1, screenTime.getTimePart(Time::MINUTE));
}

void dummyFunc() { return; }

void selectButtonLongPress()
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
}

void selectButtonShortPress()
{
  alarmOn = !alarmOn;
  updateOtherScreenChars();
}

void upButtonShortPress()
{
  if (currentMode == Mode::CLOCKSET)
  {
    unsigned long previousTime = screenTime.getTotalMillis();
          
    if (selectedEditPart == Time::SECOND)
    {
      screenTime.setTimePart(Time::SECOND, 0);
    }
    else
    {
      screenTime.addTimePart(selectedEditPart, 1);
    }
    
    clockOffset += (screenTime.getTotalMillis() - previousTime);
    
    resetBlink(false);
  }
}

void downButtonShortPress()
{
  if (currentMode == Mode::CLOCKSET)
  {
    unsigned long previousTime = screenTime.getTotalMillis();
          
    if (selectedEditPart == Time::SECOND)
    {
      screenTime.setTimePart(Time::SECOND, 0);
    }
    else
    {
      screenTime.subtractTimePart(selectedEditPart, 1);
    }
    
    clockOffset += (screenTime.getTotalMillis() - previousTime);
    
    resetBlink(false);
  }
}

void leftButtonLongPress()
{  
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

void leftButtonShortPress()
{
  if (currentMode == Mode::CLOCKSET)
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
  }
}

void rightButtonShortPress()
{
  if (currentMode == Mode::CLOCKSET)
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
  }
}

// Sets up a number of custom characters
void setupCustomChars()
{
  lcd.createChar((int)CustomChars::BACKSLASH, backslash);
  lcd.createChar((int)CustomChars::BLTOTRDIAG, bltotrdiag);
  lcd.createChar((int)CustomChars::TRTOBLDIAG, trtobldiag);
  lcd.createChar((int)CustomChars::TOPLINE, topline);
  lcd.createChar((int)CustomChars::UPPERBRACKET, upperbracket);
  lcd.createChar((int)CustomChars::SIXSTEM, sixstem);
  lcd.createChar((int)CustomChars::ALARM, alarm);
}

void setup()
{
  Serial.begin(9600);
  lcd.begin(screenWidth, screenHeight);
  setupCustomChars();

  backlightColor = defaultBacklightColor;
  lcd.setBacklight(backlightColor);

  currentMode = Mode::CLOCK;
  selectedEditPart = Time::HOUR;

  selectButtonHandler.setOnLongPressHandler(selectButtonLongPress);
  selectButtonHandler.setOnShortPressHandler(selectButtonShortPress);
  upButtonHandler.setOnShortPressHandler(upButtonShortPress);
  downButtonHandler.setOnShortPressHandler(downButtonShortPress);
  leftButtonHandler.setOnLongPressHandler(leftButtonLongPress);
  leftButtonHandler.setOnShortPressHandler(leftButtonShortPress);
  rightButtonHandler.setOnLongPressHandler(dummyFunc);
  rightButtonHandler.setOnShortPressHandler(rightButtonShortPress);

  int h = EEPROM.read(0);
  int m = EEPROM.read(1);

  screenTime.setTime(h, m, 0);

  int ah = EEPROM.read(2);
  int am = EEPROM.read(3);

  alarmTime.setTime(ah, am, 0);
  
  Serial.println("Alarm time: " + alarmTime.getReadableShort());

  updateScreenTime();
  updateOtherScreenChars();

  currentTime = millis();
  clockResetTime = currentTime;
  clockOffset = screenTime.getTotalMillis() - clockResetTime;
}

void loop()
{  
  currentTime = millis();
  currentScreenTime = currentTime + clockOffset;
  clockTimer = currentScreenTime - screenTime.getTotalMillis();

  if (currentMode == Mode::CLOCKSET) blinkTimer = currentTime - previousBlinkTime;

  if (alarmActive) alarmFlashTimer = currentTime - previousAlarmFlashTime;

  if (clockTimer >= 1000)
  {
    int prevMins = screenTime.getTimePart(Time::MINUTE);
    
    if (currentScreenTime >= 86400000)
    {
      screenTime.setTime(0);
      
      clockResetTime = currentTime;
      clockOffset = screenTime.getTotalMillis() - clockResetTime;
      currentScreenTime = currentTime + clockOffset;
    }
    else
    {
      screenTime.setTime(currentScreenTime);
    }

    if (screenTime.getTimePart(Time::MINUTE) != prevMins && currentMode != Mode::CLOCKSET) updateEEPROMClock();

    if (alarmOn && screenTime == alarmTime)
    {
      Serial.println("Alarm started!");
      alarmActive = true;
    }

    updateScreenTime();
  }

  if (blinkTimer >= blinkTime && currentMode == Mode::CLOCKSET)
  {
    previousBlinkTime = currentTime;
    
    if (clockTimer < 1000) updateScreenTime();
    
    blinkTimer = 0;
  }

  if (alarmFlashTimer >= alarmFlashTime && alarmActive)
  {
    previousAlarmFlashTime = currentTime;
    
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
  
  upButtonHandler.update();
  downButtonHandler.update();
  leftButtonHandler.update();
  rightButtonHandler.update();
  selectButtonHandler.update();
}

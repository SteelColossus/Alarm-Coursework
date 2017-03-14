#include <Adafruit_RGBLCDShield.h>

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

/* This is a digital font
 * 
uint8_t fullborder[8] = {31, 17, 17, 17, 17, 17, 17, 31};
uint8_t leftrighttopborder[8] = {31, 17, 17, 17, 17, 17, 17, 17};
uint8_t leftrightbottomborder[8] = {17, 17, 17, 17, 17, 17, 17, 31};
uint8_t lefttopbottomborder[8] = {31, 16, 16, 16, 16, 16, 16, 31};
uint8_t righttopbottomborder[8] = {31, 1, 1, 1, 1, 1, 1, 31};
uint8_t righttopborder[8] = {31, 1, 1, 1, 1, 1, 1, 1};
uint8_t rightborder[8] = {1, 1, 1, 1, 1, 1, 1, 1};
uint8_t lefttopborder[8] = {31, 16, 16, 16, 16, 16, 16, 16};

enum class CustomChars : uint8_t { FULLBORDER, LEFTRIGHTTOPBORDER, LEFTRIGHTBOTTOMBORDER, LEFTTOPBOTTOMBORDER, RIGHTTOPBOTTOMBORDER, RIGHTTOPBORDER, RIGHTBORDER, LEFTTOPBORDER };

uint8_t digits[][2] = { {(uint8_t)CustomChars::LEFTRIGHTTOPBORDER, (uint8_t)CustomChars::LEFTRIGHTBOTTOMBORDER},
                        {(uint8_t)CustomChars::RIGHTBORDER, (uint8_t)CustomChars::RIGHTBORDER},
                        {(uint8_t)CustomChars::RIGHTTOPBORDER, (uint8_t)CustomChars::LEFTTOPBOTTOMBORDER},
                        {(uint8_t)CustomChars::RIGHTTOPBORDER, (uint8_t)CustomChars::RIGHTTOPBOTTOMBORDER},
                        {(uint8_t)CustomChars::LEFTRIGHTBOTTOMBORDER, (uint8_t)CustomChars::RIGHTBORDER},
                        {(uint8_t)CustomChars::LEFTTOPBORDER, (uint8_t)CustomChars::RIGHTTOPBOTTOMBORDER},
                        {(uint8_t)CustomChars::LEFTTOPBORDER, (uint8_t)CustomChars::FULLBORDER},
                        {(uint8_t)CustomChars::RIGHTTOPBORDER, (uint8_t)CustomChars::RIGHTBORDER},
                        {(uint8_t)CustomChars::LEFTRIGHTTOPBORDER, (uint8_t)CustomChars::FULLBORDER},
                        {(uint8_t)CustomChars::LEFTRIGHTTOPBORDER, (uint8_t)CustomChars::RIGHTTOPBOTTOMBORDER}};
 */

uint8_t colon[2] = {165, 165};
uint8_t space[2] = {' ', ' '};
 
int cursorX = 0;
int cursorY = 0;

String currentText = "";

unsigned long currentTime = 0;
unsigned long previousTime = 0;
int waitTime = 1000;

uint8_t buttons;

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
};

Time screenTime;

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
}

void updateScreen(String str, int x=cursorX)
{
  if (str.length() != currentText.length())
  {
    printToScreen(str, x);
  }
  else if (str != currentText)
  {
    cursorX = x;
    
    for (int i = 0; i < str.length(); i++)
    {
      char c = str.charAt(i);

      if (c != currentText.charAt(i))
      {
        printChar(c);
      }
      else
      {
        cursorX += getBigCharWidth(c);
      }
    }
  }

  currentText = str;
}

int getCentrePos(String str)
{
  int len = 0;

  for (int i = 0; i < str.length(); i++)
  {
    char c = str.charAt(i);

    len += getBigCharWidth(c);
  }
  
  return (screenWidth / 2) - ((float)len / 2);
}

void updateScreenTime()
{
  String output = screenTime.getReadable();
  updateScreen(output, getCentrePos(output));
}

void setupCustomChars()
{
  lcd.createChar((int)CustomChars::BACKSLASH, backslash);
  lcd.createChar((int)CustomChars::BLTOTRDIAG, bltotrdiag);
  lcd.createChar((int)CustomChars::TRTOBLDIAG, trtobldiag);
  lcd.createChar((int)CustomChars::TOPLINE, topline);
  lcd.createChar((int)CustomChars::UPPERBRACKET, upperbracket);
  lcd.createChar((int)CustomChars::SIXSTEM, sixstem);

  /* For the digital font
  lcd.createChar((int)CustomChars::FULLBORDER, fullborder);
  lcd.createChar((int)CustomChars::LEFTRIGHTTOPBORDER, leftrighttopborder);
  lcd.createChar((int)CustomChars::LEFTRIGHTBOTTOMBORDER, leftrightbottomborder);
  lcd.createChar((int)CustomChars::LEFTTOPBOTTOMBORDER, lefttopbottomborder);
  lcd.createChar((int)CustomChars::RIGHTTOPBOTTOMBORDER, righttopbottomborder);
  lcd.createChar((int)CustomChars::RIGHTTOPBORDER, righttopborder);
  lcd.createChar((int)CustomChars::RIGHTBORDER, rightborder);
  lcd.createChar((int)CustomChars::LEFTTOPBORDER, lefttopborder);
   */
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.begin(screenWidth, screenHeight);
  setupCustomChars();

  Serial.setTimeout(2000);
  
  int h = 0; int m = 0;

  Serial.print("Enter the number of hours: ");

  h = Serial.parseInt();

  Serial.println(h);
  Serial.print("Enter the number of minutes: ");

  m = Serial.parseInt();
  
  Serial.println(m);

  screenTime.setTime(h, m, 0);
}

void loop() {  
  // put your main code here, to run repeatedly:
  currentTime = millis();

  if (previousTime == 0 || currentTime - previousTime >= waitTime)
  {
    if (previousTime > 0) screenTime.addTime(waitTime);

    previousTime = currentTime;

    updateScreenTime();
  }

  // Playing about, change later
  buttons = lcd.readButtons();

  if (buttons)
  {
    if (buttons & BUTTON_UP)
    {
      screenTime.addTimePart(Time::HOUR, 1);
      updateScreenTime();
    }
    if (buttons & BUTTON_RIGHT)
    {
      screenTime.addTimePart(Time::MINUTE, 1);
      updateScreenTime();
    }
  }
  
  //Serial.println(currentTime - previousTime);
}

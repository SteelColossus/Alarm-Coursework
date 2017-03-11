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

enum class CustomChars : uint8_t { BACKSLASH = 0, BLTOTRDIAG, TRTOBLDIAG, TOPLINE, UPPERBRACKET, SIXSTEM };

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

enum Characters { ZERO = 48, ONE = 49, TWO = 50, THREE = 51, FOUR = 52, FIVE = 53, SIX = 54, SEVEN = 55, EIGHT = 56, NINE = 57, COLON = 58, SPACE = 32 };
                       
int cursorX = 0;
int cursorY = 0;

unsigned long currentTime = 0;
unsigned long previousTime = 0;
int waitTime = 1000;

class Time
{
  private:
    int hours;
    int minutes;
    int seconds;

  public:
    void setTime(int h, int m, int s)
    {
      hours = h;
      minutes = m;
      seconds = s;
    }
  
    void setTime(unsigned long t)
    {
      seconds = t % 60;
      minutes = (t / 60) % 60;
      hours = (t / (60*60)) % 24;
    }

    void addTime(int h, int m, int s)
    {
      addTime(h*60*60 + m*60 + s);
    }

    void addTime(unsigned long t)
    {
      seconds += t;

      if (seconds >= 60)
      {
        minutes += seconds / 60;
        seconds = seconds % 60;
      }

      if (minutes >= 60)
      {
        hours += (minutes / 60) % 24;
        minutes = minutes % 60;
      }

      if (hours >= 24) hours = hours % 24;
    }

    String getReadableShort()
    {
      String res = "";

      if (hours < 10) res += "0";
      res += (String)hours;

      res += ":";

      if (minutes < 10) res += "0";
      res += (String)minutes;

      return res;
    }

    String getReadable()
    {
      String res = getReadableShort();
            
      res += ":";
      
      if (seconds < 10) res += "0";
      res += (String)seconds;
       
      return res;
    }
};

Time screenTime;

void setCursorPos(int x, int y)
{
  cursorX = x;
  cursorY = y;
  lcd.setCursor(cursorX, cursorY);
}

void writeBigChar(uint8_t * c, int width=2)
{
  cursorY = 0;
  
  for (int i = 0; i < width*2; i++)
  {
    setCursorPos(cursorX, cursorY);
    
    lcd.write((char)c[i]);
    
    if (i == width - 1)
    {
      cursorX -= (width - 1);
      cursorY = 1;
    }
    else
    {
      cursorX++;
    }
  }
}

void printCharacter(Characters c, int x=cursorX)
{
  setCursorPos(x, 0);
  
  switch (c)
  {
    case ZERO:
    case ONE:
    case TWO:
    case THREE:
    case FOUR:
    case FIVE:
    case SIX:
    case SEVEN:
    case EIGHT:
    case NINE:
      writeBigChar(digits[(int)c - (int)ZERO]);
      break;
    case COLON:
      writeBigChar(colon, 1);
      break;
    case SPACE:
      writeBigChar(space, 1);
      break;
  }
}

void printToScreen(String str, int x=cursorX)
{
  setCursorPos(x, 0);
  
  for (int i = 0; i < str.length(); i++)
  {
    char c = str.charAt(i);
    Characters test = static_cast<Characters>(c);

    printCharacter(test);
  }
}

int getCentrePos(String str)
{
  double centrePos = (screenWidth / 2);

  for (int i = 0; i < str.length(); i++)
  {
    char c = str.charAt(i);

    if (c >= '0' && c <= '9')
    {
      centrePos -= 1;
    }
    else if (c == COLON || c == SPACE)
    {
      centrePos -= 0.5;
    }
  }
  
  return centrePos;
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
   * 
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

  h = (Serial.parseInt() % 60);

  Serial.println(h);
  Serial.print("Enter the number of minutes: ");

  m = (Serial.parseInt() % 60);
  
  Serial.println(m);

  screenTime.setTime(h, m, 0);
}

void loop() {  
  // put your main code here, to run repeatedly:
  currentTime = millis();

  if (previousTime == 0 || currentTime - previousTime >= waitTime)
  {
    if (previousTime > 0) screenTime.addTime(waitTime / 1000);

    previousTime = currentTime;

    String output = screenTime.getReadable();

    printToScreen(output, getCentrePos(output));

    // Playing about, change later
    uint8_t buttons = lcd.readButtons();
 
    if (buttons)
    {
      if (buttons & BUTTON_UP)
      {
        screenTime.addTime(1, 0, 0);
      }
      if (buttons & BUTTON_RIGHT)
      {
        screenTime.addTime(0, 1, 0);
      }
    }
  }
}

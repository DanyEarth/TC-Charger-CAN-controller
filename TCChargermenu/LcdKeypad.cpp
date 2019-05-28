#include "LcdKeypad.h"

#define BUTTON_REPEAT_DELAY         800
#define BUTTON_REPEAT_SPEED_DELAY   250

const int buttonValues[] = {BUTTON_RIGHT_ANALOG_VALUE,
                            BUTTON_UP_ANALOG_VALUE,
                            BUTTON_DOWN_ANALOG_VALUE,
                            BUTTON_LEFT_ANALOG_VALUE,
                            BUTTON_SELECT_ANALOG_VALUE};

byte buttonBuffer[5];
char buttonBufferCount = 0;
byte button_write_pos = 0;
byte button_read_pos = 0;
volatile byte displayBrightness = 4;
volatile byte backlightState = 1;

unsigned long buttonSampleTime =0;
byte buttonState[5];              // current up or down state for each of the buttons
unsigned long buttonPressTime[5]; // press time for each of the buttons
unsigned long buttonHoldTime[5];  // hold time for each of the buttons

char* padc (char chr, unsigned char count = 0);

// ----------------------------------------------------------------------------------------------------
void backLightOn()
{
  backlightState = 1;
  pinMode(BACKLIGHT_PIN, INPUT);
}

// ----------------------------------------------------------------------------------------------------
void backLightOff()
{
  backlightState = 0;
  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, LOW);
}

// ----------------------------------------------------------------------------------------------------
void setBacklightBrightness(byte brightness)
{
  displayBrightness = constrain(brightness, 1, 4);
}

// ----------------------------------------------------------------------------------------------------
void lcdBacklightISR()
{
  const byte dutyCycle = 4;
  static byte pulseWidth;

  if (!backlightState)
  {
    return;
  }
  if (pulseWidth > dutyCycle)
  {
    pulseWidth = 0;
    //back light On
    pinMode(BACKLIGHT_PIN, INPUT);
  }
  else if (pulseWidth > displayBrightness)
  {
    //back light off
    pinMode(BACKLIGHT_PIN, OUTPUT);
    digitalWrite(BACKLIGHT_PIN, LOW);
  }
  pulseWidth++;
}

// ----------------------------------------------------------------------------------------------------
char *inttostr(char *dest, short integer)
{
  if (integer == 0)
  {
    strcpy(dest, "0");
  }
  else
  {
    byte sign = 0;
    
    if (integer < 0)
    {
      sign = 1;
      dest[0] = '-';
      integer = integer * -1;
    }

    byte idx = 0;

    if (integer > 9999) idx = 4;
    else if (integer > 999) idx = 3;
    else if (integer > 99) idx = 2;
    else if (integer > 9) idx = 1;
    else if (integer > 0) idx = 0;    

    idx += sign;
    dest[idx+1] = 0;
    
    for (; idx >=0 && integer !=0; integer /= 10, idx--)
    {
      dest[idx]=(integer % 10) + '0';
    }
  }
  return dest;
}


// ----------------------------------------------------------------------------------------------------
char *fmt (char *dest, unsigned char argc, ... )
{
  unsigned char buflen = 0;
  char* str;
  
  va_list ap;
  va_start(ap, argc);

  for (int i = 0; i < argc && buflen < LCD_COLS; i++)
  {
    str = va_arg(ap, char*);
    unsigned char len = strlen(str);
    unsigned char cpylen = (buflen + len) > LCD_COLS ? LCD_COLS - buflen : len;

    strncpy((dest + buflen), str, cpylen);
    buflen += len;
  }
  va_end(ap);
  dest[buflen] = 0;
  return dest;
}


// ----------------------------------------------------------------------------------------------------
char *rpad (char *dest, const char *str, char chr, unsigned char width)
{
  unsigned char len = strlen(str);
  
  width = width > LCD_COLS ? LCD_COLS : width;
  
  if (len < LCD_COLS && width > len)
  {
    strcpy(dest, str);
    strcat(dest, padc(chr, width - len));
  }
  else
  {
    strncpy(dest, str, width+1);
  }
  return dest;
}

// ----------------------------------------------------------------------------------------------------
char *lpad (char *dest, const char *str, char chr, unsigned char width)
{
  unsigned char len = strlen(str);
  
  width = width > LCD_COLS ? LCD_COLS : width;

  if (len < LCD_COLS && width > len)
  {
    strcpy(dest, padc(chr, width - len));
    strcat(dest, str);
  }
  else
  {
    strncpy(dest, str, width+1);
  }
  return dest;
}


// ----------------------------------------------------------------------------------------------------
char *padc (char chr, unsigned char count)
{
  static char strbuf[LCD_COLS + 1];

  count = (count > LCD_COLS) ? LCD_COLS : count;

  int i;
  for (i=0; i < count; i++)
  {
    strbuf[i] = chr;
  }
  strbuf[i] = 0;
  
  return strbuf;
}


// ----------------------------------------------------------------------------------------------------
void queueButton (byte button)
{
  if (buttonBufferCount <= sizeof (buttonBuffer))
  {
    buttonBuffer [button_write_pos] = button;
    buttonBufferCount++;
    button_write_pos++;
    
    if (button_write_pos >= sizeof (buttonBuffer))
    {
      button_write_pos = 0;
    }
  }
}

// ----------------------------------------------------------------------------------------------------
byte getButton ()
{
  buttonHandlerCycle(); // if calling buttonHandlerCycle() from ISR, comment it out here.
  
  byte button = 0;
  
  if (buttonBufferCount > 0)
  {
    button = buttonBuffer [button_read_pos];
    buttonBufferCount--;
    button_read_pos++;
    
    if (button_read_pos >= sizeof (buttonBuffer))
    {
      button_read_pos = 0;
    }
  }
  
  return button;
}

// ----------------------------------------------------------------------------------------------------
// If calling from ISR, comment out call in getButton() method.
void buttonHandlerCycle()
{
  if (millis() - buttonSampleTime >= 20)
  {
    buttonSampleTime = millis();
    
    byte btnStateNow;
    int analogReading = analogRead (BUTTON_PIN);

    byte buttonIdentified = 0;
    
    for (int i=0; i < 5; i++)
    {
      if (!buttonIdentified && analogReading <  buttonValues[i])
      {
        btnStateNow = 1;
        buttonIdentified = 1;
      }
      else
      {
        btnStateNow = 0;
      }

      // If button state has changed, action the change.

      if (buttonState[i] != btnStateNow)
      {
        // if button state changes to pressed, queue SHORT PRESS to buffer.
        if (btnStateNow)
        {
          queueButton((i+1) | BUTTON_PRESSED_IND);
          buttonPressTime[i] = millis();
          buttonHoldTime[i] = buttonPressTime[i];
        }
        else
        {
          // otherwise button state has changed to up, queue SHORT or LONG RELEASE state to buffer, and reset pressed time counter.
          if (millis() - buttonPressTime[i] > BUTTON_REPEAT_DELAY)
          {
            queueButton((i+1) | BUTTON_LONG_RELEASE_IND);
          }
          else
          {
            queueButton((i+1) | BUTTON_SHORT_RELEASE_IND);
          }
        }
        buttonState[i] = btnStateNow;
      }

      // if button state pressed, increment pressed time counter. Queue LONG PRESS to buffer, if button is held long.  
      if (btnStateNow)
      {
        if ((millis() - buttonPressTime[i] > BUTTON_REPEAT_DELAY) && (millis() - buttonHoldTime[i] > BUTTON_REPEAT_SPEED_DELAY))
        {
          queueButton((i+1) | BUTTON_LONG_PRESSED_IND);
          buttonHoldTime[i] = millis();
        }
      }
    }
  }
}

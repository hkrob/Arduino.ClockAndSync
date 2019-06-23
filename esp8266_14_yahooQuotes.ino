;/* ROB
 Pins ARDUINO / Clock
 3v VCC
 GND GND
 A4 SDA
 A5 SCL

 LCD
 * LCD 1 (VSS) to Ground
 * LCD 2 (VDD) to Positive
 * LCD 3 (V0) to Wiper of Potentiometer (Middle Pin)
 * LCD 4 (RS) to Arduino 12
 * LCD 5 (RW) to Ground
 * LCD 6 (E) to Arduino 11
 * LCD 7 D0 - not connected
 * LCD 8 D1 - not connected
 * LCD 9 D2 - not connected
 * LCD 10 D3 - not connected
 * LCD 11 D4 - Arduino 5
 * LCD 12 D5 - Arduino 4
 * LCD 13 D6 - Arduino 3
 * LCD 14 D7 - Arduino 2
 * LCD 15 A - Positive
 * LCD 16 K - Negative via 220 resistor

 WIFI
 Arduio ESP8266
When programming with LUALOADER or hardware serial
 RX TX
 TX RX
 */
boolean pause = false;


String command = "UNDEFcommand";
String response = "UNDEFresponse";

// button
const int buttonPin = 9;
int buttonState = 0;
//


#include <ctype.h> // http://www-control.eng.cam.ac.uk/~pcr20/www.cppreference.com/stdstring_details.html

// Time Sync
// Offset hours
const int offset = 8;   // Hong Kong
#include <Time.h>
boolean timeSetFromGoogle = false;
// End NTP

// memory saving for serial print
// from http://www.utopiamechanicus.com/article/low-memory-serial-print/
#include <avr/pgmspace.h>
int freeRam ()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

//
#include <SoftwareSerial.h>
SoftwareSerial swSerial(7, 8); //TX,RX

//WIFI
// #define DEBUG true
#define LCDDEBUG true
boolean WiFiConnected = false;
// end WIFI

// LCD
#include <LiquidCrystal.h>
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
// end LCD

// Clock
#include "Wire.h"
#define DS3231_I2C_ADDRESS 0x68

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return ( (val / 10 * 16) + (val % 10) );
}
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val / 16 * 10) + (val % 16) );
}

//End Clock

// Timer
unsigned long previousMillis = 0; // last time update
long TimeSyncInterval = 600000; // (milliseconds) 60000 = 10 minutes
// end Timer

// Connect to WiFi
boolean connectWiFi() {
  pause = true;
  // return true if connected
  WiFiConnected = false;
  command = "node.restart\(\)"; // this command is sent to the LUA interpreter on the ESP
  lcd.setCursor(0, 0);
  lcd.print(F("Init WiFi"));
  if (sendCommand(command, 10000, false, "OK Connected")) // OK Connected is the string the ESP replies with when all OK
  {
    //swSerial.println(F("WiFi Connected"));
    lcd.setCursor(0, 1);
    lcd.print(F("WiFi Connected"));
    WiFiConnected = true;
  }
  else
  {
    // wifi not connected
    lcd.setCursor(0, 1); // row 0, column 0
    lcd.print(F("WiFi NOT OK"));
    // swSerial.println(F("setup:WiFi NOT OK"));
    WiFiConnected = false;
  };
  pause = false;
  return (WiFiConnected);
};

void setup()
{
  swSerial.begin(9600);
  swSerial.println(F("###### setup begin ######"));
  pinMode(buttonPin, INPUT);
  int trys = 0;
  int attempts = 0;
  boolean connected = false;

  // LCD
  lcd.begin(16, 2); // set up the LCD's number of columns and rows:
  lcd.clear();
  lcd.setCursor(0, 0); // row 0, column 0
  lcd.print(F("Clock & Quote"));
  lcd.setCursor(0,1);
  lcd.print(F("v2.5"));
  delay(1000);
  lcd.clear();
  // End LCD

  // WIFI
  Serial.begin(9600); // ESP8266 connected to hardware serial ports
  command = "node.restart\(\)"; // this command is sent to the LUA interpreter on the ESP
  lcd.setCursor(0, 0);
  lcd.print(F("Init WiFi"));
  trys = 0;
  attempts = 3;
  while (!connectWiFi() && trys < attempts) {
    lcd.clear();
    lcd.setCursor(0, 0);
    // lcd.print ("Attemping WiFi");
    lcd.setCursor(0, 1);
    // lcd.print(String("Attempt:" + String(trys + 1) + " / " + String(attempts)));
    trys++;
    if (trys > 0) delay(5000);
    delay(100);
  }

  if (WiFiConnected)
  {
    lcd.clear();
    //swSerial.println(F("WiFi Connected"));
    lcd.setCursor(0, 1);
    // lcd.print(F("WiFi Connected"));
  }
  else
  {
    // wifi not connected
    lcd.clear();
    lcd.setCursor(0, 1); // row 0, column 0
    lcd.print(F("WiFi NOT OK"));
    //swSerial.println(F("setup: WiFi NOT OK"));
    delay(3000);
    halt();
  };
  //
  //  displayYahooQuoteWrapper();
  //
  //Clock
  Wire.begin();
  // set the initial time here:
  // DS3231 seconds, minutes, hours, day, date, month, year Sunday=1
  // setDS3231time(00,39,15,2,19,01,15);

  //Try to get the date and time
  askGoogleWrapper();
  swSerial.println("###### setup end ######");
} // end setup
// #
//

void loop()
{
  // swSerial.println("###### loop begin ######"); // only for diehard debugging
  //  displayTimeLCD();
  // delay(1000); // every 1 second

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > TimeSyncInterval) {
    previousMillis = currentMillis;
    // re-sync the time
    askGoogleWrapper();
    swSerial.println(F("Requesting time re - sync"));
  };
  if (currentMillis - previousMillis > 999) {
    previousMillis = currentMillis;
    // show the time
    displayTimeLCD();

  };


  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    // get yahoo quotes
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Getting Info"));
    swSerial.println("Button pressed!!");
    displayYahooQuoteWrapper();
  }

  // swSerial.println("###### loop end ######"); // only for diehard debugging
}
//
void sendData(String command, const int timeout, boolean debug, int maxResponseLength)
// Command to send to ESP8266, ms to timeout, debug control
{
  swSerial.println(F("###### sendData begin ######"));
  if (debug)
  {
    swSerial.println(F("-- - sendData begin -- -"));
    swSerial.println(F("-- - function inputs begin -- -"));
    swSerial.println("command:" + String(command));
    swSerial.println("timeout:" + String(timeout));
    swSerial.println("debug:" + String(debug));
    swSerial.println(F("-- - function inputs end -- -"));
  };
  response = "";
  Serial.println(command); // send command to the esp8266
  long int time = millis(); // millis is the time the arduino has been running the current program
  while ( (time + timeout) > millis()  ) // start the countdown
  {
    while (Serial.available())
    {
      // The esp has data so display its output to the serial window
      char c = Serial.read(); // read the next character.
      response += c;
    }
  }
  if (debug)
  {
    swSerial.println(F("-- - ESP8266 response begin -- -"));
    // swSerial.print(response);
    swSerial.println(F("-- - ESP8266 response end -- -"));
    swSerial.println(F("-- - sendData end -- -"));
  }
  swSerial.println(F("###### sendData end ######"));
  // return response;
};

boolean sendCommand(String command, const int timeout, boolean debug, String expectedResponse)
/*
Command to send to ESP8266, ms to timeout, debug control, expected response
expected response can be a substring of the full response
returns true if all OK
*/
{
  // swSerial.println(F("###### sendCommand begin ######"));
  int matched = -1;
  swSerial.println(F("-- - function inputs begin -- -"));
  swSerial.println("timeout:" + String(timeout));
  swSerial.println("debug:" + String(debug));
  swSerial.println("expectedResponse:" + String(expectedResponse));
  swSerial.println(F("-- - function inputs end -- -"));
  swSerial.flush();

  response = "";
  Serial.println(command); // send command to the esp8266
  long int time = millis(); // millis is the time the arduino has been running the current program
  while ( (time + timeout) > millis())  // start the countdown
  {
    while (Serial.available())
    {
      // The esp has data so display its output to theswSerial window
      char c = Serial.read(); // read the next character.
      response += c;
    }
  }
  matched = response.lastIndexOf(expectedResponse);
  swSerial.flush(); // wait forswSerial buffer to be sent
  swSerial.print(F("matched:"));
  swSerial.println(matched);
  //  swSerial.println(F("###### sendCommand end ######"));
  if (matched >= 0)
  {
    return true; // match found
  }
  return false; // no match
}
//

void halt()
{
  swSerial.println(F("###### halt: Arduino Halted"));
  swSerial.println(F("reset required to continue ######"));
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("halt: halted"));
  abort();
}
void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte
                   dayOfMonth, byte month, byte year)
{
  //Serial.println(F("###### setDS3231time begin ######"));
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
  //Serial.println(F("###### setDS3231time end ######"));
}
void readDS3231time(byte *second,
                    byte *minute,
                    byte *hour,
                    byte *dayOfWeek,
                    byte *dayOfMonth,
                    byte *month,
                    byte *year)
{
  //Serial.println(F("###### readDS3231time begin ######"));
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
  //Serial.println(F("###### readDS3231time end ######"));
}

void displayTimeLCD()
{
  //Serial.println(F("###### displayTimeLCD begin ######"));
  String time = "UNDEFtime";
  String date = "UNDEFdate";
  String monthText = "UNDEFmonthText";
  String weekdayText = "UNDEFweekdayText";

  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  time = String(hour) + ":";
  if (minute < 10)
  {
    time = (time + "0");
  }

  time = (time + String(minute) + ":");

  if (second < 10)
  {
    // Serial.print("0");
    time = (time + "0");
  }
  time = (time + String(second));

  switch (month)
  {
    case 1:
      monthText = "Jan";
      break;
    case 2:
      monthText = "Feb";
      break;
    case 3:
      monthText = "Mar";
      break;
    case 4:
      monthText = "Apr";
      break;
    case 5:
      monthText = "May";
      break;
    case 6:
      monthText = "Jun";
      break;
    case 7:
      monthText = "Jul";
      break;
    case 8:
      monthText = "Aug";
      break;
    case 9:
      monthText = "Sep";
      break;
    case 10:
      monthText = "Oct";
      break;
    case 11:
      monthText = "Nov";
      break;
    case 12:
      monthText = "Dec";
      break;
  }
//  date = (String(dayOfMonth) + "/" + monthText + "/" + String(year));

  //Serial.print(" Day of week:");
  switch (dayOfWeek)
  {
    case 1:
      weekdayText = "Sun";
      break;
    case 2:
      weekdayText = "Mon";
      break;
    case 3:
      weekdayText = "Tue";
      break;
    case 4:
      weekdayText = "Wed";
      break;
    case 5:
      weekdayText = "Thu";
      break;
    case 6:
      weekdayText = "Fri";
      break;
    case 7:
      weekdayText = "Sat";
      break;
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(time);
  lcd.setCursor(0, 1);
  String lcdDateOutput = "UNDEFlcdDateOutput";
  lcdDateOutput = ((weekdayText + "," + monthText) + " " + String(dayOfMonth) + ",20" + String(year));
  lcd.print(lcdDateOutput);
}

void askGoogleWrapper() {
  int trys = 0;
  int attempts = 0;
  timeSetFromGoogle = false;
  while (!askGoogle() && trys < 10) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print ("Asking Google");
    lcd.setCursor(0, 1);
    lcd.print(String("Attempt:" + String(trys + 1) + " / " + String(attempts)));
    trys++;
    if (trys > 0) delay(5000);
    delay(100);
  }
}
//
boolean askGoogle() {
  swSerial.println(F("askGoogle begin"));
  command = ("dofile\(\"googleTime.lua\"\)");
  sendData(command, 5000, false, 50);
  //
  // check to see if response contains something that proves a valid response
  int GMTfound = 0;
  GMTfound = response.indexOf('GMT');
  if (GMTfound < 1) return 0;
  timeSetFromGoogle = true;
  //
  // now to separate the input which is in the form: > Sun, 25 Jan 2015 01:50:45 GMT

  String dayText = (getValue(response, '>', 1) ); // dayText:Sun,
  swSerial.println("dayText:" + dayText);
  dayText = (getValue(dayText, ',', 0) ); // dayText:Sun,
  //swSerial.println("dayText:" + dayText);
  //
  response = ((getValue(response, ',', 1))); // re use response, but without the Day of the week
  String Day = (getValue(response, ' ', 1));
  //swSerial.println("Day:" + Day);
  //
  String Month = (getValue(response, ' ', 2));
  //swSerial.println("Month:" + Month);
  //
  String Year = (getValue(response, ' ', 3));
  //swSerial.println("Year:" + Year);
  //
  //
  String timezone = (getValue(response, ' ', 5));
  timezone.trim();
  // swSerial.println("timezone:" + timezone);
  //
  response = (getValue(response, ' ', 4));
  String Hour = (getValue(response, ':', 0));
  //swSerial.println("Hour:" + Hour);
  //
  String Minute = (getValue(response, ':', 1));
  //swSerial.println("Minute:" + Minute);
  //
  String Second = (getValue(response, ':', 2));
  //swSerial.println("Second:" + Second);

  byte bHour = Hour.toInt();
  byte bMinute = Minute.toInt();
  byte bSecond = Second.toInt();
  byte bDay = Day.toInt();
  byte bMonth = Month.toInt();
  byte bYear = Year.toInt();

  setTime(bHour, bMinute, bSecond, bDay, bMonth, bYear);
  adjustTime(offset * SECS_PER_HOUR);
  String shortYear = String(year()).substring(2, 4);
  // DS3231 seconds, minutes, hours, day, date, month, year Sunday=1
  setDS3231time(second(), minute(), hour(), weekday(), day(), month(), shortYear.toInt());
  //
  return 1;
}
//
//
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {
    0, -1
  };
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) :"";
}
//
void displayYahooQuoteWrapper() {
  int quoteDelay = 1000;
  displayYahooQuote("AUDHKD=X", 2);
  delay(quoteDelay);
  displayYahooQuote("BTCUSD=X", 2);
  delay(quoteDelay);
  displayYahooQuote("0005.HK", 1); // type 1 stock, type 2 forex
  delay(quoteDelay);
  displayYahooQuote("CNYHKD=X", 2);
  delay(quoteDelay);
  displayYahooQuote("4222.HK", 1);
  delay(quoteDelay);
  displayYahooQuote("0268.HK", 1);
  delay(quoteDelay);
  displayYahooQuote("3085.HK", 1);
  delay(quoteDelay);
  displayYahooQuote("EURHKD=X", 2);
  delay(quoteDelay * 2);

}
//
boolean displayYahooQuote(String symbol, int type) // type 1 stock, 2 forex
{
  pause = true;
  command = ("loadfile\(\"yahooQuoteRetrieval.lua\"\)\(\"" + symbol + "\"\)");
  sendData(command, 5000, true, 200);
  command = "";
  response = returnAlphaNum(response);
  /*
    for (int i = 0; i < 30 ; i++) {
      swSerial.println(String(i) + ":" + getValue(response, ',', i));
    };

    for (int i = 0; i < 30 ; i++) {
      swSerial.println(String(i) + ":" + getValue(response, '"', i));
    };

  //
  */
  String LCDoutput = "";
  String  instrumentName = "";
  String  instrumentSymbol = "";
  String percentageChange = "";

  instrumentName = (getValue(response, '\"', 10));
  //instrumentSymbol = (getValue(response, '\"', 3));
  String last = (getValue(response, ',', 2));
  String prevClose = (getValue(response, ',', 4));
  //  String todayOpen = (getValue(response, ',', 3)) ;
  percentageChange = (getValue(response, '\"', 12));

  //char direction = '=';
  //if (last.toFloat() > prevClose.toFloat() ) direction = '^';
  //if (last.toFloat() < prevClose.toFloat() ) direction = 'v';

  switch (type)
  {
    case 1: // stock
      LCDoutput = symbol + " " + instrumentName;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(LCDoutput);
      LCDoutput = last +  " " + percentageChange;
      lcd.setCursor(0, 1);
      lcd.print(LCDoutput);
      break;

    case 2: // forex
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(instrumentName);

      LCDoutput = "1   to " + last;
      lcd.setCursor(0, 1);
      lcd.print(LCDoutput);
      break;
  }
  LCDoutput = "";
  instrumentName = "";
  instrumentSymbol = "";
  pause = false;
  return true;
}
//
String returnAlphaNum (String data)
{
  pause = true;
  /*
  returns only the AlphaNumeric compononent of data
  http://www-control.eng.cam.ac.uk/~pcr20/www.cppreference.com/stdstring_details.html
  sample:
    Test Original:String Test      TestEnd
    Test Cleaned:StringTestTestEnd
  */
  String alphaNumReturn = "";
  char previousCharacter = ' ';
  char currentCharacter = '~';
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= 200; i++) {
    currentCharacter = data.charAt(i);
    if (isprint(currentCharacter)) {
      alphaNumReturn += currentCharacter;
    }
  }
  pause = false;
  return alphaNumReturn;
}
//
// EOF

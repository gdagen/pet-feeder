/* Cat Feeder v1.3 Created by Gabe Mulford 
 * Automatically feeds pet every day at user-determined times (breakfast, lunch, dinner or any combination)
 * Also has a manual push button switch for manually activating feeder outside of scheduled times.
 * 
 * Changelog:
 * 2018-12-14 Added oLED display to keep track to time and alarm
 * 2019-05-05 Added myservo.detach() function to disable servo when not using.  Will remove hum 
 *           (and power consumption) from servo.
 * 2019-05-06 Added scheduling function with setup button, eeprom read/write and made code more systematic and easier to update pins.
*/

// Clock functions
#include <Time.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
byte lastAlarm;

// EEPROM for storing schedule
#include <EEPROM.h>
int eeprom_address = 0;    // start reading from the first byte (address 0) of the EEPROM
byte eeprom_value;

/*
 * EEPROM can only hold a value from 0 to 255 in one address.  EEPROM has also a limit of 100,000 write cycles per single location (address)
 * 
 * Read example
 * eeprom_value = EEPROM.read(eeprom_address); 
 * 
 * Write example
 * EEPROM.write(eeprom_address, eeprom_value);
 * 
 * Update example (will not write if value is the same) <-- better solution
 * EEPROM.update(eeprom_address, eeprom_value);
 * 
*/

// oLED display
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(4);  // The constructor of this class takes the port number at which the display could be reset which is pin 4 (connected to SCK)

// Servo
#include <Servo.h>
Servo myservo;  // create servo object to control a servo
byte pos = 0;    // variable to store the servo position

// Define pins (update these if necessary)
#define SERVO_PIN 6
#define FEED_BUTTON 9
#define SCHED_BUTTON 8

// Define times (set these using military time)
#define BREAKFAST_HOUR 7
#define BREAKFAST_MIN  30
#define LUNCH_HOUR 12
#define LUNCH_MIN  00
// Dinner 17,30 = 5:30pm
#define DINNER_HOUR 17
#define DINNER_MIN  30

// for determining if in set schedule mode
boolean schedActive;
byte setButton = 0;
int activeTime;

void setup() {

  //start serial connection
  //Serial.begin(9600);

  // initialize OLED screen
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // oLED initialize with the I2C addr 0x3C (for the 128x64)
  display.display();  // display the splashscreen
  delay(3000);
  display.setTextColor(WHITE);  // set color for all text.
  
  // initialize servo
  myservo.attach(SERVO_PIN);    // attaches the servo on pin 6 to the servo object
  myservo.write(0);             // initialize servo to start position.  Maximum is 170
  delay(1000);                  // wait for motor to complete turn before backing off
  myservo.write(10);            // back off to prevent straining of servo motor
  myservo.detach();             // disconnect servo to prevent hum/straining 

  // initialize time
  setTime(11,59,50,1,1,19);     // set time to Saturday 12:00:00pm Jan 1, 2019
  lastAlarm = 1; // last alarm was breakfast

  // get stored schedule
  eeprom_value = EEPROM.read(eeprom_address);

  // if there is something stored, set the alarm(s)
  if (eeprom_value > 0 && eeprom_value < 8) {
    if (eeprom_value == 1) {
      Alarm.alarmRepeat(BREAKFAST_HOUR,BREAKFAST_MIN,0, Breakfast);
    } else if (eeprom_value == 2) {
      Alarm.alarmRepeat(LUNCH_HOUR,LUNCH_MIN,0, Lunch);
    } else if (eeprom_value == 3) {
      Alarm.alarmRepeat(DINNER_HOUR,DINNER_MIN,0, Dinner);
    } else if (eeprom_value == 4) {
      Alarm.alarmRepeat(BREAKFAST_HOUR,BREAKFAST_MIN,0, Breakfast);
      Alarm.alarmRepeat(LUNCH_HOUR,LUNCH_MIN,0, Lunch);
    } else if (eeprom_value == 5) {
      Alarm.alarmRepeat(BREAKFAST_HOUR,BREAKFAST_MIN,0, Breakfast);
      Alarm.alarmRepeat(DINNER_HOUR,DINNER_MIN,10, Dinner);
    } else if (eeprom_value == 6) {
      Alarm.alarmRepeat(LUNCH_HOUR,LUNCH_MIN,0, Lunch);
      Alarm.alarmRepeat(DINNER_HOUR,DINNER_MIN,0, Dinner);
    } else if (eeprom_value == 7) {
      Alarm.alarmRepeat(BREAKFAST_HOUR,BREAKFAST_MIN,0, Breakfast);
      Alarm.alarmRepeat(LUNCH_HOUR,LUNCH_MIN,0, Lunch);
      Alarm.alarmRepeat(DINNER_HOUR,DINNER_MIN,0, Dinner);
    } 
  }
    
  pinMode(FEED_BUTTON, INPUT_PULLUP);  // initialize FEED button
  pinMode(SCHED_BUTTON, INPUT_PULLUP);  // initialize FEED button
  
}

void loop() {

  // check if "setup" button pressed
  boolean schedButton = digitalRead(SCHED_BUTTON);
  if (schedButton == LOW) {
    schedActive = true;
    setButton++;
    if (setButton > 8) {setButton = 1;}
    activeTime = millis();
    delay(500);
  }

  // Start set schedule mode
  if (schedActive) {

    display.clearDisplay();
    display.setTextSize(1);
  
    if (setButton == 1 || setButton == 4 || setButton == 5 || setButton == 7) {
      display.setCursor( 0,0 );
      display.println(F("BFAST"));
    }
  
    if (setButton == 2 || setButton == 4 || setButton == 6 || setButton == 7) {
      display.setCursor( 49,0 );
      display.println(F("LUNCH"));
    }
  
    if (setButton == 3 || setButton == 5 || setButton == 6 || setButton == 7) {
      display.setCursor( 100,0 ); 
      display.println(F("DINNER"));
    }
    if (setButton == 8) {
      display.setCursor( 28,0 ); 
      display.println(F("SCHEDULE OFF"));
    }

    delay(500); // wait a half second
    display.display();  // send to screen
    delay(500); // wait a half second

    // it's been 5 seconds, save the last entry and reset
    if ((millis() - activeTime) > 6000) {
      EEPROM.update(eeprom_address, setButton);
      eeprom_value = setButton;
      schedActive = false;
      display.clearDisplay();    // Clear the display buffer.
      display.setTextSize(3);
      display.setCursor(10,15);   // x,y - y=15 is the bottom of the yellow portion
      display.println(F("SAVING"));
      delay(4000);
      resetFunc();  //call reset
    
  } else {
    // not in setup mode, run main program
      
  
    // Clear the display buffer.
    display.clearDisplay();
  
    display.setTextSize(1);
    
    if (eeprom_value == 1 || eeprom_value == 4 || eeprom_value == 5) {
      display.setCursor( 0,0 );
      display.println(F("BFAST"));
    }
  
    if (eeprom_value == 2 || eeprom_value == 4 || eeprom_value == 5) {
      display.setCursor( 49,0 );
      display.println(F("LUNCH"));
    }
  
    if (eeprom_value == 3 || eeprom_value == 5 || eeprom_value == 6) {
      display.setCursor( 100,0 ); 
      display.println(F("DINNER"));
    }
  
    // Current Time display
    display.setTextSize(4);
    display.setCursor(1,15); // x,y - y=15 is the bottom of the yellow portion
    digitalClockDisplay();
  
    // next feeding time display
    display.setTextSize(2);
    display.setCursor( 0, 49 );
    display.println(F("FEED "));
    if (lastAlarm == 1) {
      display.print(LUNCH_HOUR + ":" + LUNCH_MIN);
    } else if (lastAlarm == 2) {
      display.print(DINNER_HOUR + ":" + DINNER_MIN);
    } else {
      display.print(BREAKFAST_HOUR + ":" + BREAKFAST_MIN);
    }
    
  
    display.display();  // send to screen
      
    Alarm.delay(0); // needed to monitor alarm(s)
  
    boolean feedButton = digitalRead(FEED_BUTTON);
      
    // Manual Feeding Time
    if (feedButton == LOW) {
      display.clearDisplay();    // Clear the display buffer.
      display.setTextSize(1);
      display.setCursor( 64 - (17 * 3), 0 );  // center word
      display.println(F("WHO'S A GOOD BOY"));
      display.setTextSize(3);
      display.setCursor(20,15);   // x,y - y=15 is the bottom of the yellow portion
      display.println(F("SNACK"));
      display.setCursor(20,41);
      display.println(F("TIME!")); 
      display.display();
      FeedingTime();
      delay(500);
    }

  } // end not in setup mode, run program
}

// functions to be called when an alarm triggers:

void Breakfast() {
  display.clearDisplay();    // Clear the display buffer.
  display.setTextSize(3);
  display.setCursor(12,17); // x,y - y=15 is the bottom of the yellow portion
  display.println(F("BFAST!"));
  display.display();
  FeedingTime();
  delay(1000);
  lastAlarm = 1;
}

void Lunch() {
  display.clearDisplay();    // Clear the display buffer.
  display.setTextSize(3);
  display.setCursor(12,17); // x,y - y=15 is the bottom of the yellow portion
  display.println(F("LUNCH!"));
  display.display();
  FeedingTime();
  delay(1000);
  lastAlarm = 2;
}
void Dinner() {
  display.clearDisplay();    // Clear the display buffer.
  display.setTextSize(3);
  display.setCursor(12,17); // x,y - y=15 is the bottom of the yellow portion
  display.println(F("DINNER"));
  display.display();
  FeedingTime();
  delay(1000);
  lastAlarm = 3;
}

void FeedingTime() {
    myservo.attach(SERVO_PIN);     // attaches the servo on pin 6 to the servo object
    if (pos == 0) {
      myservo.write(140);  // adjust these for portion amount
      delay(1000);
      myservo.write(130);  // back off motor (should be -10 from above myservo.write value)
      pos = 130;
    } else {
      myservo.write(0);
      pos = 0;
      delay(1000);
      myservo.write(10);  // back off motor a bit
    }
    myservo.detach();     // turn off server to prevent humming/straining
}

void digitalClockDisplay() {
  display.print(hour());
  displayDigits(minute());
  //displayDigits(second());
}

void displayDigits(int digits) {
  display.print(F(":"));
  if (digits < 10)
    display.print('0');
  display.print(digits);
}

void(* resetFunc) (void) = 0;  //declare reset function at address 0






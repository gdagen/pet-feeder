Cat Feeder v1.3 Created by Gabe Dagen
Automatically feeds pet every day at user-determined times (breakfast, lunch, dinner or any combination)
Also has a manual push button switch for manually activating feeder outside of scheduled times.

Materials needed
----------------
Arduino uno or nano microcontroller
Cereal or dry dispenser
Basic servo
"L" bracket
zip ties or adjustable wormdrive pipe fitting 
oLED 128x64 ".96 l2c
2 SPST buttons
Wires
Enclosure
DC power mount 
Protoboard
power supply (9v 1A)


Changelog:
2018-12-14 Added oLED display to keep track to time and alarm
2019-05-05 Added myservo.detach() function to disable servo when not using.  Will remove hum 
   (and power consumption) from servo.
2019-05-06 Added scheduling function with setup button, eeprom read/write and made code more systematic and easier to update pins.

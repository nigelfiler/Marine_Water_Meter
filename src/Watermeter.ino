

/*
 
********* Initial setup *********

When setting up connect a 100 Ohm resistor in place of the waste and water tank senders. 
Adjust the 2 potentiometers until the readings for the tanks are 55% - you will then be close for when you connect up the senders, which you should test on the bench beforehand.
Connect water and waste float senders on the appropriate terminals. Polarity not important.
You will need a reset switch. In my case there was an existing button on the switch panel that I repurposed. 

 ********* Parts list **************
 VR1 - 500 Ohm multi-turn pot (Water adjustment) Adjust so you get 100% when sender is at the top and make sure you get 0% at bottom.
 VR2 - 500 Ohm multi-turn pot (Waste adjustment) Adjust so you get 100% when sender is at the top and make sure you get 0% at bottom. 
 LED1 & LED2 - Optional - LED's dont do anything at the moment but are programmable via D11 & D12. Eg: turn LED on when waste is getting above 70%
 R1 - 220 Ohm (Optional for LED)
 R2 - 220 Ohm (Optional for LED)
 R3 - 1K (For switching relay)
 R4 - 47K (Voltage divider)
 R5 - 10K (Voltage divider)
 R6, R9, R10 - 10K
 D1 - 1N4007
 D2 - 1N4007
 Q1 - BC337 (For switching relay)
 C1, C2 - 0.1uF, for smoothing the regulator. Exact value not too important
 C4,C5,C6 - 0.1uF, for debouncing rotary encoder. Exact value not too important
 Fuse - Resetable 1A fuse. ebay - PPTC radial lead polyswitch. 1A.
 Arduino - Use "Ardunio Every" this model has more memory than a Nano.
 DC-DC: 12v->9V regulator.  Arduino needs 7v minimum to correctly provide 5v on the 5v pin
 Display - 1.3" OLED eg: https://www.ebay.co.uk/itm/203653375472?var=504969017093 Any other brand of OLED is fine. 1.3" fits the panel cover. Uses the SH1106 driver.
 Display wiring: PCB: Pin1=VCC, Pin2=0v, Pin3=SCL, Pin4=n/c, Pin5=SDA. You need to make a suitable cable. Pin4 is not connected and is designed to be a 'key' so the connector can't be put on incorrectly.
 The cable is called a dupont cable. Order a 5way female to female and modify to suit.
 Voltage Meter 0-15 V DC Analog Panel VoltMeter - only needed for the case to mount the OLED: https://www.ebay.co.uk/itm/166528946897 buy this and pull out the innards.
 Relay - OMRON_G5V-2. 5v
 Thermistor - For hot water tank. Need to measure resistance at 3 different temeratures and enter into this calculator: https://www.thinksrs.com/downloads/programs/therm%20calc/ntccalibrator/ntccalculator.html
 Thermistor - Enter values from website into line 62-64. 
 Thermistor - connect 2 leads on water temp terminal. Orientation not important.
 Header pins for Arduino, Display, Rotary encoder, etc: 2.54mm Breakaway Male Header for PCB. Cut to length required.
 For connecting the sensors and power: KF128 2.54mm Pitch Mini Terminal Block. 1x6P & 1x8P.
 2x 100 ohm resistors for initial setup.

 ********* End Parts list **************

*/

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <avr/sleep.h>

/*
 Test your thermistor at different temeratures to obtain the resistance
 enter resistance into Steinhart-Hart Equation to obtain values

 Aliexpress black 2 pin thermister

 9c = 18.17k -0.9002845230e-3
 20c = 11.39k 5.437476163e-4
 59c = 3.36k -9.416539500e-7
*/

double c1 = -0.9002845230E-03;  //Steinhart-Hart Equation coefficients.
double c2 = 5.437476163E-04;
double c3 = -9.416539500E-07;

double tankTemperature = 0;
double logRt, Rt;
int Vo;            // Integer value of voltage reading
double R = 10000;  // Fixed resistance in the voltage divider R9


const int WATER_SENSOR_PIN = A0;
const int WASTE_SENSOR_PIN = A1;
int sensorTankValue0 = 0;
int sensorTankValue1 = 0;
const int SENSOR_THRESHOLD = 10; // When checking for missing float sensors
const int SENSOR_READING_MIN = 328;
const int SENSOR_READING_MAX = 1023;
const int SCREEN_UPDATE_INTERVAL = 10;
//const int SCREEN_OFF_DELAY = 60000;  // 60000 = 1 minute in milliseconds
int RELAY_PIN = 9;                     //Pin 9 to control relay.
char DEGREE_SYMBOL[] = { 0xB0, '\0' };
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

int VoltagePin = A6;
float vout = 0.0;
float vin = 0.0;

float R4 = 47000.0; // 
float R5 = 10000.0; // 
int value = 0;

int waterTankValue = 0;
int wasteTankValue = 0;
int tankConfig = 0;
int runTime = 20; // 20 Minutes before screen shuts off
int tankUpdateInterval = 15; //15 seconds between tank percentage updates
int sumpUpdateInterval = 60; //60 seconds between shower sump checks
unsigned long previousTankUpdateTime = 0;
unsigned long previousSumpUpdateTime = 0;

unsigned long relayCloseTime = 0;
const unsigned long RELAY_SETTLE_DELAY = 1500;  // 1.5 second delay
unsigned long previousMillis = 0;
const unsigned long interval = 20;


void setup() {
 
  Serial.begin(9600);
  u8g2.begin();
  pinMode(VoltagePin, INPUT); //Voltage reading pin
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  readSupplyVoltage();
  digitalWrite(RELAY_PIN, HIGH);
  relayCloseTime = millis(); 

    while (millis() - relayCloseTime < RELAY_SETTLE_DELAY) {
        /*
        Need to pause since relay has just closed and sensor will not be read properly without the delay. 
        Delay also allows voltage to be displayed for a short time before being overwritten
        */
    }
  checkWaterSensors();
  updateWaste();
  startupCheckShowerSump();
  updateHotWaterTemp();
  updateWater();  //Needed for the first run otherwise main screen is delayed by update period.
}

void loop() {
  // Check if it's time to check the tank
  if (millis() - previousTankUpdateTime >= tankUpdateInterval * 1000UL) {
    updateHotWaterTemp();
    updateWater();
    previousTankUpdateTime = millis();
  }

  // Check if it's time to check the shower sump
  if (millis() - previousSumpUpdateTime >= sumpUpdateInterval * 1000UL) {
    checkShowerSump();
    previousSumpUpdateTime = millis();
  }

  // Check if it's time to turn off the screen
  if (millis() > runTime * 60000UL) {
    sleep();
  }
}


double updateHotWaterTemp() {

  static bool firstReadDone = false;
    if (!firstReadDone) {
        Vo = 1024 - analogRead(A2); // First read to stabilize
        previousMillis = millis();
        firstReadDone = true;
    } 

    if (millis() - previousMillis >= interval) {
        Vo = 1024 - analogRead(A2); 
        firstReadDone = false;  // Reset for next call
    }

  //Serial.println(Vo);         // Read thermister voltage
  Rt = R * (1024 / (float)Vo - 1.0);  // Calculate thermister resistance
  logRt = log(Rt);
  tankTemperature = (1.0 / (c1 + c2 * logRt + c3 * logRt * logRt * logRt));  // Apply Steinhart-Hart equation.
  return tankTemperature;                                                    
}

// To check if the sensors are connected upon startup.
void checkWaterSensors(){
  sensorTankValue0 = analogRead(WATER_SENSOR_PIN);
  sensorTankValue1 = analogRead(WASTE_SENSOR_PIN);

  bool waterSensorNotConnected = (sensorTankValue0 < SENSOR_THRESHOLD);
  bool wasteSensorNotConnected = (sensorTankValue1 < SENSOR_THRESHOLD);

  if (waterSensorNotConnected) {
    displayMessage("Water Sensor");
  }

  if (wasteSensorNotConnected) {
    displayMessage("Waste Sensor");
  }

 if (waterSensorNotConnected || wasteSensorNotConnected) {
    delay(2000);  // Optional final delay before resuming normal operation
  }


}

void displayMessage(const char* sensorType) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_helvB12_tf);
  u8g2.setCursor(10, 25);
  u8g2.print(sensorType);
  u8g2.setCursor(2, 55);
  u8g2.print("Not Connected");
  u8g2.sendBuffer();
  delay(3000); // fix blocking code
}


void checkShowerSump() {
  pinMode(A3, INPUT_PULLUP);
  if (!digitalRead(A3)) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_helvB14_tf);
    u8g2.setCursor(2, 25);
    u8g2.print("Shower Sump");
    u8g2.setFont(u8g2_font_helvB18_tf);
    u8g2.setCursor(35, 55);
    u8g2.print("FULL");
    u8g2.sendBuffer();
    delay(5000);  //If sump is full display message for 5 seconds. Fix blocking code.
  }
}

void startupCheckShowerSump() {
  pinMode(A3, INPUT_PULLUP);
  if (!digitalRead(A3)) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_helvB14_tf);
    u8g2.setCursor(2, 25);
    u8g2.print("Shower Sump");
    u8g2.setFont(u8g2_font_helvB18_tf);
    u8g2.setCursor(35, 55);
    u8g2.print("FULL");
    u8g2.sendBuffer();
    delay(5000);  //If sump is full display message for 5 seconds. Fix blocking code
  }
  else
  {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_helvB14_tf);
    u8g2.setCursor(2, 25);
    u8g2.print("Shower Sump");
    u8g2.setFont(u8g2_font_helvB18_tf);
    u8g2.setCursor(45, 55);
    u8g2.print("OK");
    u8g2.sendBuffer();
    delay(2000); // Fix blocking code
  }
}

void readSupplyVoltage(){
    
    static bool firstReadDone = false;
    
    if (!firstReadDone) {
        value = analogRead(VoltagePin);// First read to stabilize
        previousMillis = millis();
        firstReadDone = true;
    } 

    if (millis() - previousMillis >= interval) {
        value = analogRead(VoltagePin);
        firstReadDone = false;  // Reset for next call
    }

    vout = (value * 4.9) / 1024.0;
    vin = vout / (R5/(R4+R5)); 
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_helvB12_tf);
    u8g2.setCursor(5, 25);
    u8g2.print("Supply Voltage");
    u8g2.setFont(u8g2_font_helvB18_tf);
    u8g2.setCursor(34, 55);
    u8g2.print(vin,1);
    u8g2.print(" v");
    u8g2.sendBuffer();
}


void updateWaste() {
  static bool firstReadDone = false;
      if (!firstReadDone) {
        sensorTankValue1 = analogRead(WASTE_SENSOR_PIN);// First read to stabilise
        previousMillis = millis();
        firstReadDone = true;
    }

      if (millis() - previousMillis >= interval) {
        sensorTankValue1 = analogRead(WASTE_SENSOR_PIN);
        firstReadDone = false;  // Reset for next call
    }

  wasteTankValue = map(sensorTankValue1, SENSOR_READING_MAX, SENSOR_READING_MIN, 0, 100);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_helvB14_tf);
  u8g2.setCursor(10, 25);
  u8g2.print("Waste Tank");
  //Serial.println(wasteTankValue);
  if(wasteTankValue < 5)
  {
   displayTankValue(wasteTankValue, 50, 55); // Centralise single digit on the screen
  }
  else
  {
  displayTankValue(wasteTankValue, 40, 55);
  }
  u8g2.sendBuffer();
  delay(2000);  //Displays waste tank value for 2 seconds at power up - Fix blocking code
  if(wasteTankValue >75){
//Turn LED on - LED on main distribution board could be used as a warning light

  }
}


void updateWater() {

  static bool firstReadDone = false;
      if (!firstReadDone) {
        sensorTankValue0 = analogRead(WATER_SENSOR_PIN);// First read to stabilise
        previousMillis = millis();
        firstReadDone = true;
    }

      if (millis() - previousMillis >= interval) {
        sensorTankValue0 = analogRead(WATER_SENSOR_PIN);
        firstReadDone = false;  // Reset for next call
    }

  int temp(tankTemperature);
  waterTankValue = map(sensorTankValue0, SENSOR_READING_MAX, SENSOR_READING_MIN, 0, 100);
  u8g2.clearBuffer();
  u8g2.setCursor(1, 8);
  u8g2.setFont(u8g2_font_helvB08_tf);
  u8g2.print("Temp: 18c"); //Temporary placeholder for future cabin temp coding
  u8g2.setCursor(80, 8);
  u8g2.print("RH: 100%"); //Temporary placeholder for future cabin humidity coding
  u8g2.setFont(u8g2_font_helvB14_tf);
  u8g2.setCursor(7, 34);
  u8g2.print("Fresh Water");
  u8g2.setFont(u8g2_font_helvB24_tf);
  displayTankValue(waterTankValue, 5, 62);
  u8g2.setCursor(75, 62);
  temp = (tankTemperature - 273.15);

  if (temp < -10) {
    u8g2.print("---");  // If sensor is disconnected the display will show -273 which does not format well on a small screen.
                        //This prevents that by displaying --- instead. Dont expect to see -10 on the water tank anyway.
  } else {
    u8g2.print(temp);
    u8g2.setFont(u8g2_font_ncenB14_tf);

    if (temp < 10) {
      u8g2.drawUTF8(88, 56, DEGREE_SYMBOL);  // Aligns the degree symbol with a single digit shown
    } else {
      u8g2.drawUTF8(102, 56, DEGREE_SYMBOL);
    }
  }
  u8g2.setFont(u8g2_font_helvB14_tf);
  u8g2.print(" C");
  u8g2.sendBuffer();
}


void displayTankValue(int tankValue, int x, int y) {
  int percentage = 0;

  if (tankValue < 16) {
    percentage = 0;
  } else if (tankValue < 18) {
    percentage = 5;
  } else if (tankValue < 31) {
    percentage = 10;
  } else if (tankValue < 42) {
    percentage = 15;
  } else if (tankValue < 48) {
    percentage = 20;
  } else if (tankValue < 52) {
    percentage = 25;
  } else if (tankValue < 57) {
    percentage = 30;
  } else if (tankValue < 61) {
    percentage = 35;
  } else if (tankValue < 64) {
    percentage = 40;
  } else if (tankValue < 68) {
    percentage = 45;
  } else if (tankValue < 72) {
    percentage = 50;
  } else if (tankValue < 75) {
    percentage = 55;
  } else if (tankValue < 78) {
    percentage = 60;
  } else if (tankValue < 80) {
    percentage = 65;
  } else if (tankValue < 83) {
    percentage = 70;
  } else if (tankValue < 85) {
    percentage = 75;
  } else if (tankValue < 87) {
    percentage = 80;
  } else if (tankValue < 89) {
    percentage = 85;
  } else if (tankValue < 93) {
    percentage = 90;
  } else if (tankValue < 96) {
    percentage = 95;
  } else if (tankValue < 140) {
    percentage = 100;
  }

  u8g2.setFont(u8g2_font_helvB18_tr);
  u8g2.setCursor(x, y);
  u8g2.print(percentage);
  u8g2.print("%");
}

// Standby mode to save power
void sleep() {
  digitalWrite(RELAY_PIN, LOW);
  u8g2.setPowerSave(1);  // Turn off screen
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_mode();  // Go to sleep
}

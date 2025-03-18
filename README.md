# Marine Water Meter

## Project Overview
A project to replace the existing analog water gauge on a Sealine S28 with a digital version based on Arduino.

The domestic fresh water gauge on the boat is not the most accurate, nor is it very user-friendly. This project improves upon it by incorporating a digital display with additional measurements such as:
- Water temperature
- Black tank level
- Shower Sump monitoring

An OLED display fits where the existing analog meter is installed, and an Arduino is mounted behind the electrical panel.

## Features
- **Power-up sequence:**
  - Checks supply voltage
  - Verifies shower sump status
  - Displays black tank level for 5 seconds
  - Shows fresh water tank level and domestic hot water temperature
- **Tank level updates every 5 seconds** to prevent value from fluctuating quickly due to boat movement
- **Power-saving mode:**
  - Board sleeps after a defined timeout to save power
  - Reduces light distraction at night
- **Temperature monitoring:**
  - Thermistor attached to the hot water cylinder surface relays temperature

## Power Consumption
- **Active mode:** 81mA
- **Sleep mode:** 16mA
- **Power input:** 7 - 32V DC (depends on regulator installed) Typically 12-14v would be the normal voltage on the boat.

## Sensors & Functionality
### Waste & Water Tank
- Uses a **0-190 Ohm water level sensor** of appropriate length for the tank
- Code calculates tank level based on resistance and displays a percentage value

### Water Temperature
- Thermistor adhered to the hot water cylinder surface calculates the temperature

### Shower Sump Monitoring
- Shower sump boxes can fail, leading to overflow into the bilge
- A **magnetic float switch** is added to the sump case
- The system monitors the switch and displays:
  - `Shower Sump FULL`
  - `Shower Sump OK`

## User Interaction
- The board can be **woken up using the existing push button** on the current water gauge
- Upon wake-up, all readings are displayed before switching to fresh tank level and hot water temperature

## Additional Considerations
- The project uses a **0-190 Ohm water level gauge sensor**
- Installation requires drilling into the black tank and water tank

## Future Ideas
- Rotary encoder can be used with the board to select other functions
- Could be used to monitor fridge temperatures
- Air quality monitor

## Video Showing Operation
  https://www.youtube.com/shorts/WlagKgHyAQc




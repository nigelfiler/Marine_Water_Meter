# Marine_Water_Meter
Project to replace existing analog water gauge with digital version based on Arduino

The fresh water gauge on the boat is not the most accurate, nor is it very user friendly, I felt this could be improved. This project is a digital replacement so I built a replacement using Arduino.

There is an Oled display that fits where the existing analogue meter is fitted and an Arduino that is fitted behind the electrical panel.


On power up the board checks the supply voltage, checks the shower sump is not full, displays the black tank level for 5 seconds and finally displays the fresh water tank level and the domestic hot water temperature on the domestic supply.

The fresh water tank is updated every 5 seconds, this allows for boat movement without the level flickering between values.

When the board reaches a defined power down time it will sleep, this will save power and light distraction at night from the display.

A thermistor attached to the outer surface of the hot water cylinder will relay the temperature of the hot water.


Power consumption: 81ma (Sleep:16mA) - based on both water sensors at 0%

Power input: 7 - 32v DC - Depending on the regulator installed.

Uses existing 0-190 ohm water level gauge sensor. I need to fit a suitable one to my black tank and need to drill the tank so if anyone has any suggestions on what to use for this, it would be appreciated.

User can wake the board using the existing push button on the current water gauge at which point all the readings will be displayed before showing the fresh tank level / Hot water temperature.

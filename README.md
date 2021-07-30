FujiHeatPump
----------
Allows Arduino boards to communicate with Fujitsu AirConditioners (Heatpumps) via the 3-wire remote interface or RX/TX lines on the indoor unit. Supports acting as a Primary or Secondary controller.

Using the 3-wire interface requires a LIN tranceiver (such as MCP2025-330) or building your own. Supports running as a Primary or Secondary controller.

Quick start
-----------

#### Controlling the heat pump

```c++
#include "FujiHeatPump.h"

FujiHeatPump hp;

void setup() {
  hp.connect(&Serial2, true);
}

void loop() {
  hp.waitForFrame();     // attempt to read state from bus and place a reply frame in the buffer
  hp.sendPendingFrame(); // send any frame waiting in the buffer
  
  //do an update
  if(weWantToUpdate) {
    hp.setOnOff(true);
    hp.setMode(static_cast<byte>(FujiMode::COOL));
    hp.setFanMode(4);
    hp.setTemp(18);
  }
  
  //get the current status
  hp.getOnOff();
  hp.getMode();
  hp.getFanMode();
  hp.getTemp();
}

```


Installation
------------
- Clone repository
- Move contents into Arduino library directory
- Restart IDE

Notes
-----
- Tested with ESP32
- Tested with TeensyLC/Teensy3.5

Parts Required
--------------

- LIN Tranceiver
  - MCP-2025-330 (or similar)
  - http://ww1.microchip.com/downloads/en/DeviceDoc/20002306B.pdf
  
========================================  
Special thanks to Jaroslaw Przybylowicz, who decoded the protocol initially (https://github.com/jaroslawprzybylowicz/fuji-iot)


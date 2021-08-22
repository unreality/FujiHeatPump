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
  hp.connect(&Serial2, true); // use Serial2, and bind as a secondary controller
}

void loop() {
  if(hp.waitForFrame()) {   // attempt to read state from bus and place a reply frame in the buffer
    delay(60);              // frames should be sent 50-60ms after recieving - potentially other work can be done here
    hp.sendPendingFrame();  // send any frame waiting in the buffer
  }
  
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

See the [API Reference](https://github.com/unreality/FujiHeatPump/blob/master/docs/Reference.md) for more details.

Installation
------------
- Clone repository
- Move contents into Arduino library directory
- Restart IDE

Secondary Mode Notes
--------------------

This library supports connecting as a primary or secondary remote control. If using it in secondary mode, there are some timing requirements that you need to be aware of. The primary controller will only send ONE frame to check whether there is a secondary controller. This occurs approx **4 seconds** after the unit is switched on. If this frame is missed, you cannot join the bus as a secondary controller. 

Example Circuit
---------------
<img src="https://github.com/unreality/FujiHeatPump/blob/master/fujiheatpump-example.png"/>


Notes
-----
- Tested with ESP32
- Tested with TeensyLC/Teensy3.5

Further Work
------------
- Errors are sent by the AC in various states, these are currently ignored.
- Several bits in messages serve unknown purposes.

Parts Required
--------------

- LIN Tranceiver
  - MCP-2025-330 (or similar)
  - http://ww1.microchip.com/downloads/en/DeviceDoc/20002306B.pdf
  
========================================  
Special thanks to Jaroslaw Przybylowicz, who decoded the protocol initially (https://github.com/jaroslawprzybylowicz/fuji-iot)


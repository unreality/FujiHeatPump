# FujiHeatPump API Reference

The FujiHeatPump Library is invoked by including *FujiHeatPump.h* in your Arduino sketch as follows:

```C++
#include "FujiHeatPump.h"
```

## *FujiHeatPump*

The FujiHeatPump object is used to control the Fujitsu Airconditioner, it has the following methods:

* `void connect(HardwareSerial *serial, bool secondary);` 
  * initialises the serial connection to the heatpump
  * **must** be called before using other methods
  * arguments
    * *serial* - The serial port to use for communication
    * *secondary* - Whether to bind to the AC as a secondary controller.
  * example: `hp.connect(&Serial2, true);`
 
 * `bool waitForFrame();`
   * Checks the serial line for any frame, decodes the frame and places the current state of the AC in the buffer.
   * Constructs a reply frame, but does not send it yet.
   * **must** be called repeatedly in each sketch and is typically placed at the top of the Arduino `loop()` method.
 * `bool sendPendingFrame();`
   * Checks whether 50ms has passed since waitForFrame() decoded its last frame, and sends the buffered frame if it has.
   * **must** be called repeatedly in each sketch and is typically placed at the end of the Arduino `loop()` method.

 * `bool isBound();`
   * Returns whether the FujiHeatPump object is 'bound' to an AC unit and can send/receive statuses.
 * `bool updatePending();`
   * Returns whether a pending **change** has been sent 
     
 * `void setOnOff(bool o);`
   * Sets on/off for the AC
 * `void setTemp(byte t);`
   * Sets target temperature for the AC
 * `void setMode(byte m);`
   * Sets mode (FAN, DRY, COOL, HEAT, AUTO) for the AC
   * FujiMode enum is available for values
   * example: `hp.setMode(static_cast<byte>(FujiMode::COOL));` 
 * `void setFanMode(byte fm);`
   * Sets fan mode (FAN_QUIET, FAN_LOW, FAN_MEDIUM, FAN_HIGH, FAN_AUTO) for the AC
   * FujiFanMode enum is available for values
   * example: `hp.setFanMode(static_cast<byte>(FujiFanMode::FAN_LOW));` 
 * `void setEconomyMode(byte em);`
   * Sets economy mode for the AC
 * `void setSwingMode(byte sm);`
   * Sets swing mode for the AC
 * `void setSwingStep(byte ss);`
   * Sets swing mode for the AC
    
 * `bool getOnOff();`
   * Get current on/off
 * `byte getTemp();`
   * Get current target temp
 * `byte getMode();`
   * Get current mode
 * `byte getFanMode();`
   * Get current fan mode
 * `byte getEconomyMode();`
   * Get current economy mode
 * `byte getSwingMode();`
   * Get current swing mode
 * `byte getSwingStep();`
   * Get current swing step
 * `byte getControllerTemp();`
   * Get current temperature reported by the **OTHER** controller on the bus
    
 * `FujiFrame *getCurrentState();`
   * Returns a FujiFrame type containing the current state of the unit
 * `FujiFrame *getUpdateState();`
   * Returns a FujiFrame struct containing the pending update state of the unit
 * `byte getUpdateFields();`
   * Returns a byte which flags which parameters are pending update
   * See FujiHeatPump.h for flags and masks.
    
FujiHeatPump also has the following public variables:
 * `debugPrint`
   * Setting this to true will print the received/sent frames to Serial
   * example: `<-- 21 81 0 48 19 20 2C 0   mSrc: 33 mDst: 1 mType: 0 write: 0 login: 0 unknown: 1 onOff: 0 temp: 25, mode: 4 cP:0 uM:2 cTemp:22` 

## *FujiFrame* struct

```c++
typedef struct FujiFrames  {
    byte onOff = 0;
    byte temperature = 16;
    byte acMode = 0;
    byte fanMode = 0;
    byte acError = 0;
    byte economyMode = 0;
    byte swingMode = 0;
    byte swingStep = 0;
    byte controllerPresent = 0;
    byte updateMagic = 0;
    byte controllerTemp = 16;

    bool writeBit = false;
    bool loginBit = false;
    bool unknownBit = false;

    byte messageType = 0;
    byte messageSource = 0;
    byte messageDest = 0;
} FujiFrame;
```

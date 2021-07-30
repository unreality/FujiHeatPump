#include "FujiHeatPump.h"



FujiFrame FujiHeatPump::decodeFrame() {
    FujiFrame ff;

    ff.messageSource       =  readBuf[0];
    ff.messageDest         =  readBuf[1] & 0b01111111;
    ff.messageType         = (readBuf[2] & 0b00110000) >> 4;

    ff.acError             = (readBuf[kErrorIndex] & kErrorMask) >> kErrorOffset;
    ff.temperature         = (readBuf[kTemperatureIndex] & kTemperatureMask) >> kTemperatureOffset;
    ff.acMode              = (readBuf[kModeIndex] & kModeMask) >> kModeOffset;
    ff.fanMode             = (readBuf[kFanIndex] & kFanMask) >> kFanOffset;
    ff.economyMode         = (readBuf[kEconomyIndex] & kEconomyMask) >> kEconomyOffset;
    ff.swingMode           = (readBuf[kSwingIndex] & kSwingMask) >> kSwingOffset;
    ff.swingStep           = (readBuf[kSwingStepIndex] & kSwingStepMask) >> kSwingStepOffset;
    ff.controllerPresent   = (readBuf[kControllerPresentIndex] & kControllerPresentMask) >> kControllerPresentOffset;
    ff.updateMagic         = (readBuf[kUpdateMagicIndex] & kUpdateMagicMask) >> kUpdateMagicOffset;
    ff.onOff               = (readBuf[kEnabledIndex] & kEnabledMask) >> kEnabledOffset;
    ff.controllerTemp      = (readBuf[kControllerTempIndex] & kControllerTempMask) >> kControllerTempOffset; // there are 2 leading bits here that are unknown

    ff.writeBit =   (readBuf[2] & 0b00001000) != 0;
    ff.loginBit =   (readBuf[1] & 0b00100000) != 0;
    ff.unknownBit = (readBuf[1] & 0b10000000)  > 0;

    return ff;
}

void FujiHeatPump::encodeFrame(FujiFrame ff){

    memset(writeBuf, 0, 8);

    writeBuf[0] = ff.messageSource;

    writeBuf[1] &= 0b10000000;
    writeBuf[1] |= ff.messageDest & 0b01111111;

    writeBuf[2] &= 0b11001111;
    writeBuf[2] |= ff.messageType << 4;

    if(ff.writeBit){
        writeBuf[2] |= 0b00001000;
    } else {
        writeBuf[2] &= 0b11110111;
    }

    writeBuf[1] &= 0b01111111;
    if(ff.unknownBit) {
        writeBuf[1] |= 0b10000000;
    }

    if(ff.loginBit){
        writeBuf[1] |= 0b00100000;
    } else {
        writeBuf[1] &= 0b11011111;
    }

    writeBuf[kModeIndex] =              (writeBuf[kModeIndex]              & ~kModeMask)              | (ff.acMode << kModeOffset);
    writeBuf[kModeIndex] =              (writeBuf[kEnabledIndex]           & ~kEnabledMask)           | (ff.onOff << kEnabledOffset);
    writeBuf[kFanIndex] =               (writeBuf[kFanIndex]               & ~kFanMask)               | (ff.fanMode << kFanOffset);
    writeBuf[kErrorIndex] =             (writeBuf[kErrorIndex]             & ~kErrorMask)             | (ff.acError << kErrorOffset);
    writeBuf[kEconomyIndex] =           (writeBuf[kEconomyIndex]           & ~kEconomyMask)           | (ff.economyMode << kEconomyOffset);
    writeBuf[kTemperatureIndex] =       (writeBuf[kTemperatureIndex]       & ~kTemperatureMask)       | (ff.temperature << kTemperatureOffset);
    writeBuf[kSwingIndex] =             (writeBuf[kSwingIndex]             & ~kSwingMask)             | (ff.swingMode << kSwingOffset);
    writeBuf[kSwingStepIndex] =         (writeBuf[kSwingStepIndex]         & ~kSwingStepMask)         | (ff.swingStep << kSwingStepOffset);
    writeBuf[kControllerPresentIndex] = (writeBuf[kControllerPresentIndex] & ~kControllerPresentMask) | (ff.controllerPresent << kControllerPresentOffset);
    writeBuf[kUpdateMagicIndex] =       (writeBuf[kUpdateMagicIndex]       & ~kUpdateMagicMask)       | (ff.updateMagic << kUpdateMagicOffset);
    writeBuf[kControllerTempIndex] =    (writeBuf[kControllerTempIndex]    & ~kControllerTempMask)    | (ff.controllerTemp << kControllerTempOffset);

}

void FujiHeatPump::connect(HardwareSerial *serial, bool secondary){
    _serial = serial;
    _serial->begin(500, SERIAL_8E1);
    _serial->setTimeout(250);
    
    if(secondary) {
        controllerIsPrimary = false;
        controllerAddress = static_cast<byte>(FujiAddress::SECONDARY);
    } else {
        controllerIsPrimary = true;
        controllerAddress = static_cast<byte>(FujiAddress::PRIMARY);
    }
    
    lastFrameReceived = 0;
}

void FujiHeatPump::printFrame(byte buf[8], FujiFrame ff) {
  Serial.printf("%X %X %X %X %X %X %X %X  ", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
  Serial.printf(" mSrc: %d mDst: %d mType: %d write: %d login: %d unknown: %d onOff: %d temp: %d, mode: %d cP:%d uM:%d cTemp:%d \n", ff.messageSource, ff.messageDest, ff.messageType, ff.writeBit, ff.loginBit, ff.unknownBit, ff.onOff, ff.temperature, ff.acMode, ff.controllerPresent, ff.updateMagic, ff.controllerTemp);

}

void FujiHeatPump::sendPendingFrame() {
    if(pendingFrame && millis() >= lastFrameReceived + 50) {
        _serial->write(writeBuf, 8);
        pendingFrame = false;
        updateFields = 0;
    }
}

bool FujiHeatPump::waitForFrame() {
    FujiFrame ff;
    
    if(_serial->available()) {

        memset(readBuf, 0, 8);
        int bytesRead = _serial->readBytes(readBuf,8);

        if(bytesRead < 8) {
            // skip incomplete frame
            return false;
        }
        
        for(int i=0;i<8;i++) {
            readBuf[i] ^= 0xFF;
        }
    
        ff = decodeFrame();

        if(debugPrint) {
            Serial.printf("<-- ");
            printFrame(readBuf, ff);
        }
        
        if(ff.messageDest == controllerAddress) {
            lastFrameReceived = millis();
            
            if(ff.messageType == static_cast<byte>(FujiMessageType::STATUS)){
                
                // some janky error checking
                /*
                if(ff.controllerPresent == 1 && ff.updateMagic != 10) {
                    return false;
                } else if(ff.controllerPresent == 1 && ff.updateMagic != 0) {
                    return false;
                }*/
                

                if(ff.controllerPresent == 1) {
                    // we have logged into the indoor unit
                    // this is what most frames are
                    ff.messageSource     = controllerAddress;
                    
                    if(seenSecondaryController) {
                        ff.messageDest       = static_cast<byte>(FujiAddress::SECONDARY);
                        ff.loginBit          = true;
                        ff.controllerPresent = 0;
                    } else {
                        ff.messageDest       = static_cast<byte>(FujiAddress::UNIT);
                        ff.loginBit          = false;
                        ff.controllerPresent = 1;
                    }
                    
                    ff.updateMagic       = 0;
                    ff.unknownBit        = true;
                    ff.writeBit          = 0;
                    ff.messageType       = static_cast<byte>(FujiMessageType::STATUS);
                    
                } else {
                    if(controllerIsPrimary) {
                        // if this is the first message we have received, announce ourselves to the indoor unit
                        ff.messageSource     = controllerAddress;
                        ff.messageDest       = static_cast<byte>(FujiAddress::UNIT);
                        ff.loginBit          = false;
                        ff.controllerPresent = 0;
                        ff.updateMagic       = 0;
                        ff.unknownBit        = true;
                        ff.writeBit          = 0;
                        ff.messageType       = static_cast<byte>(FujiMessageType::LOGIN);
                        
                        ff.onOff             = 0;
                        ff.temperature       = 0;
                        ff.acMode            = 0;
                        ff.fanMode           = 0;
                        ff.swingMode         = 0;
                        ff.swingStep         = 0;
                        ff.acError           = 0;
                    } else {
                        // secondary controller never seems to get any other message types, only status with controllerPresent == 0
                        // the secondary controller seems to send the same flags no matter which message type
                        
                        ff.messageSource     = controllerAddress;
                        ff.messageDest       = static_cast<byte>(FujiAddress::UNIT);;
                        ff.loginBit          = false;
                        ff.controllerPresent = 1;
                        ff.updateMagic       = 2;
                        ff.unknownBit        = true;
                        ff.writeBit          = 0;
                    }
                    
                }
                
                // if we have any updates, set the flags
                if(updateFields) {
                    ff.writeBit = 1;
                }
                
                if(updateFields & kOnOffUpdateMask) {
                    ff.onOff = updateState.onOff;
                }
                
                if(updateFields & kTempUpdateMask) {
                    ff.temperature = updateState.temperature;
                }
                
                if(updateFields & kModeUpdateMask) {
                    ff.acMode = updateState.acMode;
                }
                
                if(updateFields & kFanModeUpdateMask) {
                    ff.fanMode = updateState.fanMode;
                }
                
                if(updateFields & kSwingModeUpdateMask) {
                    ff.swingMode = updateState.swingMode;
                }
                
                if(updateFields & kSwingStepUpdateMask) {
                    ff.swingStep = updateState.swingStep;
                }
                
                currentState = ff;

            }
            else if(ff.messageType == static_cast<byte>(FujiMessageType::LOGIN)){
                // received a login frame OK frame
                // the primary will send packet to a secondary controller to see if it exists
                ff.messageSource     = controllerAddress;
                ff.messageDest       = static_cast<byte>(FujiAddress::SECONDARY);
                ff.loginBit          = true;
                ff.controllerPresent = 1;
                ff.updateMagic       = 0;
                ff.unknownBit        = true;
                ff.writeBit          = 0;
                
                ff.onOff             = currentState.onOff;
                ff.temperature       = currentState.temperature;
                ff.acMode            = currentState.acMode;
                ff.fanMode           = currentState.fanMode;
                ff.swingMode         = currentState.swingMode;
                ff.swingStep         = currentState.swingStep;
                ff.acError           = currentState.acError;
            } else if(ff.messageType == static_cast<byte>(FujiMessageType::ERROR)) {
                Serial.printf("AC ERROR RECV: ");
                printFrame(readBuf, ff);
                return false;
            }
            
            encodeFrame(ff);

            if(debugPrint) {
                Serial.printf("--> ");
                printFrame(writeBuf, ff);
            }

            for(int i=0;i<8;i++) {
                writeBuf[i] ^= 0xFF;
            }
                    
            pendingFrame = true;
                        

        } else if (ff.messageDest == static_cast<byte>(FujiAddress::SECONDARY)) {
            seenSecondaryController = true;
            currentState.controllerTemp = ff.controllerTemp; // we dont have a temp sensor, use the temp reading from the secondary controller
        }
        
        return true;
    }
    
    return false;
}

bool FujiHeatPump::isBound() {
    if(millis() - lastFrameReceived < 1000) {
        return true;
    }
    return false;
}

bool FujiHeatPump::updatePending() {
    if(updateFields) {
        return true;
    }
    return false;
}

void FujiHeatPump::setOnOff(bool o){
    updateFields |= kOnOffUpdateMask;
    updateState.onOff = o ? 1 : 0;   
}
void FujiHeatPump::setTemp(byte t){
    updateFields |= kTempUpdateMask;
    updateState.temperature = t;
}
void FujiHeatPump::setMode(byte m){
    updateFields |= kModeUpdateMask;
    updateState.acMode = m;
}
void FujiHeatPump::setFanMode(byte fm){
    updateFields |= kFanModeUpdateMask;
    updateState.fanMode = fm;
}
void FujiHeatPump::setEconomyMode(byte em){
    updateFields |= kEconomyModeUpdateMask;
    updateState.economyMode = em;
}
void FujiHeatPump::setSwingMode(byte sm){
    updateFields |= kSwingModeUpdateMask;
    updateState.swingMode = sm;
}
void FujiHeatPump::setSwingStep(byte ss){
    updateFields |= kSwingStepUpdateMask;
    updateState.swingStep = ss;  
}

bool FujiHeatPump::getOnOff(){
    return currentState.onOff == 1 ? true : false;
}
byte FujiHeatPump::getTemp(){
    return currentState.temperature;
}
byte FujiHeatPump::getMode(){
    return currentState.acMode;
}
byte FujiHeatPump::getFanMode(){
    return currentState.fanMode;
}
byte FujiHeatPump::getEconomyMode(){
    return currentState.economyMode;
}
byte FujiHeatPump::getSwingMode(){
    return currentState.swingMode;
}
byte FujiHeatPump::getSwingStep(){
    return currentState.swingStep;
}
byte FujiHeatPump::getControllerTemp(){
    return currentState.controllerTemp;
}

FujiFrame *FujiHeatPump::getCurrentState(){
    return &currentState;
}

FujiFrame *FujiHeatPump::getUpdateState(){
    return &updateState;
}

byte FujiHeatPump::getUpdateFields(){
    return updateFields;
}
 


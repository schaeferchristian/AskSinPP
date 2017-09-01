//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-07-26 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER

// define all device properties
#define DEVICE_ID HMID(0x00,0x1a,0x00)
#define DEVICE_SERIAL "HMRC001A00"
#define DEVICE_MODEL  0x00,0x1a
#define DEVICE_FIRMWARE 0x11
#define DEVICE_TYPE DeviceType::Remote
#define DEVICE_INFO 0x01,0x00,0x00

#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <TimerOne.h>
#include <LowPower.h>

#include <MultiChannelDevice.h>
#include <Remote.h>

// Arduino pin for the config button
// B0 == PIN 8
#define CONFIG_BUTTON_PIN 8
// Arduino pins for the buttons
// A0 == PIN 14
#define BTN1_PIN 14


// number of available peers per channel
#define PEERS_PER_CHANNEL 10

// all library classes are placed in the namespace 'as'
using namespace as;

/**
 * Configure the used hardware
 */
typedef AvrSPI<10,11,12,13> SPIType;
typedef Radio<SPIType,2> RadioType;
typedef DualStatusLed<5,4> LedType;
typedef AskSin<LedType,BatterySensor,RadioType> HalType;
class Hal : public HalType {
  // extra clock to count button press events
  AlarmClock btncounter;
public:
  void init () {
    HalType::init();
    // get new battery value after 50 key press
    battery.init(50,btncounter);
    battery.low(22);
    battery.critical(19);
  }

  void sendPeer () {
    --btncounter;
  }

  bool runready () {
    return HalType::runready() || btncounter.runready();
  }
};

typedef RemoteChannel<Hal,PEERS_PER_CHANNEL> ChannelType;
typedef MultiChannelDevice<Hal,ChannelType,1> RemoteType;

Hal hal;
RemoteType sdev(0x20);
ConfigButton<RemoteType> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);

  remoteISR(sdev,1,BTN1_PIN);

  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
}

void loop() {
  bool pinchanged = false;
  for( int i=1; i<=sdev.channels(); ++i ) {
    if( sdev.channel(i).checkpin() == true) {
      pinchanged = true;
    }
  }
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( pinchanged == false && worked == false && poll == false ) {
    hal.activity.savePower<Sleep<>>(hal);
  }
}
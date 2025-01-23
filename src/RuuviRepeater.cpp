/* 
 * Project RuuviRepeater
 * Author: Timo Pelkonen
 * Date: 19.1.2025
 * BLE repeater for Ruuvitags
 */

#ifdef TESTING
  #include "../test/ParticleDummy.h"
  //just in case unit tests are needed..
#else
  #include "Particle.h"
#endif

// Let Device OS manage the connection to the Particle Cloud
// no we don't, let's make it fully offline..
// old xenon with zero networking capabilities..
SYSTEM_MODE(MANUAL);

// Run the application and system concurrently in separate threads
SYSTEM_THREAD(ENABLED);

const pin_t MY_LED = D7; //particle Argon/Xenon blue led next to usb port
//this is used to signal that signal was received and retransmitted.
//handy when looking for good location for the repeater


const size_t SCAN_RESULT_MAX = 30;

const int TX_POWER = 8; //transmit power. +8 max, -12 minimum maybe?


BleScanResult scanResults[SCAN_RESULT_MAX];

BleAdvertisingData txData;


SerialLogHandler logHandler(LOG_LEVEL_INFO);


void setup() {


  pinMode(MY_LED, OUTPUT);
  BLE.setTxPower(TX_POWER); // MAX POWER
  BLE.on();

  BLE.setAdvertisingInterval(1600); //1600: 1 sec. 400-> 0,25 sec. two transmissions
  //this is only for the transmit part. 1 transmit per cycle so let's keep this long.

}

void loop() {



  BLE.setScanTimeout(50); // 50 = 500 ms probably ok?

  int scanCount = BLE.scan(scanResults, SCAN_RESULT_MAX);

  for (int i = 0; i < scanCount; i++) {
    uint8_t buf[BLE_MAX_ADV_DATA_LEN];
		size_t len;
    len= scanResults[i].advertisingData(buf, BLE_MAX_ADV_DATA_LEN);
    
    //Log.info("Len: %i, manufacturer %x", len, buf[7]);
    //len is supposed to be: 31 and manufacturer: 5
    if (buf[7] == 0x5 ) {
      Log.info("Ruuvitag found! (%i) - %02x:%02x:%02x:%02x:%02x:%02x", len, scanResults[i].address[5], scanResults[i].address[4], scanResults[i].address[3], scanResults[i].address[2], scanResults[i].address[1], scanResults[i].address[0] );
      txData.clear();
      txData.set(buf, BLE_MAX_ADV_DATA_LEN);

      BLE.setAddress(scanResults[i].address);
      
      digitalWrite(MY_LED, HIGH);
      BLE.setAdvertisingData(&txData);
      BLE.advertise();
      delay(100);
      BLE.stopAdvertising();
      digitalWrite(MY_LED, LOW);

    }



  /* The data is decoded from "Manufacturer Specific Data" -field, 
  for more details please check Bluetooth Advertisements section. 
  Manufacturer ID is 0x0499 , which is transmitted as 0x9904 in raw data. 
  The actual data payload is:
  ...
  https://docs.ruuvi.com/communication/bluetooth-advertisements/data-format-5-rawv2

  -8 -> 0 : ble related header.
  0: data format, = 5
  1-2 : temp
  3-4 : humidity, 40 000 = 100%
  5-6 : pressure
  7-12: acceleration
  13-14 : power info
  15: movement counter
  16-17: measurement sequence
  18-23 : mac

  */

  }


}
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

#define argon
const bool packetDebug = false; // true -> show packet data on serial. false-> don't show
//use this to get things to compile with argon

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

const int delayBLE = 32; //define delay for ble transmit part. 1600 = 1000 ms
const int delayMillis = 20; //same delay in milliseconds
/*
Interval Value	Milliseconds	Description
32	20 ms	Minimum value
160	100 ms	Default value
400	250 ms	
800	500 ms	
1600	1 sec	
3200	2 sec	Upper end of recommended range
16383	10.24 sec	Maximum value

*/

BleScanResult scanResults[SCAN_RESULT_MAX];

BleAdvertisingData txData;


SerialLogHandler logHandler(LOG_LEVEL_INFO);


void setup() {


  pinMode(MY_LED, OUTPUT);
  BLE.setTxPower(TX_POWER); // MAX POWER
  BLE.on();

  BLE.setAdvertisingInterval(delayBLE);
  //this is only for the transmit part. 1 transmit per cycle so let's keep this long.
  //actually keeping this long prevents retransmitting multiple beacon results...
}

void loop() {



  BLE.setScanTimeout(50); 
  // 50 = 500 ms seems to be pretty ok
  // 150 = 1,5s would receive all transmissions but introduce delays
  //nope, increasing the time does not ensure that all beacons are found within the timeout...


  int scanCount = BLE.scan(scanResults, SCAN_RESULT_MAX);

  uint8_t blockingArray[scanCount][2];
  bool blocked = false;

  for (int i = 0; i < scanCount; i++) {

    size_t len;
    uint8_t buf[BLE_MAX_ADV_DATA_LEN];
    size_t checksum[3];

    if (packetDebug) {
    Log.info("Buffer len %i dump: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x ", 
      len, buf[0], buf[1], buf[2], buf[3], buf[4], 
      buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], 
      buf[11], buf[12], buf[13], buf[14], buf[15], 
      buf[16], buf[17], buf[18], buf[19], buf[20], buf[21],
      buf[22], buf[23],buf[24],buf[25],buf[26],buf[27],
      buf[28],buf[29], buf[30]);
    }


    #ifdef argon
    len = scanResults[i].advertisingData().get(BleAdvertisingDataType::MANUFACTURER_SPECIFIC_DATA, buf, BLE_MAX_ADV_DATA_LEN);
    //len = scanResults[i].advertisingData().get(buf, BLE_MAX_ADV_DATA_LEN);
    checksum[0] = buf[0];
    checksum[1] = buf[1];
    checksum[2] = buf[2];
    uint8_t lenref = 26;
    #else
    len= scanResults[i].advertisingData(buf, BLE_MAX_ADV_DATA_LEN);  
    checksum[0] = buf[5];
    checksum[1] = buf[6];
    checksum[2] = buf[7];
    uint8_t lenref = 31; //?!?!?
    #endif
    
    //Log.info("Scanned: %02x %02x %02x %02x %02x %02x", buf[len-5], buf[len-4], buf[len-3], buf[len-2], buf[len-1], buf[len]);
    //Log.info("Len: %i, manufacturer %02x %02x datatype: %02x", len, buf[6], buf[5], buf[7]);
    //Log.info("Dump: len: %i, data: %02x %02x %02x %02x %02x %02x %02x %02x", len, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
    //len is supposed to be: 31, manufactuer buf[6] = 04 buf[5] = 99and data format buf[7]: 5
    if (checksum[2] == 0x5 and checksum[1] == 0x04 and checksum[0] == 0x99 and len == lenref) {
      txData.clear();

      #ifdef argon

      uint8_t nameString[] = "Ruuvi xxxx";
      //txData.appendLocalName("Ruuvi xxxx");
      
      //txData.append(particle::BleAdvertisingDataType::SHORT_LOCAL_NAME, nameString,sizeof(nameString)/sizeof(uint8_t),true);
      txData.appendCustomData(buf, len, false);
      
      //txData.append()
      //txData.append(SHORT_LOCAL_NAME)

      //nameString = "Ruuvi";
      //char fixed[] = "Ruuvi ";
      //strcat(nameString, fixed);
      //strcat(nameString, (char*)buf[24]);
      //strcat(nameString, (char*)buf[25]);
      
      //Log.info("Local name: %c", (char *) nameString);

      #else
      txData.set(buf, len);
      #endif


      /*
      SHORT_LOCAL_NAME
    COMPLETE_LOCAL_NAME
      */


      #ifdef argon
      Log.info("Ruuvitag found! Signal: %i length: %i, Address: %02x %02x %02x %02x %02x %02x",
        scanResults[i].rssi(), len,  buf[20], buf[21], buf[22], buf[23], buf[24], buf[25]);
      

      for (int j = 0; j < i; j++) {
        if (blockingArray[j][0] == buf[25] and blockingArray[j][1] == buf[26]) {
          blocked = true;
        }
      }

      blockingArray[i][0] = buf[25];
      blockingArray[i][1] = buf[26];

      //int type(BleAddressType type);
      //int set(const uint8_t addr[BLE_SIG_ADDR_LEN], BleAddressType type = BleAddressType::PUBLIC);
      
      //BleAddress address = BleAddress(scanResults[i].address[], BleAddressType type = BleAddressType::PUBLIC);
      
      BLE.setAddress(scanResults[i].address());
      #else
      for (int j = 0; j < i; j++) {
        if (blockingArray[j][0] == scanResults[i].address[1] and blockingArray[j][1] == scanResults[i].address[0]) {
          blocked = true;
        }
      }

      blockingArray[i][0] = scanResults[i].address[1];
      blockingArray[i][1] = scanResults[i].address[0];
      Log.info("Ruuvitag found! Signal: %i Lenght: %i Address: %02x:%02x:%02x:%02x:%02x:%02x", scanResults[i].rssi, len, scanResults[i].address[5], scanResults[i].address[4], scanResults[i].address[3], scanResults[i].address[2], scanResults[i].address[1], scanResults[i].address[0] );
      BLE.setAddress(scanResults[i].address);
      #endif

      if (!blocked) {
      digitalWrite(MY_LED, HIGH);

      BLE.setAdvertisingData(&txData);
      BLE.advertise();
      delay(delayMillis);
      //delay(50+random(50));
      BLE.stopAdvertising();
      digitalWrite(MY_LED, LOW);
      }
      else {
        Log.info("double, retransmit blocked");
        blocked = false;
      }

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
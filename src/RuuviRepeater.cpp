/* 
 * Project RuuviRepeater
 * Author: Timo Pelkonen
 * Date: 19.1.2025
 * Updates 16.8.2026
 * BLE repeater for Ruuvitags
 
 Compiles and tested for Argon & Xenon. The difference is the latest particle firmware and Xenon is stuck to much older version.
 So you can compare your variant particle firmware version to these and send me a merge request from your more complete compile time
 platform_id check clauses. Thank you!
 */

#ifdef TESTING
  #include "../test/ParticleDummy.h"
  //just in case unit tests are needed..
#else
  #include "Particle.h"
#endif



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

void Blink(int qty) {
	
	for (int i = 0; i < qty; i++) {
		
	digitalWrite(MY_LED, HIGH);
	delay(100);
	digitalWrite(MY_LED, LOW);
	delay(100);
		  
	}
}

void setup() {


  pinMode(MY_LED, OUTPUT);
  BLE.setTxPower(TX_POWER); // MAX POWER
  BLE.on();

  BLE.setAdvertisingInterval(delayBLE);
  //delay between transmissions, should be kept as short as possible.
  
}

void loop() {



  BLE.setScanTimeout(150); 
  // 50 = 500 ms seems to be pretty ok
  // 150 = 1,5s would receive all transmissions but introduce delays
  //nope, increasing the time does not ensure that all beacons are found within the timeout...
  
  //update: practically the delays are up to multiple minutes so no need to make this fast.
  //Testing if longer duration helps with retransmitting all the beacons.
  //5000 ms causes a lot of bloked retransmissions so not going to use that.


  int scanCount = BLE.scan(scanResults, SCAN_RESULT_MAX);

  uint8_t blockingArray[scanCount][2];
  bool blocked = false;
  
  int transmissions = 0;

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

	#if PLATFORM_ID == PLATFORM_ARGON
    len = scanResults[i].advertisingData().get(BleAdvertisingDataType::MANUFACTURER_SPECIFIC_DATA, buf, BLE_MAX_ADV_DATA_LEN);
    //len = scanResults[i].advertisingData().get(buf, BLE_MAX_ADV_DATA_LEN);
    checksum[0] = buf[0];
    checksum[1] = buf[1];
    checksum[2] = buf[2];
    uint8_t lenref = 26;
	#endif
	
	#if PLATFORM_ID == PLATFORM_XENON
    len= scanResults[i].advertisingData(buf, BLE_MAX_ADV_DATA_LEN);  
    checksum[0] = buf[5];
    checksum[1] = buf[6];
    checksum[2] = buf[7];
    uint8_t lenref = 31; //?!?!?
    #endif
    

    //len is supposed to be: 31, manufactuer buf[6] = 04 buf[5] = 99and data format buf[7]: 5
    if (checksum[2] == 0x5 and checksum[1] == 0x04 and checksum[0] == 0x99 and len == lenref) {
      txData.clear();

	#if PLATFORM_ID == PLATFORM_ARGON

      //uint8_t nameString[] = "Ruuvi xxxx";
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

	#endif
	
	#if PLATFORM_ID == PLATFORM_XENON
       txData.set(buf, len);
    #endif


      /*
      SHORT_LOCAL_NAME
    COMPLETE_LOCAL_NAME
      */

	#if PLATFORM_ID == PLATFORM_ARGON

      Log.info("Ruuvitag %02x %02x %02x %02x %02x %02x found! Signal: %i", buf[20], buf[21], buf[22], buf[23], buf[24], buf[25],
        scanResults[i].rssi());
      

      for (int j = 0; j < i; j++) {
        if (blockingArray[j][0] == buf[25] and blockingArray[j][1] == buf[26]) {
          blocked = true;
        }
      }

		//array to check if there are retransmissions within and block any.
      blockingArray[i][0] = buf[25];
      blockingArray[i][1] = buf[26];

      //int type(BleAddressType type);
      //int set(const uint8_t addr[BLE_SIG_ADDR_LEN], BleAddressType type = BleAddressType::PUBLIC);
      
      //BleAddress address = BleAddress(scanResults[i].address[], BleAddressType type = BleAddressType::PUBLIC);
	  
	  
      
      BLE.setAddress(scanResults[i].address());
	  
	  #endif
	  #if PLATFORM_ID == PLATFORM_XENON
      
      for (int j = 0; j < i; j++) {
        if (blockingArray[j][0] == scanResults[i].address[1] and blockingArray[j][1] == scanResults[i].address[0]) {
          blocked = true;
        }
      }

      blockingArray[i][0] = scanResults[i].address[1];
      blockingArray[i][1] = scanResults[i].address[0];
      Log.info("Ruuvitag %02x:%02x:%02x:%02x:%02x:%02x found! Signal: %i", scanResults[i].address[5], scanResults[i].address[4], scanResults[i].address[3], scanResults[i].address[2], scanResults[i].address[1], scanResults[i].address[0], scanResults[i].rssi );
      BLE.setAddress(scanResults[i].address);
      #endif

      if (!blocked) {

	  transmissions ++;

      BLE.setAdvertisingData(&txData);
      BLE.advertise();
      delay(delayMillis + random(50));
      //delay(50+random(50));
      BLE.stopAdvertising();
	  
	  BLE.advertise();
	  delay(delayMillis + random(50));
	  BLE.stopAdvertising();
	  //trying double advertisement to see if it helps with retransmissions not getting through

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
  Log.info("Heard %i transmissions and retransmitted %i.", scanCount, transmissions);
  
  Blink(transmissions);


}


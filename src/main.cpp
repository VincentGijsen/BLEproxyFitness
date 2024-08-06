// #include "BLE_bike.h"
#include <Adafruit_NeoPixel.h>

#define PIN 48
#define NUMPIXELS 1

Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 500
#include "BLE_service_heartrate.h"
#include "BLE_service_fitness.h"

#include <Arduino.h>
BLEAdvertisementData advertisementData = BLEAdvertisementData();
BLEServer *pServer;

ServiceHeartRate *serviceHeartRate;
ServiceFitness *serviceFitness;

byte bpm = 90, powerIn, cadenceIn, speedOut;
byte pairedToTrainer = false;

#define tunturiMACAddress "d0:33:36:33:1b:5b"                                          // CAREFULL, lowercase!!?!
static BLEUUID tunturiServiceUUID("00001826-0000-1000-8000-00805f9b34fb");             // [SERVICE_FITNESS_MACHINE_UUID]ID of servcie
static BLEUUID tunturiTrainerDataChar("00002AD2-0000-1000-8000-00805f9b34fb");         // characterisic with updates for us.
static BLEUUID tunturiTrainerControlPointChar("00002AD9-0000-1000-8000-00805f9b34fb"); // characterisic with updates for us.
/*
LED stuff
*/


void blinkRed(){
  strip.setPixelColor(0, strip.Color(100, 0, 0)); // Red
  strip.show();
  sleep(100);
  strip.setPixelColor(0, strip.Color(0, 0, 0)); // Red
  strip.show();
}

void blinkGreen(){
  strip.setPixelColor(0, strip.Color(0, 100, 0)); // 
  strip.show();
  sleep(100);
  strip.clear();
  //strip.setPixelColor(0, strip.Color(0, 0, 0)); // 
  //strip.show();
}
void blinkBlue(){
  strip.setPixelColor(0, strip.Color(0, 0, 100)); // 
  strip.show();
  sleep(100);
  strip.setPixelColor(0, strip.Color(0, 0, 0)); // 
  strip.show();
}



/*
BLE-Client services*/

BLERemoteService *pRemoteServiceTunturiControl;

#define bleServerName "BME280_ESP32"

// Callback function that gets called, when another device's advertisement has been received
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    Serial.print("found: ");
    Serial.print(advertisedDevice.getName().c_str());
    Serial.print(" - ");
    Serial.print(advertisedDevice.getAddress().toString().c_str());
    Serial.println("");
  }
};

// AS CLIENT
// when power reading notification received
void crossTrainerDataCallback(
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
  Serial.print("crosstrainerDataCB:	");
  // powerIn = (int16_t)(pData[1] + (pData[0] << 8));
  Serial.print("data: CB: length:");
  Serial.print(length, DEC);
  Serial.println(" data: ");
  for (uint8_t x = 0; x < length; x++)
  {
    // Serial.print("i");
    // Serial.print(x, DEC);
    Serial.print(" 0x");
    Serial.print(pData[x], HEX);
    // Serial.print(", ");
  }
  Serial.println(" done");
  // https://developer.huawei.com/consumer/en/doc/HMSCore-Guides/ibd-0000001051005923 for fields
  uint16_t instSpeed_1_10h = pData[3] + (pData[4] << 8);
  uint16_t avgSpeed_1_10h = pData[5] + (pData[6] << 8);
  uint16_t instCadence_1_2th = pData[6] + (pData[7] << 8);
  uint16_t avgCadence_1_2th = pData[8] + (pData[9] << 8);
  uint16_t tot_dist_meter = pData[10] + (pData[11] << 8) + (pData[12] << 16);
  uint16_t resistance_level = pData[13] + (pData[14] << 8);
  uint16_t instPowerWatt = pData[15] + (pData[16] << 8);
  uint16_t avgPowerWatt = pData[17] + (pData[18] << 8);

  uint16_t totEnergycal = pData[19] + (pData[20] << 8);
  uint16_t energyHour = pData[21] + (pData[22] << 8);
  uint8_t energyMin = pData[23];
  uint8_t heartRateBeatMin = pData[24];
  uint8_t metaRate = pData[25];

  uint16_t elapsed_time = pData[26] + (pData[27] << 8);
  uint16_t remain_time = pData[27] + (pData[28] << 8);

  Serial.print("inst speed: ");
  Serial.print(instSpeed_1_10h, DEC);
  Serial.print(", ");

  Serial.print("Heartrate: ");
  Serial.print(heartRateBeatMin, DEC);
  Serial.print(", ");

  serviceHeartRate->notifyHeartRateUpdate(heartRateBeatMin);


  Serial.print("tot-dist: ");
  Serial.print(tot_dist_meter, DEC);
  Serial.print(", ");

  Serial.print("elapsedTime: ");
  Serial.print(elapsed_time, DEC);
  Serial.print(", ");

  Serial.print("resistance_level: ");
  Serial.print(resistance_level, DEC);
  Serial.print(", ");

  serviceFitness->prepareNotifyFitnessCadence(instCadence_1_2th);

  Serial.print("inst Cadence: ");
  Serial.print(instCadence_1_2th, DEC);
  Serial.print(", ");

  serviceFitness->prepareNotifyFitnessCadence(instCadence_1_2th);

  Serial.print("inst Power: ");
  Serial.print(instPowerWatt, DEC);
  Serial.print(", ");

  serviceFitness->prepareNotifyFitnessPower(instPowerWatt);

  Serial.print("avg Cadence: ");
  Serial.print(avgCadence_1_2th, DEC);
  Serial.print(", ");

  Serial.print("avg Power: ");
  Serial.print(avgPowerWatt, DEC);
  Serial.print(", ");

  Serial.println("");
  
  serviceFitness->notifyFitnessData();
 // blinkBlue();
}

/*
// when cadence reading notification received
void cadenceCallback(
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
  cadenceIn = (int16_t)(pData[1] + (pData[0] << 8));
  cadenceIn = constrain(cadenceIn, 0, 300);
  Serial.print("cadenceIn	");
  Serial.println(cadenceIn);
}
*/

//prototype hacks
void setResitanceToTrainer(uint8_t resitanceLevel);

//callback that is injected into 'servr' logic, to ret App requst, and foward over BLE-client connection to bike
void mySetResistanceHanlder(uint8_t resistanceRequestedByApp){
//check if connected, etc..
Serial.println("Request for setting resistance received");
setResitanceToTrainer(resistanceRequestedByApp);

}
void InitBLEServer()
{
  //blinkGreen();

  pServer = BLEDevice::createServer();
  // BLEAdvertisementData *pAdvertisementData = &advertisementData;

  serviceHeartRate = new ServiceHeartRate(pServer, &advertisementData);
  serviceHeartRate->setupService();

  serviceFitness = new ServiceFitness(pServer, &advertisementData, &mySetResistanceHanlder);
  serviceFitness->setupService();

  // setup_bike(pServer, pAdvertisementData);
  //  setup_heart(pServer, pAdvertisementData);

  // added characteristics and descriptors - 2902 required for characteristics that notify or indicate

  advertisementData.setFlags(ESP_BLE_ADV_FLAG_BREDR_NOT_SPT + ESP_BLE_ADV_FLAG_GEN_DISC); // set BLE EDR not supported and general discoverable flags, necessary for RGT
  pServer->getAdvertising()->setAdvertisementData(advertisementData);

  pServer->getAdvertising()->start();

  Serial.println("Init of BLEserver complete..");
}

BLEAddress *myDevice;
BLERemoteCharacteristic *indoorBikeDataCharacteristicClient;
BLERemoteCharacteristic *indoorBikeControlCharacteristicClient;

bool connectToServer()
{
 // blinkRed();
  Serial.println("fun connectToServer()");
  // Serial.println(tunturiServiceUUID->getAddress().toString().c_str());

  BLEClient *pClient = BLEDevice::createClient();
  Serial.println("Created client");

  myDevice = new BLEAddress(tunturiMACAddress);

  // Connect to the remove BLE Server.
  bool _connected = pClient->connect(*myDevice, BLE_ADDR_TYPE_RANDOM); //, BLE_ADDR_TYPE_PUBLIC);
  // the RANDOm bit is important, else it will not connect!

  if (!_connected)
  {
    Serial.println("failed to connect");
    return false;
  }

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService *pRemoteService = pClient->getService(tunturiServiceUUID);

  // pRemoteServiceTunturiControl = pClient->getService(tunturiTrainerControlPoint);
  // crossTrianerControlCharacteristic = pRemoteServiceTunturiControl->getCharacteristic()

  if (pRemoteService == nullptr)
  {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(tunturiServiceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  indoorBikeDataCharacteristicClient = pRemoteService->getCharacteristic(tunturiTrainerDataChar);
  indoorBikeDataCharacteristicClient->registerForNotify(crossTrainerDataCallback);

  // obtain ref control char
  indoorBikeControlCharacteristicClient = pRemoteService->getCharacteristic(tunturiTrainerControlPointChar);

  pairedToTrainer = true;
  //setResitanceToTrainer(2);
  Serial.print("Paired to trainer\n");
  
  return true;
}

void setResitanceToTrainer(uint8_t resitanceLevel)
{
  const byte OPCODE_SET_RESISTANCE = 0x04;
  uint8_t data[2] = {OPCODE_SET_RESISTANCE, resitanceLevel};

  indoorBikeControlCharacteristicClient->writeValue(data, sizeof(data), false);
  
}

void setTargetpowerToTrainer(uint8_t targetlevel)
{
  const byte OPCODE_SET_TARGET_POWER = 0x05;
  uint8_t data[2] = {OPCODE_SET_TARGET_POWER, targetlevel};

  indoorBikeControlCharacteristicClient->writeValue(data, sizeof(data), false);
}

void setup()
{
  Serial.begin(115200);

 // strip.begin();
 // strip.show();                                   // Initialize all pixels to 'off'
//  strip.setPixelColor(0, strip.Color(0, 0, 0)); // Red
//  strip.setBrightness(100);
//  strip.show();


  delay(2000);
 //  strip.clear();
  Serial.println("starting up..");
  // put your setup code here, to run once:

  BLEDevice::init("IndoorBike & HRM"); // name of the ble device

  /*
  //scan for bluetooth devs;
   BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->start(30);
  */

 InitBLEServer();

  while (!connectToServer())
  {
    delay(1000);
    Serial.println("failed to connect to server, retrying indef.");
  }
  Serial.println("connection established");

  //InitBLEServer();
}

void loop()
{
  // put your main code here, to run repeatedly:
  delay(2000);

  // sendDemoData();
}

void sendDemoData()
{
  Serial.println("preparing data to send..");
  serviceFitness->prepareNotifyFitnessPower(powerIn);
  serviceFitness->prepareNotifyFitnessCadence(cadenceIn);
  serviceFitness->prepareNotifyFitnessSpeed(speedOut);

  /*
  indoorBikeDataCharacteristicData[2] = (uint8_t)(speedOut & 0xff);
  indoorBikeDataCharacteristicData[3] = (uint8_t)(speedOut >> 8); // speed value with little endian order
  indoorBikeDataCharacteristicData[4] = (uint8_t)((cadenceIn * 2) & 0xff);
  indoorBikeDataCharacteristicData[5] = (uint8_t)((cadenceIn * 2) >> 8); // cadence value
  indoorBikeDataCharacteristicData[6] = (uint8_t)(constrain(powerIn, 0, 4000) & 0xff);
  indoorBikeDataCharacteristicData[7] = (uint8_t)(constrain(powerIn, 0, 4000) >> 8); // power value, constrained to avoid negative values, although the specification allows for a sint16

  indoorBikeDataCharacteristic.setValue(indoorBikeDataCharacteristicData, 8); // values sent
  indoorBikeDataCharacteristic.notify();                                      // device notified
*/
  speedOut + 5;
  speedOut = speedOut % 100;

  cadenceIn += 1;
  cadenceIn = cadenceIn % 10;

  powerIn += 1;
  powerIn = powerIn % 1000;

  bpm++;
  if (bpm > 180)
  {
    bpm = 80;
  }
  serviceHeartRate->notifyHeartRateUpdate(bpm);
  serviceHeartRate->notifyHeartSensorPosUpdate(SENS_LOC_HAND);

  serviceFitness->notifyFitnessData();
  // bpm = (byte)randomGen(80,180);
}

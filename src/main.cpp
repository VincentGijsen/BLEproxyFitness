#include "BLE_bike.h"
#include "BLE_heart.h"

#include <Arduino.h>
BLEAdvertisementData advertisementData = BLEAdvertisementData();

// put function declarations here:
int myFunction(int, int);
byte bpm =90;

#define MACAddress "24:0a:c4:62:4b:2e" // address of the device sending data

// AS CLIENT
void powerCallback( // when power reading notification received
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
  powerIn = (int16_t)(pData[1] + (pData[0] << 8));
  Serial.print("powerIn	");
  Serial.println(powerIn);
}

void cadenceCallback( // when cadence reading notification received
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

void InitBLEServer()
{
  BLEServer *pServer = BLEDevice::createServer();
  BLEAdvertisementData *pAdvertisementData = &advertisementData;
 
 
  setup_bike(pServer, pAdvertisementData);
  setup_heart(pServer, pAdvertisementData);

  // added characteristics and descriptors - 2902 required for characteristics that notify or indicate

  advertisementData.setFlags(ESP_BLE_ADV_FLAG_BREDR_NOT_SPT + ESP_BLE_ADV_FLAG_GEN_DISC); // set BLE EDR not supported and general discoverable flags, necessary for RGT
  pServer->getAdvertising()->setAdvertisementData(advertisementData);
 

  pServer->getAdvertising()->start();

  Serial.println("Init of BLEserver complete..");
}

bool connectToServer()
{
  Serial.print("Forming a connection to ");
  // Serial.println(myDevice->getAddress().toString().c_str());

  BLEClient *pClient = BLEDevice::createClient();
  Serial.println(" - Created client");

  myDevice = new BLEAddress(MACAddress);

  // Connect to the remove BLE Server.
  pClient->connect(*myDevice, BLE_ADDR_TYPE_PUBLIC);
  if (pClient->isConnected())
  {
    Serial.println(" - Connected to server");
  }
  else
  {
    Serial.println("failed");
  }

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService *pRemoteService = pClient->getService(serviceUUID);

  if (pRemoteService == nullptr)
  {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pPowerCharacteristic = pRemoteService->getCharacteristic(powerUUID);
  pPowerCharacteristic->registerForNotify(powerCallback);
  pCadenceCharacteristic = pRemoteService->getCharacteristic(cadenceUUID);
  pCadenceCharacteristic->registerForNotify(cadenceCallback);

  return true;
}

void setup()
{
  Serial.begin(115200);

  delay(2000);
  Serial.println("starting up..");
  // put your setup code here, to run once:
  int result = myFunction(2, 3);

  // BLEDevice::init("IndoorBike"); // name of the ble device

  BLEDevice::init("IndoorBike & HRM"); // name of the ble device

  // connectToServer();

  InitBLEServer();
}

void loop()
{
  // put your main code here, to run repeatedly:
  delay(1000);
  indoorBikeDataCharacteristicData[2] = (uint8_t)(speedOut & 0xff);
  indoorBikeDataCharacteristicData[3] = (uint8_t)(speedOut >> 8); // speed value with little endian order
  indoorBikeDataCharacteristicData[4] = (uint8_t)((cadenceIn * 2) & 0xff);
  indoorBikeDataCharacteristicData[5] = (uint8_t)((cadenceIn * 2) >> 8); // cadence value
  indoorBikeDataCharacteristicData[6] = (uint8_t)(constrain(powerIn, 0, 4000) & 0xff);
  indoorBikeDataCharacteristicData[7] = (uint8_t)(constrain(powerIn, 0, 4000) >> 8); // power value, constrained to avoid negative values, although the specification allows for a sint16

  indoorBikeDataCharacteristic.setValue(indoorBikeDataCharacteristicData, 8); // values sent
  indoorBikeDataCharacteristic.notify();                                      // device notified

  speedOut + 5;
  speedOut = speedOut % maxPowLev;

  cadenceIn += 1;
  cadenceIn = cadenceIn % 10;

  powerIn += 1;
  powerIn = powerIn % 1000;

  bpm++;
  if (bpm > 180)
  {
    bpm = 80;
  }
  
notifyHeartRate(bpm);
notifyHeartSensorPos(SENS_LOC_HAND);

  // bpm = (byte)randomGen(80,180);
}

// put function definitions here:
int myFunction(int x, int y)
{
  return x + y;
}
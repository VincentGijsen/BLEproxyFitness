

#ifndef BLE_serviceFitness_h
#define BLE_serviceFitness_h

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEAdvertising.h>
#include <BLE2902.h>
#include <Arduino.h>

#define fitnessMachineService BLEUUID((uint16_t)0x1826) // fitness machine service uuid, as defined in gatt specifications

// no app cares about these flags, but they are there just in case
const uint16_t indoorBikeDataCharacteristicDef = 0b0000000001000100;                          // flags for indoor bike data characteristics - power and cadence
const uint32_t fitnessMachineFeaturesCharacteristicsDef = 0b00000000000000000100000010000010; // flags for Fitness Machine Features Field - cadence, resistance level and inclination level
const uint32_t targetSettingFeaturesCharacteristicsDef = 0b00000000000000000010000000001100;  // flags for Target Setting Features Field - power and resistance level, Indoor Bike Simulation Parameters

static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b"); // custom uuid service of the crank
static BLEUUID powerUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");   // custom uuid of power characteristic
static BLEUUID cadenceUUID("ceb5483e-36e1-4688-b7f5-ea07361b26a8"); // custom uuid of cadence characteristic

class ServiceFitness
{
public:
    ServiceFitness(BLEServer *pServer, BLEAdvertisementData *pAdvertisementData, void(*funSetResistanceReq)(uint8_t));
    void setupService();
    void prepareNotifyFitnessSpeed(uint8_t speed);
    void prepareNotifyFitnessCadence(uint8_t cadence);
    void prepareNotifyFitnessPower(uint8_t power);
    void notifyFitnessData();
    void setResistanceSet(uint8_t resistance);
    void setGrade(uint8_t grade);
    void setCallbackResitHandler(void (*fun)(uint8_t));
private:
    BLEServer *_pServer;
    BLEService *_pFitness;
    BLEAdvertisementData *_pAdvertisementData;
    void (*pCallbackSetResistance)(uint8_t) ;
    const int16_t minPowLev = 0; // power level range settings, no app cares about this
    const int16_t maxPowLev = 16 ;
    const int16_t minPowInc = 1;
    const int16_t maxResLev = 16;

    uint8_t resistanceSet = 0;
    uint8_t grade = 0;
    uint16_t speedOut = 100;
    int16_t powerOut = 100;

    // int16_t cadenceIn;
    // int16_t powerIn;

    uint8_t indoorBikeDataCharacteristicData[8] = { // values for setup - little endian order
        (uint8_t)(indoorBikeDataCharacteristicDef & 0xff),
        (uint8_t)(indoorBikeDataCharacteristicDef >> 8),
        (uint8_t)(speedOut & 0xff),
        (uint8_t)(speedOut >> 8),
        (uint8_t)(speedOut & 0xff),
        (uint8_t)(speedOut >> 8),
        0x64,
        0};

    uint8_t fitnessMachineFeatureCharacteristicsData[8] = { // values for setup - little endian order
        (uint8_t)(fitnessMachineFeaturesCharacteristicsDef & 0xff),
        (uint8_t)(fitnessMachineFeaturesCharacteristicsDef >> 8),
        (uint8_t)(fitnessMachineFeaturesCharacteristicsDef >> 16),
        (uint8_t)(fitnessMachineFeaturesCharacteristicsDef >> 24),
        (uint8_t)(targetSettingFeaturesCharacteristicsDef & 0xff),
        (uint8_t)(targetSettingFeaturesCharacteristicsDef >> 8),
        (uint8_t)(targetSettingFeaturesCharacteristicsDef >> 16),
        (uint8_t)(targetSettingFeaturesCharacteristicsDef >> 24)};
};

class MyCallback : public BLECharacteristicCallbacks
{
public:
    MyCallback(ServiceFitness *pFitnessServiceRef, void (*funResistanceCbl)(uint8_t));
    void onWrite(BLECharacteristic *pCharacteristic);

private:
    ServiceFitness *pFitnessService;
    void (*pCallbackSetResistance)(uint8_t) ;
};

#endif
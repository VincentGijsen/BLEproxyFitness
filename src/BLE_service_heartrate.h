#ifndef BLE_service_heartrate_h
#define BLE_service_heartrate_h

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEAdvertising.h>
#include <BLE2902.h>
#include <Arduino.h>

#define SENS_LOC_OTHER 0x00
#define SENS_LOC_CHEST 0x01
#define SENS_LOC_WIRST 0x02
#define SENS_LOC_HAND 0x04
#define SENS_LOC_EARLOBE 0x05
#define SENS_LOC_FOOT 0x06

#define heartrateService BLEUUID((uint16_t)0x180D) //

static BLEUUID heartrateUUID("bc015a62-26a4-4e03-ac68-7e79372a615e"); // hearrate

class ServiceHeartRate
{
public:
    ServiceHeartRate(BLEServer *pServer, BLEAdvertisementData *pAdvertisementData);
    void setupService();
    void notifyHeartSensorPosUpdate(uint8_t bpm);
    void notifyHeartRateUpdate(uint8_t heartRate);

private:
    BLEServer *pServer;
    BLEService *pHeart;
    BLEAdvertisementData *pAdvertisementData;

    BLERemoteCharacteristic *pHeartRateCharacteristic;

    byte flags = 0b00111110;
    bool isInitialized=false;
};

#endif

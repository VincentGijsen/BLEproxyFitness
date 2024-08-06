#include "BLE_service_heartrate.h"

// ServiceHeartRate::ServiceHeartRate()

BLECharacteristic heartRateMeasurementCharacteristics(BLEUUID((uint16_t)0x2A37), BLECharacteristic::PROPERTY_NOTIFY);
BLECharacteristic sensorPositionCharacteristic(BLEUUID((uint16_t)0x2A38), BLECharacteristic::PROPERTY_READ);
BLEDescriptor heartRateDescriptor(BLEUUID((uint16_t)0x2901));
BLEDescriptor sensorPositionDescriptor(BLEUUID((uint16_t)0x2901));

ServiceHeartRate::ServiceHeartRate(BLEServer *pServer, BLEAdvertisementData *pAdvertisementData)
{
    this->pServer = pServer;
    this->pAdvertisementData = pAdvertisementData;
}

void ServiceHeartRate::setupService()
{
    Serial.println("Setup HearRate Service...");
    this->pHeart = this->pServer->createService(heartrateService);

    heartRateMeasurementCharacteristics.addDescriptor(new BLE2902());

    this->pHeart->addCharacteristic(&heartRateMeasurementCharacteristics);

    heartRateDescriptor.setValue("Rate from 0 to 200");
    heartRateMeasurementCharacteristics.addDescriptor(&heartRateDescriptor);

    pHeart->addCharacteristic(&sensorPositionCharacteristic);
    sensorPositionDescriptor.setValue("Position 0 - 6");
    sensorPositionCharacteristic.addDescriptor(&sensorPositionDescriptor);

    this->pServer->getAdvertising()->addServiceUUID(heartrateService);

    this->pHeart->start();
    this->isInitialized = true;
}

void ServiceHeartRate::notifyHeartSensorPosUpdate(uint8_t bpm)
{
    if(!isInitialized){
        return ;
    }
    byte val[1] = {bpm};
    sensorPositionCharacteristic.setValue(val, 1);
    sensorPositionCharacteristic.notify();
}

void ServiceHeartRate::notifyHeartRateUpdate(uint8_t heartRate)
{
    if(!isInitialized){
        return ;
    }
    Serial.println("Sending heartrate update..");

    /*
       Field #1 - Flags (byte)
           Bit 0   - Heart Rate Value Format
                       0 = uint8
                       1 = uint16
           Bit 1-2 - Sensor Contact Status
                       0 - Sensor Contact feature is not supported in the current connection
                       1 - Sensor Contact feature is not supported in the current connection
                       2 - Sensor Contact feature is supported, but contact is not detected
                       3 - Sensor Contact feature is supported and contact is detected
           Bit 3   - Energy Expended Status
                       0 = Energy Expended field is not present
                       1 = Energy Expended field is present. Units: kilo Joules
           Bit 3   - RR-Interval bit
                       0 = RR-Interval values are not present.
                       1 = One or more RR-Interval values are present.
           Bit 5-7 - Reserved
       Field #2 - Heart Rate Measurement Value (uint8)
       Field #3 - Heart Rate Measurement Value (uint16)
       Field #4 - Energy Expended (uint16)
       Field #5 - RR-Interval (uint16)
       */

    // Flags = Format uint16
    byte cscMeasurementFlags = 0b00000001;
    byte value[3] = {cscMeasurementFlags, 0, 0};

    value[1] = heartRate & 0xFF;
    value[2] = (heartRate >> 8) & 0xFF;

    byte hrmPos[1] = {SENS_LOC_HAND};

    heartRateMeasurementCharacteristics.setValue(value, 3);

    heartRateMeasurementCharacteristics.notify();
}

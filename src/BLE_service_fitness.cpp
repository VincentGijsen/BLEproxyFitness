#include "BLE_service_fitness.h"

// required characteristics
BLECharacteristic fitnessMachineFeatureCharacteristics(BLEUUID((uint16_t)0x2ACC), BLECharacteristic::PROPERTY_READ);
BLECharacteristic indoorBikeDataCharacteristic(BLEUUID((uint16_t)0x2AD2), BLECharacteristic::PROPERTY_NOTIFY);
BLECharacteristic resistanceLevelRangeCharacteristic(BLEUUID((uint16_t)0x2AD6), BLECharacteristic::PROPERTY_READ);
BLECharacteristic powerLevelRangeCharacteristic(BLEUUID((uint16_t)0x2AD8), BLECharacteristic::PROPERTY_READ);
BLECharacteristic fitnessMachineControlPointCharacteristic(BLEUUID((uint16_t)0x2AD9), BLECharacteristic::PROPERTY_INDICATE | BLECharacteristic::PROPERTY_WRITE);
BLECharacteristic fitnessMachineStatusCharacteristic(BLEUUID((uint16_t)0x2ADA), BLECharacteristic::PROPERTY_NOTIFY);

BLERemoteCharacteristic *pCadenceCharacteristic;
BLERemoteCharacteristic *pPowerCharacteristic;

MyCallback::MyCallback(ServiceFitness *pFitnessServiceRef, void (*funResistanceCbl)(uint8_t))
{
    pFitnessService = pFitnessServiceRef;
    pCallbackSetResistance = funResistanceCbl;
}

void MyCallback::onWrite(BLECharacteristic *pCharacteristic)
{
    std::string message = pCharacteristic->getValue(); // std::string - very important, as it doesnt stop when getting a byte of zeros
    Serial.print("'");
    for (int i = 0; i < message.length(); i++)
        Serial.print(message[i]);                         // print the message byte by byte - any conversion to a normal String terminates it on a byte of zeros
    Serial.print("'");                                    // for debugging and new messages
    uint8_t value[3] = {0x80, (uint8_t)message[0], 0x02}; // confirmation data, default - 0x02 - Op "Code not supported", no app cares
    uint8_t statusValue[1];
    switch ((uint8_t)message[0])
    {
    case 0x03: // inclination level setting
    {
        uint16_t grade = (message[2] << 8) + message[1];
        grade *= 10;
        pFitnessService->setGrade(grade);
        value[2] = 0x01; // ok sent
        // sim = true;
    }
    break;
    case 0x04: // resistance level setting
    Serial.print("app req setRestitance: ");
    this->pCallbackSetResistance(message[1]);
      //  if( this->pCallbackSetResistance != nullptr){
          
        //};
        pFitnessService->setResistanceSet(message[1] );
        Serial.print("  ");
        Serial.println(message[1], DEC);
        value[2] = 0x01; // ok sent
        // sim = false;
        break;
    case 0x00: // request control
        // Serial.println("control");
        value[2] = 0x01; // ok sent
        // sim = false;
        break;
    case 0x07:           // start the training, not used, just confirmation
        value[2] = 0x01; // ok sent
        // sim = false;
        break;
    case 0x05: // power level setting
        // powerSet = (message[2] << 8) + message[1];
        //  Serial.print("  ");
        //  Serial.println(powerSet);
        value[2] = 0x01; // ok sent
        // sim = false;
        break;
    case 0x11:
    Serial.println("winddspeed set..");
        int16_t windSpeed = (message[2] << 8) + message[1];
        uint16_t grade = (message[4] << 8) + message[3];
        int8_t crr = message[5];
        int8_t cw = message[6];
        //			Serial.print("	");
        //			Serial.print(windSpeed);
        //			Serial.print("	");
        //			Serial.print(grade);
        //			Serial.print("	");
        //			Serial.print(crr);
        //			Serial.print("	");
        //			Serial.print(cw);
        //			Serial.print("	");
        //			Serial.println(message.length());
        pFitnessService->setGrade(grade);
        uint8_t statusValue[7] = {0x12, (uint8_t)message[1], (uint8_t)message[2], (uint8_t)message[3], (uint8_t)message[4], (uint8_t)message[5], (uint8_t)message[6]};
        fitnessMachineStatusCharacteristic.setValue(statusValue, 7); // Fitness Machine Status updatet, no app cares
        // Serial.println("set");
        fitnessMachineStatusCharacteristic.notify();

        value[2] = 0x01; // ok sent
                         // sim = true;
        break;
    }
    pCharacteristic->setValue(value, 3); // response to write
    pCharacteristic->indicate();         // indicate response
}

ServiceFitness::ServiceFitness(BLEServer *pServer, BLEAdvertisementData *pAdvertisementData, void(*funSetResistanceReq)(uint8_t))
{
    this->_pServer = pServer;
    Serial.print("Pointer to pServer: ");
    // Serial.println(&pServer);
    this->_pAdvertisementData = pAdvertisementData;
    this->pCallbackSetResistance = funSetResistanceReq;
}

void ServiceFitness::setupService()
{

    const std::string fitnessData = {0b00000001, 0b00100000, 0b00000000};          // advertising data on "Service Data AD Type" - byte of flags (little endian) and two for Fitness Machine Type (little endian)
                                                                                   // indoor bike supported
    this->_pAdvertisementData->setServiceData(fitnessMachineService, fitnessData); // already includdes Service Data AD Type ID and Fitness Machine Service UUID
                                                                                   // with fitnessData 6 bytes

    this->_pFitness = this->_pServer->createService(fitnessMachineService);
    this->_pFitness->addCharacteristic(&indoorBikeDataCharacteristic);
    indoorBikeDataCharacteristic.addDescriptor(new BLE2902());

    this->_pFitness->addCharacteristic(&fitnessMachineFeatureCharacteristics);
    this->_pFitness->addCharacteristic(&resistanceLevelRangeCharacteristic);

    this->_pFitness->addCharacteristic(&fitnessMachineControlPointCharacteristic);

    BLE2902 *descr = new BLE2902();
    descr->setIndications(1); // default indications on

    fitnessMachineControlPointCharacteristic.addDescriptor(descr);

    this->_pFitness->addCharacteristic(&fitnessMachineStatusCharacteristic);
    fitnessMachineStatusCharacteristic.addDescriptor(new BLE2902());
    MyCallback *cb = new MyCallback(this,pCallbackSetResistance );
    fitnessMachineControlPointCharacteristic.setCallbacks(cb); // set callback for write

    this->_pServer->getAdvertising()->addServiceUUID(fitnessMachineService);

    this->_pFitness->start();

    const uint8_t minPowLev = 1;
   // const uint8_t minPowLev = 16;
    const uint8_t maxResLevel = 16;

    uint8_t powerRange[6] = {
        (uint8_t)(minPowLev & 0xff),
        (uint8_t)(minPowLev >> 8),
        (uint8_t)(maxPowLev & 0xff),
        (uint8_t)(maxPowLev >> 8),
        (uint8_t)(minPowInc & 0xff),
        (uint8_t)(minPowInc >> 8)};

    uint8_t resistanceRange[6] = {
        (uint8_t)(0 & 0xff),
        (uint8_t)(0 >> 8),
        (uint8_t)(maxResLev & 0xff),
        (uint8_t)(maxResLev >> 8),
        (uint8_t)(1 & 0xff),
        (uint8_t)(1 >> 8)};
    resistanceLevelRangeCharacteristic.setValue(resistanceRange, 6);
    powerLevelRangeCharacteristic.setValue(powerRange, 6);
    fitnessMachineFeatureCharacteristics.setValue(fitnessMachineFeatureCharacteristicsData, 8); // flags
}

void ServiceFitness::prepareNotifyFitnessSpeed(uint8_t speed)
{
    Serial.print("Speed updated to: ");
    Serial.println(speed);
    this->indoorBikeDataCharacteristicData[2] = (uint8_t)(speed & 0xff);
    this->indoorBikeDataCharacteristicData[3] = (uint8_t)(speed >> 8); // speed value with little endian order
}

void ServiceFitness::notifyFitnessData()
{
    Serial.println("Sending bikeDataUpdate..");
    indoorBikeDataCharacteristic.setValue(this->indoorBikeDataCharacteristicData, 8); // values sent
    indoorBikeDataCharacteristic.notify();
}

void ServiceFitness::setResistanceSet(uint8_t resistance)
{
    this->resistanceSet = resistance;
}

void ServiceFitness::setGrade(uint8_t grade)
{
    this->grade = grade;
}

void ServiceFitness::setCallbackResitHandler(void (*fun)(uint8_t))
{
   this->pCallbackSetResistance = fun;
}

void ServiceFitness::prepareNotifyFitnessCadence(uint8_t cadence)
{
    Serial.print("cadence updated to: ");
    Serial.println(cadence);
    this->indoorBikeDataCharacteristicData[4] = (uint8_t)((cadence * 2) & 0xff);
    this->indoorBikeDataCharacteristicData[5] = (uint8_t)((cadence * 2) >> 8); // cadence value
}

void ServiceFitness::prepareNotifyFitnessPower(uint8_t power)
{
    Serial.print("power updated to: ");
    Serial.println(power);
    this->indoorBikeDataCharacteristicData[6] = (uint8_t)(constrain(power, 0, 4000) & 0xff);
    this->indoorBikeDataCharacteristicData[7] = (uint8_t)(constrain(power, 0, 4000) >> 8); // power value, constrained to avoid negative values, although the specification allows for a sint16
}

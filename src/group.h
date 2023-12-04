#include <Arduino.h>
#include <EEPROM.h>
#define EXTRACTING 1
#define CLEANING_RUN 2
#define CLEANING_PAUSE 2
#define FREE 0
void writeLongIntoEEPROM(int address, long number)
{
    EEPROM.write(address, (number >> 24) & 0xFF);
    delay(5);
    EEPROM.write(address + 1, (number >> 16) & 0xFF);
    delay(5);
    EEPROM.write(address + 2, (number >> 8) & 0xFF);
    delay(5);
    EEPROM.write(address + 3, number & 0xFF);
    delay(5);
};
long readLongFromEEPROM(int address)
{
    delay(5);
    return ((long)EEPROM.read(address) << 24) +
           ((long)EEPROM.read(address + 1) << 16) +
           ((long)EEPROM.read(address + 2) << 8) +
           (long)EEPROM.read(address + 3);
    delay(5);
};
class CFMCGroup
{
private:
    unsigned long setupKeyHoldTimeStamp = 0;
    bool isExtracting = false;
    unsigned long setupLEDBlinkTimeStamp = 0;
    unsigned long wtfl_timeStamp;
    bool ledBlinkHigh = false;
    bool toggleSetupMode = false;
    bool wtfl_now;
    void startExtracting(int key);
    void stopExtracting(unsigned long millisync_Stop);
    long water_now = 0;
    long water_false_coffee = 0;
    bool true_coffee = false;
    long time_wtfl_decated;
    long time_end_false_coffee;
    float pulsePerTime;
    float pulsePerTime_extracting;
    long water_fast_loss;
    unsigned long start_extrating_timeStamp;
    unsigned long time_extracting;

    int EEPROM_START_ADDRESS;
    long key0_water;
    long key0_time;
    long key0_false_coffee_water;
    long key0_time_end_false_coffee;

    long key1_water;
    long key1_time;
    long key1_false_coffee_water;
    long key1_time_end_false_coffee;

    long key2_water;
    long key2_time;
    long key2_false_coffee_water;
    long key2_time_end_false_coffee;

    long key3_water;
    long key3_time;
    long key3_false_coffee_water;
    long key3_time_end_false_coffee;

public:
    uint8_t groupName;
    uint8_t keySetup = 7;
    bool isKeyHold = false;
    byte key_bit_Register;
    byte *water_flow_Register;
    uint8_t water_flow_bit_order;
    byte led_Register = 0B01001111;
    int solenoid_bit_order;

    byte *relay_register;
    bool *isSetupMode;
    uint8_t *relayRegister;
    bool *blinkPipe;
    uint8_t state = FREE;
    int key_extracting = -1;
    unsigned long holdTimeIntoSetup = 5000;
    CFMCGroup(uint8_t _groupName, int _eeprom_start_address);
    ~CFMCGroup();
    void scan(unsigned long millisSync);
    void loadData();
};

CFMCGroup::CFMCGroup(uint8_t _groupName, int _eeprom_start_address)
{
    groupName = _groupName;
    EEPROM_START_ADDRESS = _eeprom_start_address;
}

CFMCGroup::~CFMCGroup()
{
}
void CFMCGroup::scan(unsigned long millisSync)
{
    if (key_bit_Register != 0)
    {
        switch (key_bit_Register)
        {
        case 0B01000000:
            if (setupKeyHoldTimeStamp == 0)
            {
                setupKeyHoldTimeStamp = millisSync;
            }
            if (key_extracting == -1)
            {

                if (!toggleSetupMode)
                {
                    if (millisSync - setupKeyHoldTimeStamp > holdTimeIntoSetup)
                    {
                        (*isSetupMode) = !(*isSetupMode);
                        toggleSetupMode = 1;
                    }
                }
            }
            break;
        case 0B00000001:
            if (!isKeyHold)
            {
                isKeyHold = 1;
                if (key_extracting == -1)
                {
                    wtfl_timeStamp = millisSync;
                    start_extrating_timeStamp = millisSync;
                    startExtracting(0);
                }
                else if (key_extracting == 0)
                {
                    stopExtracting(millisSync);
                }
            }

            break;
        case 0B00000010:
            if (!isKeyHold)
            {
                isKeyHold = 1;
                if (key_extracting == -1)
                {
                    led_Register = 0B01000010;
                    wtfl_timeStamp = millisSync;
                    start_extrating_timeStamp = millisSync;
                    startExtracting(1);
                }
                else if (key_extracting == 1)
                {
                    stopExtracting(millisSync);
                }
            }

            break;
        case 0B00000100:
            if (!isKeyHold)
            {
                isKeyHold = 1;
                if (key_extracting == -1)
                {
                    led_Register = 0B01000100;
                    wtfl_timeStamp = millisSync;
                    start_extrating_timeStamp = millisSync;
                    startExtracting(2);
                }
                else if (key_extracting == 2)
                {
                    stopExtracting(millisSync);
                }
            }
            break;
        case 0B00001000:
            if (!isKeyHold)
            {
                isKeyHold = 1;
                if (key_extracting == -1)
                {
                    led_Register = 0B01001000;
                    wtfl_timeStamp = millisSync;
                    start_extrating_timeStamp = millisSync;
                    startExtracting(3);
                }
                else if (key_extracting == 3)
                {
                    stopExtracting(millisSync);
                }
            }
            break;
        case 0B01000001:
            if (!(*isSetupMode) && (key_extracting == 0 && key_extracting != 6)) // go to clean mode
            {
                key_extracting = 7;
                state = CLEANING_RUN;
            }
            break;
        default:
            break;
        }
    }
    else // key up
    {
        if (setupKeyHoldTimeStamp && !toggleSetupMode)
        {
            if (key_extracting != -1)
            {
                if (millisSync - setupKeyHoldTimeStamp > 100)
                {
                    stopExtracting(millisSync);
                }
            }
            else
            {
                if (millisSync - setupKeyHoldTimeStamp > 100 && !(*isSetupMode))
                {
                    wtfl_timeStamp = millisSync;
                    startExtracting(6);
                }
            }
        }
        toggleSetupMode = 0;
        setupKeyHoldTimeStamp = 0;
        isKeyHold = 0;
    }
    if ((*isSetupMode) || key_extracting == 7) // blink
    {
        if ((*blinkPipe))
        {
            led_Register = 0B00000000;
            ledBlinkHigh = false;
        }
        else
        {
            if (key_extracting == 7)
            {
                led_Register = 0B01000001;
            }
            else
            {
                led_Register = 0B01001111;
            }
            ledBlinkHigh = true;
        }
    }
    else if (!(*isSetupMode) && key_extracting == -1) // stop extracting
    {
        led_Register = 0B01001111;
    }

    switch (key_extracting) // led extracting
    {
    case 0:
        led_Register = 0B01000001;
        time_extracting = millisSync - start_extrating_timeStamp;
        if ((((*water_flow_Register) >> (water_flow_bit_order)) & 0x01) != wtfl_now)
        {
            time_wtfl_decated = millisSync - wtfl_timeStamp;
            wtfl_timeStamp = millisSync;
            pulsePerTime = 1000.00 / float(time_wtfl_decated);
            wtfl_now = (((*water_flow_Register) >> (water_flow_bit_order)) & 0x01);
            if (true_coffee)
            {
                water_now++;
                pulsePerTime = float(water_now) / float(time_extracting - time_end_false_coffee) * 1000.00;
            }
            else
            {
                water_false_coffee++;
            }
            Serial.print(time_extracting);
            Serial.print(",");
            Serial.print(pulsePerTime);
            Serial.print(",");
            Serial.print(water_false_coffee);
            Serial.print(",");
            Serial.println(water_now);
        }
        if (time_extracting > 200)
        {
            if (!true_coffee)
            {
                if (time_wtfl_decated > 100 && time_extracting > 3500)
                {
                    true_coffee = true;
                    time_end_false_coffee = time_extracting;
                }
                else if (water_false_coffee == (key0_water + key0_false_coffee_water))
                {
                    stopExtracting(millisSync);
                }
            }
            else
            {
                if (!(*isSetupMode))
                {
                    if (water_false_coffee < key0_false_coffee_water)
                    {
                        if (float(time_extracting - time_end_false_coffee) >= float(key0_time * pulsePerTime_extracting / pulsePerTime) + float(float(key0_false_coffee_water-water_false_coffee) / pulsePerTime * 1000.00))
                        {
                            stopExtracting(millisSync);
                        }
                    }
                    else
                    {
                        if (float(time_extracting - time_end_false_coffee) >= float(key0_time * pulsePerTime_extracting / pulsePerTime) - float(float(water_false_coffee - key0_false_coffee_water) / pulsePerTime * 1000.00))
                        {
                            stopExtracting(millisSync);
                        }
                    }
                }
            }
        }

        // if ((((*water_flow_Register) >> (water_flow_bit_order)) & 0x01) != wtfl_now)
        // {
        //     water_now++;
        //     wtfl_now = (((*water_flow_Register) >> (water_flow_bit_order)) & 0x01);
        //     Serial.println(millisSync - wtfl_timeStamp);
        //     wtfl_timeStamp = millisSync;
        //     if (!(*isSetupMode))
        //     {
        //         if (water_now == key0_water)
        //         {
        //             stopExtracting(millisSync);
        //         }
        //     }
        // }
        break;
    case 1:
        led_Register = 0B01000010;
        if ((((*water_flow_Register) >> (water_flow_bit_order)) & 0x01) != wtfl_now)
        {
            water_now++;
            wtfl_now = (((*water_flow_Register) >> (water_flow_bit_order)) & 0x01);
            if (!(*isSetupMode))
            {
                if (water_now == key1_water)
                {
                    stopExtracting(millisSync);
                }
            }
        }
        break;
    case 2:
        led_Register = 0B01000100;
        if ((((*water_flow_Register) >> (water_flow_bit_order)) & 0x01) != wtfl_now)
        {
            water_now++;
            wtfl_now = (((*water_flow_Register) >> (water_flow_bit_order)) & 0x01);
            if (!(*isSetupMode))
            {
                if (water_now == key2_water)
                {
                    stopExtracting(millisSync);
                }
            }
        }
        break;
    case 3:
        led_Register = 0B01001000;
        if ((((*water_flow_Register) >> (water_flow_bit_order)) & 0x01) != wtfl_now)
        {
            water_now++;
            wtfl_now = (((*water_flow_Register) >> (water_flow_bit_order)) & 0x01);
            if (!(*isSetupMode))
            {
                if (water_now == key3_water)
                {
                    stopExtracting(millisSync);
                }
            }
        }
        break;
    case 6:
        led_Register = 0B01000000;
        break;

    default:
        break;
    }
}
void CFMCGroup::startExtracting(int key)
{
    if (solenoid_bit_order)
    {
        Serial.print("|time|"); /// time_extracting
        Serial.print("tpp|");   // time/pulse
        Serial.print("wfp|");   // water_false_coffee
        Serial.println("wn|");  // water_now
        delay(100);
        key_extracting = key;
        (*relay_register) |= (1 << solenoid_bit_order);
        state = EXTRACTING;
        wtfl_now = ((*water_flow_Register) >> (water_flow_bit_order)) & 0x01;
        true_coffee = false;
        water_now = 0;
        water_false_coffee = 0;
        time_end_false_coffee = 0;
        time_extracting = 0;
        water_fast_loss = 0;
        pulsePerTime = 0;

        switch (key)
        {
        case 0:
            pulsePerTime_extracting = float(key0_water) / float(key0_time) * 1000.00;
            break;
        case 1:
            pulsePerTime_extracting = float(key1_water) / float(key1_time) * 1000.00;
            break;
        case 2:
            pulsePerTime_extracting = float(key2_water) / float(key2_time) * 1000.00;
            break;
        case 3:
            pulsePerTime_extracting = float(key3_water) / float(key3_time) * 1000.00;
            break;
        default:
            break;
        }
        Serial.println(pulsePerTime_extracting);
    }
}
void CFMCGroup::stopExtracting(unsigned long millisync_Stop)
{
    if (solenoid_bit_order)
    {
        (*relay_register) &= ~(1 << solenoid_bit_order);
        if ((*isSetupMode))
        {
            switch (key_extracting)
            {
            case 0:
                writeLongIntoEEPROM(EEPROM_START_ADDRESS, water_now);
                writeLongIntoEEPROM(EEPROM_START_ADDRESS + 4, time_extracting - time_end_false_coffee);
                writeLongIntoEEPROM(EEPROM_START_ADDRESS + 8, water_false_coffee);
                writeLongIntoEEPROM(EEPROM_START_ADDRESS + 12, time_end_false_coffee);
                key0_water = water_now;
                key0_time = time_extracting - time_end_false_coffee;
                key0_false_coffee_water = water_false_coffee;
                key0_time_end_false_coffee = time_end_false_coffee;
                break;
            case 1:
                writeLongIntoEEPROM(EEPROM_START_ADDRESS + 16, water_now);
                writeLongIntoEEPROM(EEPROM_START_ADDRESS + 20, time_extracting - time_end_false_coffee);
                writeLongIntoEEPROM(EEPROM_START_ADDRESS + 24, water_false_coffee);
                writeLongIntoEEPROM(EEPROM_START_ADDRESS + 28, time_end_false_coffee);
                key1_water = water_now;
                key1_time = time_extracting - time_end_false_coffee;
                key1_false_coffee_water = water_false_coffee;
                key1_time_end_false_coffee = time_end_false_coffee;
                break;
            case 2:
                writeLongIntoEEPROM(EEPROM_START_ADDRESS + 32, water_now);
                writeLongIntoEEPROM(EEPROM_START_ADDRESS + 36, time_extracting - time_end_false_coffee);
                writeLongIntoEEPROM(EEPROM_START_ADDRESS + 40, water_false_coffee);
                writeLongIntoEEPROM(EEPROM_START_ADDRESS + 44, time_end_false_coffee);
                key2_water = water_now;
                key2_time = time_extracting - time_end_false_coffee;
                key2_false_coffee_water = water_false_coffee;
                key2_time_end_false_coffee = time_end_false_coffee;
                break;
            case 3:
                writeLongIntoEEPROM(EEPROM_START_ADDRESS + 48, water_now);
                writeLongIntoEEPROM(EEPROM_START_ADDRESS + 52, time_extracting - time_end_false_coffee);
                writeLongIntoEEPROM(EEPROM_START_ADDRESS + 56, water_false_coffee);
                writeLongIntoEEPROM(EEPROM_START_ADDRESS + 60, time_end_false_coffee);
                key3_water = water_now;
                key3_time = time_extracting - time_end_false_coffee;
                key3_false_coffee_water = water_false_coffee;
                key3_time_end_false_coffee = time_end_false_coffee;
                break;
            default:
                break;
            }
        }
        state = FREE;

        key_extracting = -1;
        water_now = 0;
        water_false_coffee = 0;
        true_coffee = false;
        time_end_false_coffee = 0;
        time_extracting = 0;
        water_fast_loss = 0;
        pulsePerTime = 0;
    }
}
void CFMCGroup::loadData()
{
    key0_water = readLongFromEEPROM(EEPROM_START_ADDRESS);
    key0_time = readLongFromEEPROM(EEPROM_START_ADDRESS + 4);
    key0_false_coffee_water = readLongFromEEPROM(EEPROM_START_ADDRESS + 8);
    key0_time_end_false_coffee = readLongFromEEPROM(EEPROM_START_ADDRESS + 12);

    key1_water = readLongFromEEPROM(EEPROM_START_ADDRESS + 16);
    key1_time = readLongFromEEPROM(EEPROM_START_ADDRESS + 20);
    key1_false_coffee_water = readLongFromEEPROM(EEPROM_START_ADDRESS + 24);
    key1_time_end_false_coffee = readLongFromEEPROM(EEPROM_START_ADDRESS + 28);

    key2_water = readLongFromEEPROM(EEPROM_START_ADDRESS + 32);
    key2_time = readLongFromEEPROM(EEPROM_START_ADDRESS + 36);
    key2_false_coffee_water = readLongFromEEPROM(EEPROM_START_ADDRESS + 40);
    key2_time_end_false_coffee = readLongFromEEPROM(EEPROM_START_ADDRESS + 44);

    key3_water = readLongFromEEPROM(EEPROM_START_ADDRESS + 48);
    key3_time = readLongFromEEPROM(EEPROM_START_ADDRESS + 52);
    key3_false_coffee_water = readLongFromEEPROM(EEPROM_START_ADDRESS + 56);
    key3_time_end_false_coffee = readLongFromEEPROM(EEPROM_START_ADDRESS + 60);

    if (key0_water == -1)
    {
        writeLongIntoEEPROM(EEPROM_START_ADDRESS, 131);
        key0_water = 135;
    }
    if (key0_time == -1)
    {
        writeLongIntoEEPROM(EEPROM_START_ADDRESS + 4, 24624);
        key0_time = 17525;
    }
    if (key0_false_coffee_water == -1)
    {
        writeLongIntoEEPROM(EEPROM_START_ADDRESS + 8, 166);
        key0_false_coffee_water = 195;
    }
    if (key0_time_end_false_coffee == -1)
    {
        writeLongIntoEEPROM(EEPROM_START_ADDRESS + 12, 5200);
    }

    if (key1_water == -1)
    {
        writeLongIntoEEPROM(EEPROM_START_ADDRESS + 16, 131);
        key1_water = 131;
    }
    if (key1_time == -1)
    {
        writeLongIntoEEPROM(EEPROM_START_ADDRESS + 20, 24624);
        key1_time = 24624;
    }
    if (key1_false_coffee_water == -1)
    {
        writeLongIntoEEPROM(EEPROM_START_ADDRESS + 24, 166);
        key1_false_coffee_water = 166;
    }
    if (key1_time_end_false_coffee == -1)
    {
        writeLongIntoEEPROM(EEPROM_START_ADDRESS + 28, 5200);
    }

    if (key2_water == -1)
    {
        writeLongIntoEEPROM(EEPROM_START_ADDRESS + 32, 131);
        key2_water = 131;
    }
    if (key2_time == -1)
    {
        writeLongIntoEEPROM(EEPROM_START_ADDRESS + 36, 24624);
        key2_time = 24624;
    }
    if (key2_false_coffee_water == -1)
    {
        writeLongIntoEEPROM(EEPROM_START_ADDRESS + 40, 166);
        key2_false_coffee_water = 166;
    }
    if (key2_time_end_false_coffee == -1)
    {
        writeLongIntoEEPROM(EEPROM_START_ADDRESS + 44, 5200);
    }

    if (key3_water == -1)
    {
        writeLongIntoEEPROM(EEPROM_START_ADDRESS + 48, 131);
        key3_water = 131;
    }
    if (key3_time == -1)
    {
        writeLongIntoEEPROM(EEPROM_START_ADDRESS + 52, 24624);
        key3_time = 24624;
    }
    if (key3_false_coffee_water == -1)
    {
        writeLongIntoEEPROM(EEPROM_START_ADDRESS + 56, 166);
        key3_false_coffee_water = 166;
    }
    if (key3_time_end_false_coffee == -1)
    {
        writeLongIntoEEPROM(EEPROM_START_ADDRESS + 60, 5200);
    }
}
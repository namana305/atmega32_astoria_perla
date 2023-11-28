#include <Arduino.h>

#define EXTRACTING 1
#define CLEANING_RUN 2
#define CLEANING_PAUSE 2
#define FREE 0

class CFMCGroup
{
private:
    unsigned long setupKeyHoldTimeStamp = 0;
    bool isExtracting = false;
    unsigned long setupLEDBlinkTimeStamp = 0;
    bool ledBlinkHigh = false;
    bool toggleSetupMode = false;
    void startExtracting();
    void stopExtracting();

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
    CFMCGroup(uint8_t _groupName);
    ~CFMCGroup();
    void scan(unsigned long millisSync);
};

CFMCGroup::CFMCGroup(uint8_t _groupName)
{
    groupName = _groupName;
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
                    key_extracting = 0;
                    startExtracting();
                    state = EXTRACTING;
                }
                else if (key_extracting == 0)
                {
                    key_extracting = -1;
                    stopExtracting();
                    state = FREE;
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
                    key_extracting = 1;
                    startExtracting();
                    state = EXTRACTING;
                }
                else if (key_extracting == 1)
                {
                    key_extracting = -1;
                    stopExtracting();
                    state = FREE;
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
                    key_extracting = 2;
                    startExtracting();
                    state = EXTRACTING;
                }
                else if (key_extracting == 2)
                {
                    key_extracting = -1;
                    stopExtracting();
                    state = FREE;
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
                    key_extracting = 3;
                    startExtracting();
                    state = EXTRACTING;
                }
                else if (key_extracting == 3)
                {
                    key_extracting = -1;
                    stopExtracting();
                    state = FREE;
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
                    key_extracting = -1;
                    stopExtracting();
                    state = FREE;
                }
            }
            else
            {
                if (millisSync - setupKeyHoldTimeStamp > 100 && !(*isSetupMode))
                {
                    key_extracting = 6;
                    startExtracting();
                    state = EXTRACTING;
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
        break;
    case 1:
        led_Register = 0B01000010;
        break;
    case 2:
        led_Register = 0B01000100;
        break;
    case 3:
        led_Register = 0B01001000;
        break;
    case 6:
        led_Register = 0B01000000;
        break;

    default:
        break;
    }
}
void CFMCGroup::startExtracting()
{
    if (solenoid_bit_order)
    {
        (*relay_register) |= (1 << solenoid_bit_order);
    }
}
void CFMCGroup::stopExtracting()
{
    if (solenoid_bit_order)
    {
        (*relay_register) &= ~(1 << solenoid_bit_order);
    }
}

#include <Arduino.h>
#include <138.h>
#include <group.h>
#include <MsTimer2.h>
// Pin Numbers
// ********************************
//                          +---\/---+
//  	   (XCK/T0) D0 PB0  01|        |40  PA0 AI7 D31 (ADC0)
//  	       (T1) D1 PB1  02|        |39  PA1 AI6 D30 (ADC1)
//  	(INT2/AIN0) D2 PB2  03|        |38  PA2 AI5 D29 (ADC2)
//  	 (OC0/AIN1) D3 PB3  04|        |37  PA3 AI4 D28 (ADC3)
//  	       (SS) D4 PB4  05|        |36  PA4 AI3 D27 (ADC4)
//  	     (MOSI) D5 PB5  06|        |35  PA5 AI2 D26 (ADC5)
//  	     (MISO) D6 PB6  07|        |34  PA6 AI1 D25 (ADC6)
//  	      (SCK) D7 PB7  08|        |33  PA7 AI0 D24 (ADC7)
//                 RESET  09|        |32  AREF
//                   VCC  10|        |31  GND
//                   GND  11|        |30  AVCC
//                 XTAL2  12|        |29  PC7 D23 (TOSC2)
//                 XTAL1  13|        |28  PC6 D22 (TOSC1)
//          (RXD) D8 PD0  14|        |27  PC5 D21 (TDI)
//          (TXD) D9 PD1  15|        |26  PC4 D20 (TDO)
//        (INT0) D10 PD2  16|        |25  PC3 D19 (TMS)
//        (INT1) D11 PD3  17|        |24  PC2 D18 (TCK)
//        (OC1B) D12 PD4  18|        |23  PC1 D17 (SDA)
//        (OC1A) D13 PD5  19|        |22  PC0 D16 (SCL)
//        (ICP1) D14 PD6  20|        |21  PD7 D15 (OC2)
//                          +--------+
//--------------------------------------------------------------------
//------------------------------PORTD---------------------------------
//------7-------6-------5-------4-------3-------2--------1--------0---
//---0B 0       0       0       0       0       0        0        0---
//-----D15-----D14-----D13-----D12-----D11-----D10-------D9-------D8--
//------↓------↓--------↓-------↓-------↓-------↓---------↓-------↓---
//------↓------↓--------↓-|HC138_A0|HC138_A1|HC138_A2|--|TXD|---|RXD|-

CFMCGroup group0(0);
CFMCGroup group1(1);
CFMCGroup group2(2);
CFMCGroup group3(3);
byte buff_gr0_key;
byte buff_gr1_key;
byte buff_gr2_key;
byte buff_gr3_key;

byte _relay_register;
byte buff_relay_register = _relay_register;
byte _wtfl_register;
byte buff_wtfl_register = _wtfl_register;

int buff_wt_level;
int wtfl_level;
bool _isSetupMode = false;
void scan1_3__1234();
void scan1_3__567();
void scan2_4__1234();
void scan2_4__567();

unsigned long start;
unsigned long millisSync;
unsigned long microSync;
unsigned long scanTimeStamp;

int scanStep = 0;
unsigned long diviScanTimeStamp;
bool _blinkPipe = false;
void scanLed();
void turnOffAllLEDs();
void mainLoop();
void test()
{
  Serial.println("test");
}

void setup()
{
  DDRD = 0B00011110;
  // put your setup code here, to run once:
  Serial.begin(115200);
  // led
  DDRC = 0B11111111;
  PORTC = 0B00000000;

  PORTD &= ~0B00001000;
  PORTD &= ~0B00011000;
  PORTD |= 0B00011100;

  group0.isSetupMode = &_isSetupMode;
  group1.isSetupMode = &_isSetupMode;
  group2.isSetupMode = &_isSetupMode;
  group3.isSetupMode = &_isSetupMode;

  group0.blinkPipe = &_blinkPipe;
  group1.blinkPipe = &_blinkPipe;
  group2.blinkPipe = &_blinkPipe;
  group3.blinkPipe = &_blinkPipe;

  group0.relay_register = &_relay_register;
  group1.relay_register = &_relay_register;
  group2.relay_register = &_relay_register;
  group3.relay_register = &_relay_register;

  group0.solenoid_bit_order = 5;
  group1.solenoid_bit_order = 1;
  group2.solenoid_bit_order = 2;
  group3.solenoid_bit_order = -1;

  group0.water_flow_Register = &_wtfl_register;
  group1.water_flow_Register = &_wtfl_register;
  group2.water_flow_Register = &_wtfl_register;
  group3.water_flow_Register = &_wtfl_register;

  MsTimer2::set(1, scanLed);
  MsTimer2::start();
}

void loop()
{
  millisSync = millis();
  mainLoop();
}
void scan1_3__1234()
{
  DDRC = 0B11111111;
  _NOP();
  PORTC = 0B00000001;
  _NOP();
  PORTD &= ~0B00001000;
  PORTD &= ~0B00011000;
  PORTD |= 0B00011100;

  PORTC = ~((group0.led_Register & 0B00001111) | (group2.led_Register & 0B00001111) << 4);
  _NOP();
  PORTD &= ~0B00011000;
  PORTD |= 0B00011100;

  _NOP();
  DDRC = 0B00000000;
  _NOP();
  PORTC = 0B00000000;
  _NOP();
  (*portInputRegister(digitalPinToPort(PIN_PC7)) & digitalPinToBitMask(PIN_PC7)) ? (group2.key_bit_Register) |= (1 << (3)) : (group2.key_bit_Register) &= ~(1 << (3));

  ((PINC & (1 << PINC6)) == (1 << PINC6)) ? (group2.key_bit_Register) |= (1 << (2)) : (group2.key_bit_Register) &= ~(1 << (2));
  ((PINC & (1 << PINC5)) == (1 << PINC5)) ? (group2.key_bit_Register) |= (1 << (1)) : (group2.key_bit_Register) &= ~(1 << (1));
  ((PINC & (1 << PINC4)) == (1 << PINC4)) ? (group2.key_bit_Register) |= (1 << (0)) : (group2.key_bit_Register) &= ~(1 << (0));
  ((PINC & (1 << PINC3)) == (1 << PINC3)) ? (group0.key_bit_Register) |= (1 << (3)) : (group0.key_bit_Register) &= ~(1 << (3));
  ((PINC & (1 << PINC2)) == (1 << PINC2)) ? (group0.key_bit_Register) |= (1 << (2)) : (group0.key_bit_Register) &= ~(1 << (2));
  ((PINC & (1 << PINC1)) == (1 << PINC1)) ? (group0.key_bit_Register) |= (1 << (1)) : (group0.key_bit_Register) &= ~(1 << (1));
  ((PINC & (1 << PINC0)) == (1 << PINC0)) ? (group0.key_bit_Register) |= (1 << (0)) : (group0.key_bit_Register) &= ~(1 << (0));
  _NOP();
}
void scan1_3__567()
{

  DDRC = 0B11111111;
  _NOP();
  PORTC = 0B00000010;
  _NOP();
  PORTD &= ~0B00001000;
  PORTD &= ~0B00011000;
  PORTD |= 0B00011100;

  PORTC = ~(group0.led_Register >> 4 | (group2.led_Register >> 4 << 4));
  _NOP();
  PORTD &= ~0B00011000;
  PORTD |= 0B00011100;
  // delayMicroseconds(100);

  _NOP();
  DDRC = 0B00000000;
  _NOP();
  PORTC = 0B00000000;
  _NOP();
  (*portInputRegister(digitalPinToPort(PIN_PC7)) & digitalPinToBitMask(PIN_PC7)) ? (group2.key_bit_Register) |= (1 << (6)) : (group2.key_bit_Register) &= ~(1 << (6));
  //((PINC & (1 << PINC6)) == (1 << PINC6)) ? (group2.key_bit_Register) |= (1 << (7)) : (group2.key_bit_Register) &= ~(1 << (7));
  ((PINC & (1 << PINC5)) == (1 << PINC5)) ? (group2.key_bit_Register) |= (1 << (5)) : (group2.key_bit_Register) &= ~(1 << (5));
  ((PINC & (1 << PINC4)) == (1 << PINC4)) ? (group2.key_bit_Register) |= (1 << (4)) : (group2.key_bit_Register) &= ~(1 << (4));

  //((PINC & (1 << PINC3)) == (1 << PINC3)) ? (group0.key_bit_Register) |= (1 << (7)) : (group0.key_bit_Register) &= ~(1 << (7));
  ((PINC & (1 << PINC2)) == (1 << PINC2)) ? (group0.key_bit_Register) |= (1 << (6)) : (group0.key_bit_Register) &= ~(1 << (6));
  ((PINC & (1 << PINC1)) == (1 << PINC1)) ? (group0.key_bit_Register) |= (1 << (5)) : (group0.key_bit_Register) &= ~(1 << (5));
  ((PINC & (1 << PINC0)) == (1 << PINC0)) ? (group0.key_bit_Register) |= (1 << (4)) : (group0.key_bit_Register) &= ~(1 << (4));
  _NOP();
}
void scan2_4__1234()
{
  DDRC = 0B11111111;
  _NOP();
  PORTC = 0B00000100;
  _NOP();
  PORTD &= ~0B00001000;
  PORTD &= ~0B00011000;
  PORTD |= 0B00011100;

  PORTC = ~((group1.led_Register & 0B00001111) | (group3.led_Register & 0B00001111) << 4);
  _NOP();
  PORTD &= ~0B00011000;
  PORTD |= 0B00011100;

  _NOP();
  DDRC = 0B00000000;
  _NOP();
  PORTC = 0B00000000;
  _NOP();
  (*portInputRegister(digitalPinToPort(PIN_PC7)) & digitalPinToBitMask(PIN_PC7)) ? (group3.key_bit_Register) |= (1 << (3)) : (group3.key_bit_Register) &= ~(1 << (3));
  ((PINC & (1 << PINC6)) == (1 << PINC6)) ? (group3.key_bit_Register) |= (1 << (2)) : (group3.key_bit_Register) &= ~(1 << (2));
  ((PINC & (1 << PINC5)) == (1 << PINC5)) ? (group3.key_bit_Register) |= (1 << (1)) : (group3.key_bit_Register) &= ~(1 << (1));
  ((PINC & (1 << PINC4)) == (1 << PINC4)) ? (group3.key_bit_Register) |= (1 << (0)) : (group3.key_bit_Register) &= ~(1 << (0));
  ((PINC & (1 << PINC3)) == (1 << PINC3)) ? (group1.key_bit_Register) |= (1 << (3)) : (group1.key_bit_Register) &= ~(1 << (3));
  ((PINC & (1 << PINC2)) == (1 << PINC2)) ? (group1.key_bit_Register) |= (1 << (2)) : (group1.key_bit_Register) &= ~(1 << (2));
  ((PINC & (1 << PINC1)) == (1 << PINC1)) ? (group1.key_bit_Register) |= (1 << (1)) : (group1.key_bit_Register) &= ~(1 << (1));
  ((PINC & (1 << PINC0)) == (1 << PINC0)) ? (group1.key_bit_Register) |= (1 << (0)) : (group1.key_bit_Register) &= ~(1 << (0));
  _NOP();
}
void scan2_4__567()
{

  DDRC = 0B11111111;
  _NOP();
  PORTC = 0B00001000;
  _NOP();
  PORTD &= ~0B00001000;
  PORTD &= ~0B00011000;
  PORTD |= 0B00011100;

  PORTC = ~(group1.led_Register >> 4 | (group3.led_Register >> 4 << 4));
  _NOP();
  PORTD &= ~0B00011000;
  PORTD |= 0B00011100;

  _NOP();
  DDRC = 0B00000000;
  _NOP();
  PORTC = 0B00000000;
  _NOP();
  (*portInputRegister(digitalPinToPort(PIN_PC7)) & digitalPinToBitMask(PIN_PC7)) ? (group3.key_bit_Register) |= (1 << (6)) : (group3.key_bit_Register) &= ~(1 << (6));
  ((PINC & (1 << PINC5)) == (1 << PINC5)) ? (group3.key_bit_Register) |= (1 << (5)) : (group3.key_bit_Register) &= ~(1 << (5));
  ((PINC & (1 << PINC4)) == (1 << PINC4)) ? (group3.key_bit_Register) |= (1 << (4)) : (group3.key_bit_Register) &= ~(1 << (4));

  ((PINC & (1 << PINC2)) == (1 << PINC2)) ? (group1.key_bit_Register) |= (1 << (6)) : (group1.key_bit_Register) &= ~(1 << (6));
  ((PINC & (1 << PINC1)) == (1 << PINC1)) ? (group1.key_bit_Register) |= (1 << (5)) : (group1.key_bit_Register) &= ~(1 << (5));
  ((PINC & (1 << PINC0)) == (1 << PINC0)) ? (group1.key_bit_Register) |= (1 << (4)) : (group1.key_bit_Register) &= ~(1 << (4));
  _NOP();
}
void turnOffAllLEDs()
{
  DDRC = 0B11111111;
  _NOP();
  PORTC = 0B11111111;
  _NOP();
  PORTD &= ~0B00001000;
  PORTD &= ~0B00011000;
  PORTD |= 0B00011100;
}
void mainLoop()
{
  cli();
  //   send relay
  DDRB = 0B11111111;
  PORTB = _relay_register;

  PORTD &= ~0B00001100;
  PORTD |= 0B00011100;
  PORTD &= ~0B00010100;
  PORTD |= 0B00011100;
  // read wtfl
  DDRB = 0B00000000;
  PORTB = 0B00000000;
  PORTD &= ~0B00000100;

  _NOP();
  ((PINB & (1 << PINB7)) == (1 << PINB7)) ? (_wtfl_register) |= (1 << (7)) : (_wtfl_register) &= ~(1 << (7));
  ((PINB & (1 << PINB6)) == (1 << PINB6)) ? (_wtfl_register) |= (1 << (6)) : (_wtfl_register) &= ~(1 << (6));
  ((PINB & (1 << PINB5)) == (1 << PINB5)) ? (_wtfl_register) |= (1 << (5)) : (_wtfl_register) &= ~(1 << (5));
  ((PINB & (1 << PINB4)) == (1 << PINB4)) ? (_wtfl_register) |= (1 << (4)) : (_wtfl_register) &= ~(1 << (4));
  ((PINB & (1 << PINB3)) == (1 << PINB3)) ? (_wtfl_register) |= (1 << (3)) : (_wtfl_register) &= ~(1 << (3));
  ((PINB & (1 << PINB2)) == (1 << PINB2)) ? (_wtfl_register) |= (1 << (2)) : (_wtfl_register) &= ~(1 << (2));
  ((PINB & (1 << PINB1)) == (1 << PINB1)) ? (_wtfl_register) |= (1 << (1)) : (_wtfl_register) &= ~(1 << (1));
  ((PINB & (1 << PINB0)) == (1 << PINB0)) ? (_wtfl_register) |= (1 << (0)) : (_wtfl_register) &= ~(1 << (0));
  PORTD |= 0B00011100;

  group0.scan(millisSync);
  group1.scan(millisSync);
  group2.scan(millisSync);
  group3.scan(millisSync);
  if (buff_gr0_key != group0.key_bit_Register)
  {
    Serial.print("GR1:");
    Serial.println(group0.key_bit_Register, BIN);
    buff_gr0_key = group0.key_bit_Register;
  }
  if (buff_gr1_key != group1.key_bit_Register)
  {
    Serial.print("GR2:");
    Serial.println(group1.key_bit_Register, BIN);
    buff_gr1_key = group1.key_bit_Register;
  }
  if (buff_gr2_key != group2.key_bit_Register)
  {
    Serial.print("GR3:");
    Serial.println(group2.key_bit_Register, BIN);
    buff_gr2_key = group2.key_bit_Register;
  }
  if (buff_gr3_key != group3.key_bit_Register)
  {
    Serial.print("GR4:");
    Serial.println(group3.key_bit_Register, BIN);
    buff_gr3_key = group3.key_bit_Register;
  }
  if (buff_wtfl_register != _wtfl_register)
  {
    Serial.print("WTFL:");
    Serial.println(_wtfl_register, BIN);
    buff_wtfl_register = _wtfl_register;
  }
  if (buff_relay_register != _relay_register)
  {
    Serial.print("RELAY:");
    Serial.println(_relay_register, BIN);
    buff_relay_register = _relay_register;
  }
  if (millisSync - start > 500)
  {
    wtfl_level = analogRead(A0);
    if (wtfl_level != buff_wt_level)
    {
      Serial.print("ADC0:");
      Serial.println(wtfl_level);
      buff_wt_level = wtfl_level;
    }
    _blinkPipe = !_blinkPipe;
    start = millisSync;
  }

  if ((group0.state == EXTRACTING || group0.state == CLEANING_RUN) || (group1.state == EXTRACTING || group1.state == CLEANING_RUN) || (group2.state == EXTRACTING || group2.state == CLEANING_RUN) || (group3.state == EXTRACTING || group3.state == CLEANING_RUN))
  {
    _relay_register |= (1 << 0);
  }
  else
  {
    _relay_register &= ~(1 << 0);
  }
  sei();
}
void scanLed()
{
  switch (scanStep)
  {
  case 0:
    scan1_3__1234();
    scanStep++;
    break;
  case 1:
    turnOffAllLEDs();
    scanStep++;
    break;
  case 2:
    scan1_3__567();
    scanStep++;
    break;
  case 3:
    turnOffAllLEDs();
    scanStep++;
    break;
  case 4:
    scan2_4__1234();
    scanStep++;
    break;
  case 5:
    turnOffAllLEDs();
    scanStep++;
    break;
  case 6:
    scan2_4__567();
    scanStep++;
    break;
  case 7:
    turnOffAllLEDs();
    scanStep++;
    break;
  case 8:
    scanStep = 0;
    break;
  default:
    break;
  }
}
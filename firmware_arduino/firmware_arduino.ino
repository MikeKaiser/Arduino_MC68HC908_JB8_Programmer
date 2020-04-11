#include <Arduino.h>

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#error Unsupported CPU. Only ATmega328P is supported
#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
#define   ledPin 13
#define   D9Pin 13
#define   HC908_RST  2
#define   HC908_PTA0 3   // This is PD3(pin5) on the ATmeg328P chip so we read it from PORTD directly in time critical bits
#define   HC908_PTA1 4
#define   HC908_PTA2 5
#define   HC908_PTA3 6
#elif defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
#error Unsupported CPU. Only ATmega328P is supported
#else
#error Unsupported CPU. Only ATmega328P is supported
#endif



void SetTimer1ForISR()
{
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A = 3;            // compare match register 16MHz/256/2Hz = 31250
  TCCR1B |= (1 << WGM12);   // CTC mode
  //TCCR1B |= (1 << CS12);    // 256 prescaler 
  TCCR1B |= (1 << CS10);    // 1 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
}

// From the ATmega368P datasheet we see the following.
// ISR Vectors   Program Address   Name
// 11            0x0014            TIMER1_CAPT_vect
// 12            0x0016            TIMER1_COMPA_vect
// 13            0x0018            TIMER1_COMPB_vect
// 14            0x001A            TIMER1_OVF_vect
ISR(TIMER1_COMPA_vect)          // timer compare interrupt service routine
{
  OCR1A = 3;
  digitalWrite(ledPin, digitalRead(ledPin) ^ 1);   // toggle LED pin
}


void SetTimer1ForPWMOnD9()
{
  //set timer1 toggle at xHz
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1C = 0;
  
  TCNT1  = 0;//initialize counter value to 0

  ICR1 = 4 ;    // set TOP (this gives us 3.125MHz)
  OCR1A = 1 ;   // set duty cycle
  OCR1B = 0 ;

  int WGMode = 14; //(Fast PWM using ICR1 register)
  TCCR1A = (TCCR1A & ~0x03) | (WGMode & 0x03); // lower 2 bits go into TCCR1A
  TCCR1B = (TCCR1B & ~0x18) | ((WGMode & 0x0C) << 1); // upper 2 bits go into  TCCR1B

  int CmpOutModeA = 2; // Clear OC1A (Arduino Pin9) on compare match
  TCCR1A = (TCCR1A & ~0xC0) | ( CmpOutModeA << 6 );

  int CmpOutModeB = 3; // Set OC1B (Arduino Pin10) on compare match, Clear OC1B at BOTTOM (This mode is glitch-less, mode 2 glitches)
  TCCR1A = (TCCR1A & ~0x30) | ( CmpOutModeB << 4 );

  int timerClockSrc = 1; // ioClk/1 (no prescale)
  TCCR1B = (TCCR1B & ~0x07) | (timerClockSrc & 0x07);
}

void SetPTA0AsInput()
{
  digitalWrite( HC908_PTA0, 0 );
  pinMode( HC908_PTA0, INPUT );
}

void SetPTA0Low()
{
  digitalWrite( HC908_PTA0, 0 );
  pinMode( HC908_PTA0, OUTPUT );
}

void SetPTA0High()
{
  SetPTA0AsInput();
}

void WriteBit( unsigned char b )
{
  if( b == 0 )
    SetPTA0Low();
  else
    SetPTA0High();
}

#define BIT_TIME 147
#define BIT_READ_HALF_TIME 78
#define BIT_READ_TIME 155
volatile int cnt;
void WaitBitTime()
{
  // At 6Mhz, 9600bps is 6000000 / 9600 =  625   Cycles per bit   (104.166uS per bit)
  // At 6Mhz, 4800bps is 6000000 / 4800 = 1250   Cycles per bit   (208.333uS per bit)
  // At 3Mhz, 9600bps is 3000000 / 9600 =  312.5 Cycles per bit   (104.166uS per bit)
  // At 3Mhz, 4800bps is 3000000 / 4800 =  625   Cycles per bit   (208.333uS per bit)
  // Once I got the CPU replying to our initial security bytes
  // (by just sending 0xFF repeated so that we got a single low start bit and 9 high bits),
  // I tuned this value manually so that our start time matched the length of the replied start bit
  for( cnt=0; cnt<BIT_TIME; ++cnt )
    __asm__("nop\n\t");
}

void WaitReadHalfBitTime()
{
  for( cnt=0; cnt<BIT_READ_HALF_TIME; ++cnt )
    __asm__("nop\n\t");
}

void WaitReadBitTime()
{
  for( cnt=0; cnt<BIT_READ_TIME; ++cnt )
    __asm__("nop\n\t");
}

void WriteByte( unsigned char v )
{
  digitalWrite( HC908_PTA2, 1 );
  WriteBit( 0 );  // Startbit
  WaitBitTime();
  WriteBit( v & 1 );
  WaitBitTime();
  WriteBit( v & 2 );
  WaitBitTime();
  WriteBit( v & 4 );
  WaitBitTime();
  WriteBit( v & 8 );
  WaitBitTime();
  WriteBit( v & 16 );
  WaitBitTime();
  WriteBit( v & 32 );
  WaitBitTime();
  WriteBit( v & 64 );
  WaitBitTime();
  WriteBit( v & 128 );
  WaitBitTime();
  WriteBit( 1 );  // Stopbit
  WaitBitTime();
  SetPTA0AsInput();
  digitalWrite( HC908_PTA2, 0 );
}

void WriteBreak()
{
  digitalWrite( HC908_PTA2, 1 );
  WriteBit( 0 );  // Startbit
  WaitBitTime();
  WriteBit( 0 );
  WaitBitTime();
  WriteBit( 0 );
  WaitBitTime();
  WriteBit( 0 );
  WaitBitTime();
  WriteBit( 0 );
  WaitBitTime();
  WriteBit( 0 );
  WaitBitTime();
  WriteBit( 0 );
  WaitBitTime();
  WriteBit( 0 );
  WaitBitTime();
  WriteBit( 0 );
  WaitBitTime();
  WriteBit( 0 );  // Stopbit (on a break, the stop bit is also zero)
  WaitBitTime();
  SetPTA0AsInput();
  digitalWrite( HC908_PTA2, 0 );
}

bool ReadByte( uint8_t * ret )
{
  unsigned int cnt = 0;

  *ret = 0;
  
  // wait for PTA0 (ATmega328P PORTD bit 3) to go low, then wait 1.5 bit times before reading 8 bits
  //PORTD |= 1<<HC908_PTA1; PORTD &= ~(1<<HC908_PTA1);
  volatile unsigned char pta0 = PIND;
  while( (pta0 & (1<<HC908_PTA0)) != 0 )
  {
    pta0 = PIND;
    if( (cnt++) == 65535 )
      return false;
  }
  //PORTD |= 1<<HC908_PTA1; PORTD &= ~(1<<HC908_PTA1);
  
  WaitReadHalfBitTime();
  WaitReadBitTime();
  if( PIND & (1<<HC908_PTA0) )  *ret |= 1;
  PORTD |= 1<<HC908_PTA1; PORTD &= ~(1<<HC908_PTA1);
  WaitReadBitTime();
  if( PIND & (1<<HC908_PTA0) )  *ret |= 2;
  PORTD |= 1<<HC908_PTA1; PORTD &= ~(1<<HC908_PTA1);
  WaitReadBitTime();
  if( PIND & (1<<HC908_PTA0) )  *ret |= 4;
  PORTD |= 1<<HC908_PTA1; PORTD &= ~(1<<HC908_PTA1);
  WaitReadBitTime();
  if( PIND & (1<<HC908_PTA0) )  *ret |= 8;
  PORTD |= 1<<HC908_PTA1; PORTD &= ~(1<<HC908_PTA1);
  WaitReadBitTime();
  if( PIND & (1<<HC908_PTA0) )  *ret |= 16;
  PORTD |= 1<<HC908_PTA1; PORTD &= ~(1<<HC908_PTA1);
  WaitReadBitTime();
  if( PIND & (1<<HC908_PTA0) )  *ret |= 32;
  PORTD |= 1<<HC908_PTA1; PORTD &= ~(1<<HC908_PTA1);
  WaitReadBitTime();
  if( PIND & (1<<HC908_PTA0) )  *ret |= 64;
  PORTD |= 1<<HC908_PTA1; PORTD &= ~(1<<HC908_PTA1);
  WaitReadBitTime();
  if( PIND & (1<<HC908_PTA0) )  *ret |= 128;
  PORTD |= 1<<HC908_PTA1; PORTD &= ~(1<<HC908_PTA1);
  WaitReadBitTime();
  WaitReadHalfBitTime();
  return true;
}

bool ReadEcho( uint8_t val )
{
  uint8_t rx;
  if( !ReadByte( &rx ) )
    return false;
  return rx == val;
}

bool WriteByteWithEcho( uint8_t data )
{
  WriteByte( data );
  return ReadEcho( data );
}


bool ReadMemory( unsigned char addrH, unsigned char addrL, uint8_t * data )
{
  if( !WriteByteWithEcho( 0x4A ) ) //ASCII 'J'
    return false;
  if( !WriteByteWithEcho( addrH ) )
    return false;
  if( !WriteByteWithEcho( addrL ) )
    return false;
  return ReadByte( data );
}

bool WriteMemory( unsigned char addrH, unsigned char addrL, unsigned char data )
{
  if( !WriteByteWithEcho( 0x49 ) ) // ASCII 'I'
    return false;
  if( !WriteByteWithEcho( addrH ) )
    return false;
  if( !WriteByteWithEcho( addrL ) )
    return false;
  if( !WriteByteWithEcho( data ) )
    return false;
  WaitReadBitTime();
  WaitReadBitTime();
  WaitReadBitTime();
  WaitReadBitTime();
}

bool WriteNextMemory( unsigned char data )
{
  if( !WriteByteWithEcho( 0x19 ) )
    return false;
  if( !WriteByteWithEcho( data ) )
    return false;
  WaitReadBitTime();
  WaitReadBitTime();
}

bool RunUserProgram( unsigned char data )
{
  return WriteByteWithEcho( 0x28 );//ASCII '('
}



// HC908 Memory Map
// 0x0000 - 0x003F  I/O
// 0x0040 - 0x013F  RAM
// 0xDC00 - 0xFBFF  Flash
// 0xFE08           Flash Control Register
// 0xFE09           Flash Block Protect Register
// 0xFFF0 - 0xFFFF  Flash Vectors
// 0xFFF6 - 0xFFFD  Security Key

#define FLASH_CTRL_H 0xFE
#define FLASH_CTRL_L 0x08
#define FLASH_CTRL_HVEN  (1<<3)
#define FLASH_CTRL_MASS  (1<<2)
#define FLASH_CTRL_ERASE (1<<1)
#define FLASH_CTRL_PGM   (1<<0)

void WaitTrcv()
{
  // 1us
  //PORTD |= 1<<HC908_PTA1;
  for( cnt=0; cnt<1; ++cnt )
    __asm__("nop\n\t");
  //PORTD &= ~(1<<HC908_PTA1);
}
void WaitTnvs()
{
  // 5us
  //PORTD |= 1<<HC908_PTA1;
  for( cnt=0; cnt<4; ++cnt )
    __asm__("nop\n\t");
  //PORTD &= ~(1<<HC908_PTA1);
}
void WaitTpgs()
{
  // 10us
  //PORTD |= 1<<HC908_PTA1;
  for( cnt=0; cnt<8; ++cnt )
    __asm__("nop\n\t");
  //PORTD &= ~(1<<HC908_PTA1);
}
void WaitTprog()
{
  // 20us
  //PORTD |= 1<<HC908_PTA1;
  for( cnt=0; cnt<16; ++cnt )
    __asm__("nop\n\t");
  //PORTD &= ~(1<<HC908_PTA1);
}
void WaitTnvh()
{
  // 5us
  //PORTD |= 1<<HC908_PTA1;
  for( cnt=0; cnt<4; ++cnt )
    __asm__("nop\n\t");
  //PORTD &= ~(1<<HC908_PTA1);
}
void WaitTnvh1()
{
  // 100us
  //PORTD |= 1<<HC908_PTA1;
  for( cnt=0; cnt<80; ++cnt )
    __asm__("nop\n\t");
  //PORTD &= ~(1<<HC908_PTA1);
}
void WaitTme()
{
  // 2ms (milliseconds)
  //PORTD |= 1<<HC908_PTA1;
  for( cnt=0; cnt<1600; ++cnt )
    __asm__("nop\n\t");
  //PORTD &= ~(1<<HC908_PTA1);
}

void MassErase()
{
  WriteMemory( FLASH_CTRL_H, FLASH_CTRL_L, FLASH_CTRL_MASS | FLASH_CTRL_ERASE );
  WaitTme();
  WriteMemory( 0xFF, 0xE0, 0 );
  WaitTme();//WaitTnvs();
  WriteMemory( FLASH_CTRL_H, FLASH_CTRL_L, FLASH_CTRL_MASS | FLASH_CTRL_ERASE | FLASH_CTRL_HVEN );
  WaitTme();
  WriteMemory( FLASH_CTRL_H, FLASH_CTRL_L, FLASH_CTRL_MASS |                    FLASH_CTRL_HVEN );
  WaitTme();//WaitTnvh1();
  WriteMemory( FLASH_CTRL_H, FLASH_CTRL_L, FLASH_CTRL_MASS );
  WaitTme();//WaitTrcv();
  WriteMemory( FLASH_CTRL_H, FLASH_CTRL_L, 0 );
  WaitTme();
}

void StartRow( unsigned char addrH, unsigned char addrL )
{
  WriteMemory( FLASH_CTRL_H, FLASH_CTRL_L, FLASH_CTRL_PGM );
  WaitTme();
  WriteMemory( addrH, addrL, 0 );
  WaitTme();//WaitTnvs();
  WriteMemory( FLASH_CTRL_H, FLASH_CTRL_L, FLASH_CTRL_PGM | FLASH_CTRL_HVEN );
  WaitTme();//WaitTpgs();
}

void ProgByte( unsigned char addrH, unsigned char addrL, unsigned char data )
{
  WriteMemory( addrH, addrL, data );
  WaitTprog();
}

void EndRow()
{
  WriteMemory( FLASH_CTRL_H, FLASH_CTRL_L, FLASH_CTRL_HVEN );
  WaitTme();//WaitTnvh();
  WriteMemory( FLASH_CTRL_H, FLASH_CTRL_L, 0 );
  WaitTme();//WaitTrcv();
}


void ConfigureInitialHC908Pins()
{
  digitalWrite( HC908_RST,  0 );
  digitalWrite( HC908_PTA0, 1 );
  digitalWrite( HC908_PTA1, 1 );
  digitalWrite( HC908_PTA2, 0 );
  digitalWrite( HC908_PTA3, 1 );
}

void ResetHC908()
{
  ConfigureInitialHC908Pins();
  delay(5);
  digitalWrite( HC908_RST,  1 );
}

bool SendSecurityBytes()
{
  uint8_t rx;
  for( int i=0; i<7; ++i )
  {
    if( !WriteByteWithEcho( 0xFF ) )
      return false;
    delay(3);
  }
  if( !WriteByteWithEcho( 0xFF ) )
    return false;
    
  return ReadByte(&rx);
}


void setup()
{
  // Init timers so we can synthesis a 3MHz signal and the 9600bps clock for our weird serial line.
  // We can't use the standard software UART because/ the input and output are multiplexed.
  // From https://www.robotshop.com/community/forum/t/arduino-101-timers-and-interrupts/13072
  // We see the Arduino delay(), millis() and micros() functions rely on Timer0 which is 8bit.
  // As we are on an Uno / Duemilanov, we have the 16bit Timer1 (which would be used for the Servo library)
  // and the 8bit Timer2 used for tone()



  pinMode(ledPin, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);

  pinMode( HC908_RST,  OUTPUT );
  pinMode( HC908_PTA1, OUTPUT );
  pinMode( HC908_PTA2, OUTPUT );
  pinMode( HC908_PTA3, OUTPUT );
  SetPTA0AsInput();

  // tell the HC908 it's running at 6MHz (in reality it's 3.125MHz)
  // and tell it that the internal bus frequency is 3MHz (in reality it's 1.5625MHz)
  ConfigureInitialHC908Pins();


  // initialize timer1 
  noInterrupts();           // disable all interrupts
  SetTimer1ForPWMOnD9();
  interrupts();             // enable all interrupts

  delay( 5 );
  digitalWrite( HC908_RST,  1 );  // Take the HC908 out of reset and latch it into MonitorMode
  digitalWrite( ledPin, 1 );
  digitalWrite( HC908_PTA1, 0 );
  delay( 5 );

  Serial.begin( 250000 );

#if 0
  // Send the 8 security bytes. This should be 8 x 0xFF for the blank chip
  for( int i=0; i<8; ++i )
  {
    WriteByte( 0xFF );
    ReadByte();
    WaitReadBitTime();
    WaitReadBitTime();
    WaitReadBitTime();
  }
#endif
  
#if 0
  delay( 10 );
  WriteBreak();
  ReadByte();
#endif

#if 0
  unsigned char rx;
  delay( 10 );
  rx = ReadMemory( 0x0040 );
  if( rx & (1<<6) )
  {
    Serial.println( "Security not in effect" );
  }
  else
  {
    Serial.print( "Security byte set to " );
    Serial.println( rx, HEX );
  }
  delay( 10 );
  WriteMemory( 0x0040, 0x55 );
  delay( 10 );
  rx = ReadMemory( 0x0040 );
  Serial.println( rx, HEX );
  delay( 10 );
  WriteMemory( 0x0040, 0x33 );
  delay( 10 );
  rx = ReadMemory( 0x0040 );
  Serial.println( rx, HEX );
#endif
}

unsigned char gBuf[4];
int gBufIdx = 0;
void ProcessByte( unsigned char b )
{
  gBuf[gBufIdx++] = b;
  if( gBufIdx >= 4 )
  {
    unsigned char txb;
    switch( gBuf[0] )
    {
      case 0:
        StartRow( gBuf[1], gBuf[2] );
        Serial.write( 'K' );
        break;
      case 1:
        ProgByte( gBuf[1], gBuf[2], gBuf[3] );
        Serial.write( 'K' );
        break;
      case 2:
        EndRow();
        Serial.write( 'K' );
        break;
      case 3:
        MassErase();
        Serial.write( 'K' );
        break;
      case 4:
        if( ReadMemory( gBuf[1], gBuf[2], &txb ) )
        {
          Serial.write( txb );
          Serial.write( 'K' );
        }
        else
        {
          Serial.write( txb );
          Serial.write( 'N' );
        }
        break;
      case 5:
        ResetHC908();
        Serial.write( 'K' );
        break;
      case 6:
        if( SendSecurityBytes() )
          Serial.write( 'K' );
        else
          Serial.write( 'N' );
        break;
      default:
        Serial.write( 'N' );
    }
    gBufIdx = 0;
  }
}

void loop()
{
  static unsigned int watchdog = 0;
  delay(1); // 1 millisecond
  if( Serial.available() > 0 )
  {
    ProcessByte( Serial.read() );
    watchdog = 0;
  }
  else
  {
    // Check watchdog timer to see if we need to reset our statemachine
    watchdog++;
    if( watchdog > 65500 )
    {
      watchdog = 0;
      gBufIdx = 0;
    }
  }
}

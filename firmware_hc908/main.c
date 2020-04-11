#define USE_USB_PIPE            // if defined => USB, if not => RS232


#include <mc68hc908jb8.h>
#include "main.h"



/* SDCC handles interrupt vectors in an odd way
   The 68HC908 has a vector table located at 0xFFF0 as follows

	Address		Name				SDCC Index
	0xFFF0 		Keypad     			7
	0xFFF2 		TIMER overflow		6
	0xFFF4 		TIMER channel 1		5
	0xFFF6 		TIMER channel 0		4
	0xFFF8 		IRQ1          		3
	0xFFFA 		USB            		2
	0xFFFC 		SWI             	1
	0xFFFE 		RESET				0

	SDCC, however, uses the "__interrupt <n>" statement to define an interrupt service routine.
	<n> is the index of the interrupt in the table. Stupidly, the 68HC908 port defines these indicies in the reverse order
*/




#if 0
void initSADC()
{
	// disable internal Pull-Ups on PTE
	POCR &= ~0x80;	// disable PTE20P
}



unsigned scaleSADC( unsigned t1, unsigned t2 )
{

	unsigned retval;
	unsigned char n;

	//-----------------------------------------------------------
	// retval = (t2/t1) * 256  (0 <= retval <= 0xff)
	// We are taking care of overflow/accuracy by actually doing:
	// retval = (t2 * 2^j) / (t1 / 2^k)
	// with j+k = 8 (because 2^8=256) 
	//-----------------------------------------------------------
	for( n = 0; n < 8; n++ )
	{
		if( ( t2 & 0x8000 ) == 0 ) t2 <<= 1;
		else t1 >>= 1;
	}
	retval = t2 / t1;
	retval *= 135;	// *135/128 = *1.055
	retval >>= 7;
	if( retval > 0x00ff ) retval = 0x00ff;
	return( retval );
}



void delay025ms( void )
{

	__asm
	lda #240
		__l_01: dbnza __l_01
		__endasm;				// 3cyc x 333ns x 240+9 = 0,249ms
}



int getSADC( char channel )
{

	unsigned tcal, tcal0, tcal1;
	unsigned tacq, tacq0, tacq1;
	unsigned t;

	// convert channel # 1/2/3 to 0x01/0x02/0x04
	if( channel == 3 ) channel++;

	PTD &= ~0x78;		// all PTD[3..6] = L
	PTE |= 0x07;		// all PTE[0..2] = H;

	// *** calibration cycle ***

	DDRD |= 0x78;		// Output L on PTD[3..6]
	DDRE |= 0x07;		// Output H on PTE[0..2]
	delay025ms();		// discharge C13..C15

	DDRE &= ~0x07;		// PTE[0..2] HiZ (Input)
	tcal0 = ( TCNTH << 8 ) + TCNTL;		// cal start time
	while( ( PTE & channel ) != 0 );
	tcal1 = TCNT;		// cal end time
	tcal = tcal1 - tcal0;

	// *** acquisition cycle ***

	DDRD &= ~0x38;		// PTD[3..5] = HiZ, PTD[6] = L
	DDRE |= 0x07;		// Output H on PTE[0..2]
	delay025ms();		// discharge C13..C15

	DDRE &= ~0x07;		// PTE HiZ (Input)
	tacq0 = TCNT;		// acq start time
	while( ( PTE & channel ) != 0 );
	tacq1 = TCNT;		// acq end time
	tacq = tacq1 - tacq0;

	if( tacq <= tcal ) return( 0 );	// underflow
	t = tacq - tcal;
	if( t >= tcal ) return( 0xff );	// overflow

	// *** calculate scaled result ***
	t = scaleSADC( tcal, t );
	return t;
}








const device_descriptor DeviceDesc =
{					
	sizeof( device_descriptor ),		// Size of this Descriptor in Bytes
	DT_DEVICE,							// Descriptor Type (=1)
	{0x10, 0x01},						// USB Spec Release Number in BCD = 1.10
	0,									// Device Class Code (none)
	0,									// Device Subclass Code	(none)
	0,									// Device Protocol Code (none)
	8,									// Maximum Packet Size for EP0 
	{0x70, 0x0c},						// Vendor ID = MCT Elektronikladen
	{0x00, 0x00},						// Product ID = Generic Demo
	{0x00, 0x01},						// Device Release Number in BCD
	1,									// Index of String Desc for Manufacturer
	2,									// Index of String Desc for Product
	0,									// Index of String Desc for SerNo
	1									// Number of possible Configurations
};



const configuration_descriptor ConfigDesc =
{					
	sizeof( configuration_descriptor ),	// Size of this Descriptor in Bytes
	DT_CONFIGURATION,					// Descriptor Type (=2)
	{sizeof( configuration_descriptor ) + sizeof( interface_descriptor ) + sizeof( endpoint_descriptor ) + sizeof( endpoint_descriptor ),
	 0x00},								// Total Length of Data for this Conf
	1,									// No of Interfaces supported by this Conf
	1,									// Designator Value for *this* Configuration
	0,									// Index of String Desc for this Conf
	0xc0,								// Self-powered, no Remote-Wakeup
	0									// Max. Power Consumption in this Conf (*2mA)
};



const interface_descriptor InterfaceDesc =
{					
	sizeof( interface_descriptor ),		// Size of this Descriptor in Bytes
	DT_INTERFACE,						// Descriptor Type (=4)
	0,									// Number of *this* Interface (0..)
	0,									// Alternative for this Interface (if any)
	2,									// No of EPs used by this IF (excl. EP0)
	0xff,								// IF Class Code (0xff = Vendor specific)
	0x01,								// Interface Subclass Code
	0xff,								// IF Protocol Code  (0xff = Vendor specific)
	0									// Index of String Desc for this Interface
};



const endpoint_descriptor Endpoint1Desc =
{
	sizeof( endpoint_descriptor ),		// Size of this Descriptor in Bytes
	DT_ENDPOINT,						// Descriptor Type (=5)
	0x81,								// Endpoint Address (EP1, IN)
	0x03,								// Interrupt
	{0x08, 0x00},						// Max. Endpoint Packet Size
	10									// Polling Interval (Interrupt) in ms
};



const endpoint_descriptor Endpoint2Desc =
{
	sizeof( endpoint_descriptor ),		// Size of this Descriptor in Bytes
	DT_ENDPOINT,						// Descriptor Type (=5)
	0x02,								// Endpoint Address (EP2, OUT)
	0x03,								// Interrupt
	{0x08, 0x00},						// Max. Endpoint Packet Size
	10									// Polling Interval (Interrupt) in ms
};



// Language IDs
//--------------
#define SD0LEN 4
//--------------

const uchar String0Desc[ SD0LEN ] =
{
	// Size, Type
	SD0LEN, DT_STRING,
	// LangID Codes
	0x09, 0x04
};

// Manufacturer String
//--------------------------------------------
#define SD1LEN sizeof("ProceduralWorlds.co.uk")*2
//--------------------------------------------
const uchar String1Desc[ SD1LEN ] = {
	// Size, Type
	SD1LEN, DT_STRING,
	// Unicode String
	'P', 0,
	'r', 0,
	'o', 0,
	'c', 0,
	'e', 0,
	'd', 0,
	'u', 0,
	'r', 0,
	'a', 0,
	'l', 0,
	'W', 0,
	'o', 0,
	'r', 0,
	'l', 0,
	'd', 0,
	's', 0,
	'.', 0,
	'c', 0,
	'o', 0,
	'.', 0,
	'u', 0,
	'k', 0
};

// Product String
//-----------------------------------------------
#define SD2LEN sizeof("USBTest_0000")*2
//-----------------------------------------------
const uchar String2Desc[ SD2LEN ] = {
	// Size, Type
	SD2LEN, DT_STRING,
	// Unicode String
	'U', 0,
	'S', 0,
	'B', 0,
	'T', 0,
	'e', 0,
	's', 0,
	't', 0,
	'_', 0,
	'0', 0,
	'0', 0,
	'0', 0,
	'0', 0
};

// Table of String Descriptors
//
const uchar* const StringDescTable[] = {
	String0Desc,
	String1Desc,
	String2Desc
};

//============================================================================









//-- Definitions -------------------------------------------------------------

// Specification of *active* Key Inputs:
// PTA[4,5,6] = %01110000 = 0x70
// First Key connected to Port Bit 4

#define KEY_MASK 0x70
#define KEY_FIRST 4

//-- Variables ---------------------------------------------------------------

// Var used to track the Key Status

unsigned char KeyState;



void initKey()
{

	POCR |= 0x01;		// enable PTA Pullups
	PTA |= KEY_MASK;	// write 1 to Output Latches
	DDRA |= KEY_MASK;	// output H-Level Pulse
	DDRA &= ~KEY_MASK;	// back to Input
	KBIER = KEY_MASK;	// enable Interrupts
	KBSCR = 0x04;		// reset ACKK (just in case)
	KeyState = 0;		// reset internal Status Var
}



char getKey( unsigned char x )
{

	x += KEY_FIRST - 1;	// calculate Bit Position
	x = 1 << x;			// create Bit Mask
	if( KeyState & x )	// test the relevant Status Bit
		return 1;
	return 0;
}



void isrKey() __interrupt 7
{

	KeyState ^= ~( PTA | ~KEY_MASK );
	KBSCR = 0x04;		// reset ACKK (for noise safety only)
}

//============================================================================














//-- some Assembly Level Stuff -----------------------------------------------

#define nop()		__asm\
						nop\
					__endasm




//-- Code Starts here --------------------------------------------------------

#ifdef USE_USB_PIPE

//-- Source Code Option - set as required by Hardware! -----------------------
//
#define USB_IPUE	1			// Internal Pull-up Enable
								// 1=Enable	(use build-in Pull-up Resistor)
								// 0=Disable (external Pull-up required)
//-- Variables ---------------------------------------------------------------

volatile uchar USB_State;

#define MAX_TXBUF_SIZE 16		// must be 2^x!
volatile uchar TxBuffer[ MAX_TXBUF_SIZE ];
volatile uchar TxBuf_RdIdx;
volatile uchar TxBuf_WrIdx;

#define MAX_RXBUF_SIZE 16		// must be 2^x!
volatile uchar RxBuffer[ MAX_RXBUF_SIZE ];
volatile uchar RxBuf_RdIdx;
volatile uchar RxBuf_WrIdx;

volatile uchar SuspendCounter;

setup_buffer SetupBuffer;
uchar SetupSize;
const uchar* SetupDataPtr;

uchar R0Sequence;	// DATA0/1 Flag for EP0 Rx
uchar R2Sequence;	// DATA0/1 Flag for EP2 Rx



// Force STALL Condition for EP0 (both IN and OUT)
// as a Response to an invalid SETUP Request
// Flags will be auto-cleared by the next SETUP Token
//
void forceSTALL()
{
	UCR3 |= BM_OSTALL0 + BM_ISTALL0;
}



#define ENDPOINT_HALT	0x00
#define RT_ENDPOINT		0x02


// CLEAR_FEATURE Standard Device Request Handler
// called by handleSETUP();
//
void clearFeature()
{

	if( SetupBuffer.wValue.hi ||
		SetupBuffer.wIndex.hi ||
		SetupBuffer.wLength.hi ||
		SetupBuffer.wLength.lo )	// check 0-fields
	{
		forceSTALL();
	}
	else if( ( SetupBuffer.bmRequestType == RT_ENDPOINT ) &&
			( SetupBuffer.wValue.lo == ENDPOINT_HALT ) &&
			( ( SetupBuffer.wIndex.lo == 0x81 ) || ( SetupBuffer.wIndex.lo == 0x02 ) ) )
	{
		// clear EP1/2 Force STALL Bit
		if( SetupBuffer.wIndex.lo == 0x81 )
		{
			// EP1
			UCR1 &= ~( BM_T1SEQ + BM_STALL1 );	// clear STALL, Sequence = DATA0
		}
		else
		{
			// EP2
			UCR2 &= ~BM_STALL2;				// clear STALL
			R2Sequence = 0;					// Sequence = DATA0
		}
		// prepare to send empty DATA1 at next IN Transaction
		UCR0 = BM_T0SEQ | BM_TX0E | 0;
	}
	else
	{
		forceSTALL();
	}
}




// SET_ADDRESS Standard Device Request Handler
// called by handleSETUP();
//
void setAddress()
{

	if( SetupBuffer.wIndex.hi ||
		SetupBuffer.wIndex.lo ||
		SetupBuffer.wLength.hi ||
		SetupBuffer.wLength.lo ||
		SetupBuffer.wValue.hi ||
		( SetupBuffer.wValue.lo & 0x80 ) )
	{
		forceSTALL();
	}
	else
	{
		// prepare to send empty DATA1 at next IN Transaction
		UCR0 = BM_T0SEQ | BM_TX0E | 0;
	}
}




// SET_CONFIGURATION Standard Device Request Handler
// called by handleSETUP();
//
void setConfiguration()
{

	if( SetupBuffer.wIndex.hi ||
		SetupBuffer.wIndex.lo ||
		SetupBuffer.wLength.hi ||
		SetupBuffer.wLength.lo ||
		SetupBuffer.wValue.hi ||
		( SetupBuffer.wValue.lo > 1 ) ||
		( USB_State == US_DEFAULT ) )
	{
		forceSTALL();
	}
	else
	{
		if( SetupBuffer.wValue.lo > 0 )
		{
			// no need to remember the Configuration Value
			// since we support only one Configuration anyway
			USB_State = US_CONFIGURED;
			// Activate Interrupt Endpoints, reset STALL and DATA-Toggle
			UCR1 = BM_TX1E + 0;	// EP1 Tx Enable, Data Size is 0
			UCR2 = BM_RX2E;		// EP2 Rx Enable
		}
		else
		{
			// Zero means: go back to Adressed State
			USB_State = US_ADDRESSED;
			UCR1 = 0;			// deactivate EP1
			UCR2 = 0;			// deactivate EP2
		}
		// prepare to send empty DATA1 at next IN Transaction
		UCR0 = BM_T0SEQ | BM_TX0E | 0;
	}
}




// GET_DESCRIPTOR Standard Device Request Handler
// called by handleSETUP();
//
void getDescriptor()
{

	uchar n;
	uchar* dest;

	switch( SetupBuffer.wValue.hi )
	{

		case DT_DEVICE:			// Get Device Descriptor
			SetupDataPtr = (uchar*)&DeviceDesc;
			SetupSize = DeviceDesc.bLength;
			break;

		case DT_CONFIGURATION:	// Get Configuration Descriptor
			SetupDataPtr = (uchar*)&ConfigDesc;
			SetupSize = ConfigDesc.wTotalLength.lo;
			break;

		case DT_STRING:			// Get String Descriptor
			// ### Table Index Boundary should be checked
			SetupDataPtr = StringDescTable[ SetupBuffer.wValue.lo ];
			SetupSize = *SetupDataPtr;
			break;

		default:
			forceSTALL();
			break;
	}

	if( SetupBuffer.wValue.hi == DT_DEVICE ||
		SetupBuffer.wValue.hi == DT_CONFIGURATION ||
		SetupBuffer.wValue.hi == DT_STRING )
	{

		// check if requested Length is less than Descriptor Length
		if( ( SetupBuffer.wLength.lo < SetupSize ) && ( SetupBuffer.wLength.hi == 0 ) )
			SetupSize = SetupBuffer.wLength.lo;
		// copy (up to) 8 Bytes to EP0 Data Registers
		n = 0;
		dest = (uchar*)&UE0D0;
		while( SetupSize != 0 && n < 8 )
		{
			*dest = *SetupDataPtr;
			dest++;
			SetupDataPtr++;
			SetupSize--;
			n++;
		}
		// prepare to send n Bytes as DATA1 at next IN Transaction
		// Rem: RX0E (currently disabled) will be re-enabled at end of handleSETUP()
		UCR0 = BM_T0SEQ | BM_TX0E | n;
		// check if this is the last DATA packet to send
		if( n < 8 )
			SetupBuffer.bRequest = REQUEST_COMPLETE;
	}
}



void handleSETUP()
{

	UCR0 &= ~BM_RX0E;			// Deactivate EP0 Receiver
	UIR2 = BM_RXD0FR;			// Reset EP0 Receive Flag

	SetupBuffer = *(setup_buffer*)( &UE0D0 );

	if( USR0 != 0x48 )
	{			// SETUP Transaction must be DATA0 with Size=8
		forceSTALL();			// otherwise we have an Error Condition
	}
	else
	{						// now we will check the Request Type
		if( ( SetupBuffer.bmRequestType & 0x60 ) != 0 )
		{
			forceSTALL();		// Non-Standard Requests will not be handled!
		}
		else
		{					// Standard Request Decoder:
			switch( SetupBuffer.bRequest )
			{
				case CLEAR_FEATURE:		// 1
					clearFeature();
					break;
				case SET_ADDRESS:		// 5
					setAddress();
					break;
				case GET_DESCRIPTOR:	// 6
					getDescriptor();
					break;
				case SET_CONFIGURATION:	// 9
					setConfiguration();
					break;
				default:
					forceSTALL();
					break;
			}
		}
	}
	UCR0 |= BM_RX0E;			// Activate EP0 Receiver
}



void handleOUT()
{

	UCR0 &= ~( BM_RX0E + BM_TX0E );	// Deactivate EP0 Receiver + Transmitter
	UIR2 = BM_RXD0FR;			// Reset EP0 Receive Flag

	// OUT Transactions over EP0 appear as Status Stage
	// of a Standard Device Request only

	UCR0 |= BM_RX0E;			// Activate EP0 Receiver
}



void handleIN()
{

	uchar n;
	uchar* dest;

	UCR0 &= ~BM_TX0E;			// Deactivate EP0 Transmitter
	UIR2 = BM_TXD0FR;			// Reset EP0 Transmit complete Flag

	switch( SetupBuffer.bRequest )
	{
		case SET_ADDRESS:
			UADDR = SetupBuffer.wValue.lo | BM_USBEN;
			if( SetupBuffer.wValue.lo != 0 ) USB_State = US_ADDRESSED;
			else USB_State = US_DEFAULT;
			SetupBuffer.bRequest = REQUEST_COMPLETE;
			break;
		case GET_DESCRIPTOR:
			// copy (up to) 8 Bytes to EP0 Data Registers
			n = 0;
			dest = (uchar*)&UE0D0;
			while( SetupSize != 0 && n < 8 )
			{
				*dest = *SetupDataPtr;
				dest++;
				SetupDataPtr++;
				SetupSize--;
				n++;
			}
		// prepare to send n Bytes at next IN Transaction
		// toggle DATA0/1
			UCR0 = ( ( UCR0 ^ BM_T0SEQ ) & BM_T0SEQ ) + BM_TX0E + BM_RX0E + n;
			// check if this is the last DATA packet to send
			if( n < 8 ) SetupBuffer.bRequest = REQUEST_COMPLETE;
			break;
		case CLEAR_FEATURE:
		case SET_CONFIGURATION:
			// nothing to do - handshake finished
			SetupBuffer.bRequest = REQUEST_COMPLETE;
			break;
		case REQUEST_COMPLETE:
			// Request is finished - just clear the TXD0F Flag (see above)
			// and do not re-enable EP0 Transmitter, since there is no more
			// data to send
			break;
		default:
			forceSTALL();
			break;
	}
}



// handle IN Packet Transmit complete over EP1
//
void handleIN1()
{

	uchar n;
	uchar volatile* dest;

	UCR1 &= ~BM_TX1E;			// Deactivate EP1 Transmitter
	UIR2 = BM_TXD1FR;			// Reset EP1 Transmit complete Flag

	// refill EP1 Tx Data Buffer
	n = 0;
	dest = &UE1D0;
	while( ( TxBuf_RdIdx != TxBuf_WrIdx ) && n < 8 )
	{
		*dest = TxBuffer[ TxBuf_RdIdx ];
		TxBuf_RdIdx = ( TxBuf_RdIdx + 1 ) & ( MAX_TXBUF_SIZE - 1 );
		dest++;
		n++;
	}
// Activate EP1 Transmitter to send n Bytes
	UCR1 = ( ( UCR1 ^ BM_T1SEQ ) & BM_T1SEQ ) + BM_TX1E + n;
}



// handle OUT Packet received over EP2
//
void handleOUT2()
{

	uchar n;
	uchar newIdx;
	uchar volatile* src;

	UCR2 &= ~BM_RX2E;			// Deactivate EP2 Receiver
	UIR2 = BM_RXD2FR;			// Reset EP2 Receive Flag

	// ### Sender's DATA Toggle should be checked!

	// read out EP2 Rx Data Buffer
	src = &UE2D0;
	n = USR1 & BM_RP2SIZ;		// Check Transfer Size
	while( n )
	{
		newIdx = ( RxBuf_WrIdx + 1 ) & ( MAX_RXBUF_SIZE - 1 );
		while( newIdx == RxBuf_RdIdx )
			;	// wait if TxBuffer is full
		RxBuffer[ RxBuf_WrIdx ] = *src;
		RxBuf_WrIdx = newIdx;
		src++;
		n--;
	}
	UCR2 = BM_RX2E;				// Activate EP2 Receiver
}



void initUSB()
{

	UADDR = BM_USBEN | 0;		// USB enable, default address
	UCR0 = 0;					// reset EP0
	UCR1 = 0;					// reset EP1
	UCR2 = 0;					// reset EP2
	UCR3 = BM_TX1STR +			// clear TX1ST Flag
		USB_IPUE * BM_PULLEN;	// enable/disable internal Pull-up
	UCR4 = 0;					// USB normal operation
	UIR0 = 0;					// disable Interrupts
	UIR2 = 0xff;				// clear all Flags in UIR1
	R0Sequence = 0;				// EP0 Rx starts with DATA0
	R2Sequence = 0;				// EP2 Rx starts with DATA0
	USB_State = US_POWERED;		// powered, but not yet reset
	TxBuf_RdIdx = 0;			// reset Buffer Indexes
	TxBuf_WrIdx = 0;
	RxBuf_RdIdx = 0;
	RxBuf_WrIdx = 0;
}



uchar getUSB()
{

	uchar c;

	while( RxBuf_RdIdx == RxBuf_WrIdx )
		;	// wait if RxBuffer is empty
	c = RxBuffer[ RxBuf_RdIdx ];
	RxBuf_RdIdx = ( RxBuf_RdIdx + 1 ) & ( MAX_RXBUF_SIZE - 1 );
	return c;
}



void putUSB( uchar c )
{

	uchar newIdx;

	newIdx = ( TxBuf_WrIdx + 1 ) & ( MAX_TXBUF_SIZE - 1 );
	while( newIdx == TxBuf_RdIdx )
		;	// wait if TxBuffer is full
	TxBuffer[ TxBuf_WrIdx ] = c;
	TxBuf_WrIdx = newIdx;
}



// USB Interrupt Handler
// All Interrupt Sources of the JB8's integrated USB peripheral
// will be treated by this ISR
//
void isrUSB() __interrupt 2
{

	if( UIR1 & BM_EOPF )
	{		// End of Packet detected?
		SuspendCounter = 0;				// reset 3ms-Suspend Counter
		UIR2 = BM_EOPFR;				// reset EOP Intr Flag
	}
	else if( UIR1 & BM_RXD0F )
	{	// has EP0 received some data?
		if( USR0 & BM_SETUP )				// was it a SETUP Packet?
			handleSETUP();
		else							// or a normal OUT Packet
			handleOUT();
	}
	else if( UIR1 & BM_TXD0F )
	{	// has EP0 sent Data?
		handleIN();
	}
	else if( UIR1 & BM_TXD1F )
	{	// has EP1 sent Data?
		handleIN1();
	}
	else if( UIR1 & BM_RXD2F )
	{	// has EP2 received Data?
		handleOUT2();
	}
	else if( UIR1 & BM_RSTF )
	{	// USB Reset Signal State detected?
		initUSB();						// Soft Reset of USB Systems
		UCR3 |= BM_ENABLE1 | BM_ENABLE2;	// Enable EP1 and EP2
		UIR0 = BM_TXD0IE | BM_RXD0IE |	// EP0 Rx/Tx Intr Enable and
			BM_TXD1IE | BM_RXD2IE |	// EP1 Tx and EP2 Rx Intr Enable
			BM_EOPIE;				// and End-of-Packet Intr Enable
		UCR0 |= BM_RX0E;				// EP0 Receive Enable
		USB_State = US_DEFAULT;			// Device is powered and reset
	}
}

//============================================================================

#define initPipe	initUSB
#define getPipe  	getUSB
#define putPipe  	putUSB
#else

// Hardware Dependencies - Physical Port Usage for Software SCI Tx/Rx
// Change the following Definitions to meet your Hardware Requirements:


// Transmit Line is PTC[0]

#define	setTxLow()	(PTC &= ~0x01)
#define setTxHigh()	(PTC |= 0x01)
#define enaTxOut()	(DDRC |= 0x01)

// Receive Line is PTA[7]

#define tstRxLvl()	(PTA & 0x80)
#define enaRxIn()	(DDRA &= ~0x80)


// Hardware Dependencies - SSCI Bit Timing generated by System Timer TIM
// Change the following Definitions to meet your Hardware Requirements:


// clear TSTOP Bit in TSC Register to activate Counter
// PS0..PS2 Prescaler Bits in TSC Register must be 0 (default)
// so the Counter Rate is 3 MHz (0.333�s)

// 9600 Baud -> 104.1666 us per Bit -> 312.5 Clocks per Bit @ 3MHz
// 2400 Baud -> 416.6666 us per Bit -> 1250  Clocks per Bit @ 3MHz
// Adjust Value depending on Subroutine Call Overhead



void delayHalfBit()
{
	// subtract ~20 Clocks for Overhead!
	// 120 * 5 Clocks = 600 Clocks
	__asm
	lda #120
		__dhbl:	deca
		nop
		bne __dhbl
		__endasm;
}


void delayBitTime()
{
	delayHalfBit();
	delayHalfBit();
}



void initSSCI()
{
	setTxHigh();				// set Output Data Latch H
	enaTxOut();					// enable Output Driver for Tx
	enaRxIn();					// Rx is an Input Line
}



void putSSCI( char c )
{

	unsigned char n;
	unsigned char ccr_save;

//	ccr_save = getCCR();		// save current Interrupt Mask
//	disableINTR();				// disable Interrupts

	setTxLow();					// send Startbit
	delayBitTime();

	n = 8;
	do
	{						// send 8 Databits, LSB first
		if( ( c & 1 ) == 0 )
			setTxLow();
		else
			setTxHigh();
		delayBitTime();
		c >>= 1;
	}
	while( --n );

	setTxHigh();				// send Stopbit
	delayBitTime();
	delayBitTime();
//	setCCR(ccr_save);			// restore previous Interrupt Mask
}



char getSSCI()
{

	char c;
	unsigned char n;
	unsigned char ccr_save;

//	ccr_save = getCCR();		// save current Interrupt Mask
//	disableINTR();				// disable Interrupts

	while( tstRxLvl() != 0 );		// wait for H-L transition
	delayHalfBit();
	n = 8;
	do
	{						// get 8 Databits
		delayBitTime();
		c >>= 1;
		if( tstRxLvl() != 0 )
			c |= 0x80;
	}
	while( --n );
	delayBitTime();
	if( tstRxLvl() == 0 )
	{
		// check Rx Line during Stopbit
		// add framing error
		// handling if desired
	}
	delayHalfBit();
//	setCCR(ccr_save);			// restore previous Interrupt Mask
	return c;
}

//============================================================================
#define initPipe	initSSCI
#define getPipe  	getSSCI
#define putPipe  	putSSCI
#endif





// Things that should be done immediately after Reset
// (this is called by the C-Startup Module)
//
void _HC08Setup()
{
	CONFIG = 0x21;	// USB Reset Disable, COP Disable
	TSC = 0x00;	// clear TSTOP, Prescaler=0
}




void isrTIMEROverflow()	__interrupt	6
{
}

void isrTIMERChannel1()	__interrupt	5
{
}

void isrTIMERChannel0()	__interrupt	4
{
}

void isrIRQ1()          __interrupt	3
{
}

void isrSWI()           __interrupt 1
{
}

void isrRESET()			__interrupt	0
{
}


void onLED( int i )
{
	i;
}

void offLED( int i )
{
	i;
}




void main()
{
	uchar n, a;
	uchar io_buffer[ 8 ];
	uchar adc[ 3 ];

	initPipe();		// init RS232 or USB Pipe
	initKey();		// init Key Input
	initSADC();		// init Soft ADC

__asm
	cli
__endasm;


	a = 0;
	while( 1 )
	{
		// update ADC results (1 out of 3 at one time)

		adc[ a ] = getSADC( a + 1 );
		if( ++a == 3 ) a = 0;

		// get data from input pipe

		n = 0;
		do
		{
			io_buffer[ n++ ] = getPipe();
		}
		while( n < 8 );

		// process input data

		if( io_buffer[ 0 ] == 0 ) offLED( 1 );	else onLED( 1 );
		if( io_buffer[ 1 ] == 0 ) offLED( 2 );	else onLED( 2 );
		if( io_buffer[ 2 ] == 0 ) offLED( 3 );	else onLED( 3 );

		// send data to output pipe

		io_buffer[ 0 ] = getKey( 1 );
		io_buffer[ 1 ] = getKey( 2 );
		io_buffer[ 2 ] = getKey( 3 );
		io_buffer[ 3 ] = adc[ 0 ];
		io_buffer[ 4 ] = adc[ 1 ];
		io_buffer[ 5 ] = adc[ 2 ];

		n = 0;
		do
		{
			putPipe( io_buffer[ n++ ] );
		}
		while( n < 8 );
	}
}
#else

void main()
{
	// Sit in a loop toggling PTA0 to test the firmware is running
	DDRA = 255;	// set all pins as output
	PTA = 255;
	while( 1 )
	{
		PTA = 255;
		PTA = 0;
	}
}


#endif



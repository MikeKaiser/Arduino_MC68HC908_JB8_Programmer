
#ifndef __HC08_H
#define __HC08_H	1

// Control Register Definitions for HC908JB8 ---------------------------------

#define _IO_BASE	0
#define _P(off)		*(unsigned char volatile *)(_IO_BASE + off)
#define _LP(off)	*(unsigned short volatile *)(_IO_BASE + off)


#define PTA		_P(0x00)
#define	PTB		_P(0x01)
#define	PTC		_P(0x02)
#define	PTD		_P(0x03)
#define	DDRA	_P(0x04)
#define	DDRB	_P(0x05)
#define	DDRC	_P(0x06)
#define	DDRD	_P(0x07)
#define	PTE		_P(0x08)
#define	DDRE	_P(0x09)
#define	TSC		_P(0x0A)
//not implemented (0x0B)
#define	TCNTH	_P(0x0C)
#define	TCNTL	_P(0x0D)
#define	TMODH	_P(0x0E)
#define	TMODL	_P(0x0F)
#define	TSC0	_P(0x10)
#define	TCH0H	_P(0x11)
#define	TCH0L	_P(0x12)
#define	TSC1	_P(0x13)
#define	TCH1H	_P(0x14)
#define	TCH1L	_P(0x15)
#define	KBSCR	_P(0x16)
#define	KBIER	_P(0x17)
#define	UIR2	_P(0x18)
#define	UCR2	_P(0x19)
#define	UCR3	_P(0x1A)
#define	UCR4	_P(0x1B)
#define	IOCR	_P(0x1C)
#define	POCR	_P(0x1D)
#define	ISCR	_P(0x1E)
#define	CONFIG	_P(0x1F)
#define	UE0D0	_P(0x20)
#define	UE0D1	_P(0x21)
#define	UE0D2	_P(0x22)
#define	UE0D3	_P(0x23)
#define	UE0D4	_P(0x24)
#define	UE0D5	_P(0x25)
#define	UE0D6	_P(0x26)
#define	UE0D7	_P(0x27)
#define	UE1D0	_P(0x28)
#define	UE1D1	_P(0x29)
#define UE1D2	_P(0x2A)
#define	UE1D3	_P(0x2B)
#define	UE1D4	_P(0x2C)
#define	UE1D5	_P(0x2D)
#define	UE1D6	_P(0x2E)
#define	UE1D7	_P(0x2F)
#define	UE2D0	_P(0x30)
#define	UE2D1	_P(0x31)
#define	UE2D2	_P(0x32)
#define	UE2D3	_P(0x33)
#define	UE2D4	_P(0x34)
#define	UE2D5	_P(0x35)
#define	UE2D6	_P(0x36)
#define	UE2D7	_P(0x37)
#define	UADDR	_P(0x38)
#define	UIR0	_P(0x39)
#define	UIR1	_P(0x3A)
#define	UCR0	_P(0x3B)
#define	UCR1	_P(0x3C)
#define	USR0	_P(0x3D)
#define	USR1	_P(0x3E)
//not implemented (0x3F)

// 16-Bit Registers:
#define	TCNT	_LP(0x0C)
#define	TMOD	_LP(0x0E)
#define	TCH0	_LP(0x11)
#define	TCH1	_LP(0x14)

//-- Bit Mask Definitions ----------------------------------------------------

// Bits in UADDR:
#define BM_USBEN	0x80		// USB Module Enable

// Bits in UIR0:
#define BM_EOPIE	0x80		// End-of-Packet Detect Interrupt Enable
#define BM_RXD2IE	0x10		// EP2 Rx Interrupt Enable
#define BM_TXD1IE	0x08		// EP1 Tx Interrupt Enable
#define BM_TXD0IE	0x02		// EP0 Tx Interrupt Enable
#define BM_RXD0IE	0x01		// EP0 Rx Interrupt Enable

// Bits in UIR1:
#define BM_EOPF		0x80		// End-of-Packet Detect Flag
#define BM_RSTF 	0x40		// Clear Reset Indicator Bit
#define BM_RXD2F	0x10		// EP2 Data Receive Flag
#define BM_TXD1F	0x08		// EP1 Data Transmit complete Flag
#define BM_TXD0F	0x02		// EP0 Data Transmit complete Flag
#define BM_RXD0F	0x01		// EP0 Data Receive Flag

// Bits in UIR2:
#define BM_EOPFR	0x80		// End-of-Packet Flag Reset
//#define BM_RSTFR 	0x40		// Clear Reset Indicator Bit
#define BM_RXD2FR	0x10		// EP2 Receive Flag Reset
#define BM_TXD1FR	0x08		// EP1 Transmit complete Flag Reset
#define BM_TXD0FR	0x02		// EP0 Transmit complete Flag Reset
#define BM_RXD0FR	0x01		// EP0 Receive Flag Reset

// Bits in UCR0:
#define BM_T0SEQ	0x80		// EP0 Tx Sequence Bit (DATA0/1)
#define BM_TX0E		0x20		// EP0 Tx Enable
#define BM_RX0E		0x10		// EP0 Rx Enable
//#define BM_TP0SIZ	0x0f		// EP0 Tx Data Packet Size

// Bits in UCR1:
#define BM_T1SEQ	0x80		// EP1 Tx Sequence Bit (DATA0/1)
#define BM_STALL1	0x40		// EP1 Force Stall Bit
#define BM_TX1E		0x20		// EP1 Tx Enable
//#define BM_TP1SIZ	0x0f		// EP1 Tx Data Packet Size

// Bits in UCR2:
#define BM_STALL2	0x40		// EP2 Force Stall Bit
#define BM_RX2E		0x10		// EP2 Rx Enable

// Bits in UCR3:
#define BM_TX1STR	0x40		// Clear EP0 Transmit-1st Flag
#define BM_OSTALL0	0x20		// EP0 force STALL Bit for OUT Token
#define BM_ISTALL0	0x10		// EP0 force STALL Bit for IN Token
#define BM_PULLEN	0x04		// Pull-up Enable
#define BM_ENABLE2	0x02		// EP2 Enable
#define BM_ENABLE1	0x01		// EP1 Enable

// Bits in USR0:
// #define BM_R0SEQ	0x80		// EP0 Rx Sequence Bit (DATA0/1)
#define BM_SETUP	0x40		// Setup Token Detect Bit
//#define BM_RP0SIZ	0x0f		// EP0 Rx Data Packet Size

// Bits in USR1:
//#define BM_R2SEQ	0x80		// EP2 Rx Sequence Bit (DATA0/1)
#define BM_RP2SIZ	0x0f		// EP2 Rx Data Packet Size

















#define uchar unsigned char

//-- Data Type Definitions ---------------------------------------------------

typedef struct
{				// Data Type "Intel Word"
	uchar lo;					// (High/Low Byte swapped)
	uchar hi;
} iword;



// Standard Device Descriptor
// according to USB1.1 spec page 197
//
typedef struct
{
	uchar bLength;				// Size of this Descriptor in Bytes
	uchar bDescriptorType;		// Descriptor Type (=1)
	iword bcdUSB;				// USB Spec Release Number in BCD
	uchar bDeviceClass;			// Device Class Code
	uchar bDeviceSubClass;		// Device Subclass Code	
	uchar bDeviceProtocol;		// Device Protocol Code
	uchar bMaxPacketSize0;		// Maximum Packet Size for EP0 
	iword idVendor;				// Vendor ID 
	iword idProduct;			// Product ID
	iword bcdDevice;			// Device Release Number in BCD
	uchar iManufacturer;		// Index of String Desc for Manufacturer
	uchar iProduct;				// Index of String Desc for Product
	uchar iSerialNumber;		// Index of String Desc for SerNo
	uchar bNumConfigurations;	// Number of possible Configurations
} device_descriptor;



// Standard Configuration Descriptor
// according to USB1.1 spec page 199
//
typedef struct
{
	uchar bLength;				// Size of this Descriptor in Bytes
	uchar bDescriptorType;		// Descriptor Type (=2)
	iword wTotalLength;			// Total Length of Data for this Conf
	uchar bNumInterfaces;		// No of Interfaces supported by this Conf
	uchar bConfigurationValue;	// Designator Value for *this* Configuration
	uchar iConfiguration;		// Index of String Desc for this Conf
	uchar bmAttributes;			// Configuration Characteristics (see below)
	uchar bMaxPower;			// Max. Power Consumption in this Conf (*2mA)
} configuration_descriptor;



// Standard Interface Descriptor
// according to USB1.1 spec page 202
//
typedef struct
{
	uchar bLength;				// Size of this Descriptor in Bytes
	uchar bDescriptorType;		// Descriptor Type (=4)
	uchar bInterfaceNumber;		// Number of *this* Interface (0..)
	uchar bAlternateSetting;	// Alternative for this Interface (if any)
	uchar bNumEndpoints;		// No of EPs used by this IF (excl. EP0)
	uchar bInterfaceClass;		// Interface Class Code
	uchar bInterfaceSubClass;	// Interface Subclass Code
	uchar bInterfaceProtocol;	// Interface Protocol Code
	uchar iInterface;			// Index of String Desc for this Interface
} interface_descriptor;



// Standard Endpoint Descriptor
// according to USB1.1 spec page 203
//
typedef struct
{
	uchar bLength;				// Size of this Descriptor in Bytes
	uchar bDescriptorType;		// Descriptor Type (=5)
	uchar bEndpointAddress;		// Endpoint Address (Number + Direction)
	uchar bmAttributes;			// Endpoint Attributes (Transfer Type)
	iword wMaxPacketSize;		// Max. Endpoint Packet Size
	uchar bInterval;			// Polling Interval (Interrupt) in ms
} endpoint_descriptor;



// Structure of Setup Packet sent during
// SETUP Stage of Standard Device Requests
// according to USB1.1 spec page 183
//
typedef struct
{
	uchar bmRequestType;		// Characteristics (Direction,Type,Recipient)
	uchar bRequest;				// Standard Request Code
	iword wValue;				// Value Field
	iword wIndex;				// Index or Offset Field
	iword wLength;				// Number of Bytes to transfer (Data Stage)
} setup_buffer;



// USB Status Codes
//
#define US_ATTACHED			0x00	// (not used here)
#define US_POWERED			0x01
#define US_DEFAULT			0x02
#define US_ADDRESSED		0x03
#define US_CONFIGURED		0x04
#define US_SUSPENDED		0x80



// USB Standard Device Request Codes
// according to USB1.1 spec page 187
//
#define GET_STATUS			0x00
#define CLEAR_FEATURE		0x01
#define SET_FEATURE			0x03
#define SET_ADDRESS			0x05
#define GET_DESCRIPTOR		0x06
#define SET DESCRIPTOR		0x07	// optional
#define GET_CONFIGURATION	0x08
#define SET_CONFIGURATION	0x09
#define GET_INTERFACE		0x0a
#define SET_INTERFACE		0x0b
#define SYNCH_FRAME			0x0c	// optional

#define REQUEST_COMPLETE	0xff	// not part of the Standard - just
									// a Flag to indicate that the recent
									// Request has been finished


// Descriptor Types
// according to USB1.1 spec page 187
//
#define DT_DEVICE			1
#define DT_CONFIGURATION	2
#define DT_STRING			3
#define DT_INTERFACE		4
#define DT_ENDPOINT			5



#endif


#include <iostream>
#include <stdint.h>
#include <Windows.h>
#include "Serial.h"


uint8_t gMemory[65536];
bool    gWritten[65536] = {0};
uint8_t gVerify[65536];
char	lineBuf[ 520 ];
int		lineBufIdx = 0;
Serial	gSerial;

#define SECONDS			1000000
#define MILLISECONDS	1000
#define MICROSECONDS	1


// SREC Record Format
// 'S' '1' C C  A A A A         [D D]x(CC-1)   M M
// 'S' '2' C C  A A A A A A     [D D]x(CC-1)   M M
// 'S' '3' C C  A A A A A A A A [D D]x(CC-1)   M M
// 'S' '7' C C  A A A A A A A A M M
// 'S' '8' C C  A A A A A A     M M
// 'S' '9' C C  A A A A         M M
// S0, S4, S5, S6 are ignored for now
// S2, S3, S7, S8 are ignored on this CPU as it only has a 16bit memory space



int HexToInt( unsigned char c )
{
	if( c >= '0' && c <= '9' )
		return (int)( c - '0' );
	if( c >= 'A' && c <= 'F' )
		return (int)( c - 'A' ) + 10;
	if( c >= 'a' && c <= 'f' )
		return (int)( c - 'a' ) + 10;
	return 0;
}

void SetMemory( uint32_t addr, uint8_t data )
{
	gMemory[addr] = data;
	gWritten[addr] = true;
}

void SetMemory( uint32_t addr, uint32_t bufIdx, uint8_t count )
{
	// Scan the line buffer for data and write it to our memory buffer
	for( int i=0; i<count; ++i )
	{
		SetMemory( addr++, (HexToInt(lineBuf[bufIdx++]) << 4) | HexToInt(lineBuf[bufIdx++]) );
	}
}

bool RowHasData( uint32_t addr, uint32_t rowLen )
{
	for( uint32_t i=0, j=addr; i<rowLen; ++i, ++j )
	{
		if( gMemory[j] != 0xFF )
			return true;
	}
	return false;
}

char SafeChar( char c )
{
	if( isprint( c ) )
		return c;
	return '-';
}

bool WaitForSerialData( uint32_t maxWaitUS )
{
	LARGE_INTEGER freq, startpc, currpc;
	QueryPerformanceFrequency( &freq );
	QueryPerformanceCounter( &startpc );
	while( gSerial.BytesWaiting() == 0 )
	{
		Sleep( 1 );
		QueryPerformanceCounter( &currpc );

		uint64_t delta = currpc.QuadPart - startpc.QuadPart;
		delta = (delta * 1000000) / freq.QuadPart;
		if( delta > maxWaitUS )
			return false;
	}
	return true;
}

bool WaitForOK( uint32_t maxWaitUS )
{
	if( WaitForSerialData(maxWaitUS) )
	{
		char buf;
		gSerial.ReadData( &buf, 1 );
		if( buf == 'K' )
		{
			return true;
		}
		else
		{
			printf( "WaitForOK() Did not get OK. Got 0x%02X '%c'\n", buf, SafeChar(buf) );
			return false;
		}
	}
	printf( "WaitForOK() Did not get a response\n" );
	return false;	
}

void ProgramRow( uint32_t addr, uint32_t count )
{
	while( gMemory[addr] == 0xFF )
	{
		addr++;
		count--;
	}

	uint8_t tx[4];

	// Start Row
	tx[0] = 0;
	tx[1] = (addr >> 8) & 0xFF;
	tx[2] =  addr & 0xFF;
	tx[3] = 0;
	gSerial.WriteData( (char *)tx, 4 );
	WaitForOK( 1 * SECONDS );

	// Program
	for( uint32_t i=0, j=addr; i<count; ++i, ++j )
	{
		tx[0] = 1;
		tx[1] = (j >> 8) & 0xFF;
		tx[2] =  j & 0xFF;
		tx[3] = gMemory[j];
		gSerial.WriteData( (char *)tx, 4 );
		WaitForOK( 1 * SECONDS );
	}

	// End Row
	tx[0] = 2;
	tx[1] = 0;
	tx[2] = 0;
	tx[3] = 0;
	gSerial.WriteData( (char *)tx, 4 );
	WaitForOK( 1 * SECONDS );
}

uint8_t ReadByte( uint32_t addr, int maxWaitUS )
{
	uint8_t tx[4];
	tx[0] = 4;
	tx[1] = (addr >> 8) & 0xFF;
	tx[2] =  addr & 0xFF;
	tx[3] = 0;
	gSerial.WriteData( (char *)tx, 4 );

	tx[0] = 0xFF;
	if( WaitForSerialData( maxWaitUS ) )
	{
		gSerial.ReadData( (char *)tx, 1 );
		if( WaitForOK( maxWaitUS ) )
		{
			return tx[0];
		}
	}
	return 0xFF;
}

void WriteByte( uint32_t addr, uint8_t data, int maxWaitUS )
{
	uint8_t tx[4];
	tx[0] = 1;
	tx[1] = (addr >> 8) & 0xFF;
	tx[2] =  addr & 0xFF;
	tx[3] = data;
	gSerial.WriteData( (char *)tx, 4 );
	WaitForOK( maxWaitUS );
}

void MassErase( uint32_t timeout )
{
	uint8_t tx[4];
	tx[0] = 3;
	tx[1] = 0;
	tx[2] = 0;
	tx[3] = 0;
	gSerial.WriteData( (char *)tx, 4 );
	WaitForOK( timeout );
}

bool ResetHC908( uint32_t timeout )
{
	uint8_t tx[4];
	tx[0] = 5;
	tx[1] = 0;
	tx[2] = 0;
	tx[3] = 0;
	gSerial.WriteData( (char *)tx, 4 );
	return WaitForOK( timeout );
}

bool SendSecurity( uint32_t timeout )
{
	uint8_t tx[4];
	tx[0] = 6;
	tx[1] = 0;
	tx[2] = 0;
	tx[3] = 0;
	gSerial.WriteData( (char *)tx, 4 );
	return WaitForOK( timeout );
}



void ProcessRecord()
{
	int count = 0;
	int addr = 0; // 16 bit address

	count = HexToInt( lineBuf[ 2 ] ) << 4;
	count |= HexToInt( lineBuf[ 3 ] );

	switch( lineBuf[ 1 ] )
	{
		case '1':
			// Note : [count] includes the 2 address bytes and the checksum byte
			addr = HexToInt( lineBuf[ 4 ] ) << 12;
			addr |= HexToInt( lineBuf[ 5 ] ) << 8;
			addr |= HexToInt( lineBuf[ 6 ] ) << 4;
			addr |= HexToInt( lineBuf[ 7 ] );
			SetMemory( addr, 8, count-3 ); // skip the 4 address characters in the count 
			break;
		case '9':
			break;
	}
}



void ProcessByte( unsigned char b )
{
	if( lineBufIdx == 0 ) // if we are waiting for the start of line
	{
	  // Waiting for 'S', ignore all other bytes
		if( b == 'S' )
			lineBuf[ lineBufIdx++ ] = b;
	}
	else
	{
		if( b == '\r' || b == '\n' )
		{
			ProcessRecord();
			lineBufIdx = 0;
		}
		else
		{
			lineBuf[ lineBufIdx++ ] = b;
		}
	}
}





int main( int argc, char * argv[] )
{
	std::vector<Serial::PortAvail> ports;
	gSerial.Enum( ports );
	for( size_t i=0; i<ports.size(); ++i )
	{
		printf( "COM%d %s\n", ports[i].port, ports[i].busy?"BUSY":"available" );
	}


	int portIdx = 0;
	if( ports.size() == 1 )
	{
		portIdx = ports[0].port;
	}
	else if( argc > 2 )
	{
		portIdx = atoi( argv[2] );
	}


	if( gSerial.Open( portIdx, 250000, Serial::Parity::None, Serial::StopBits::StopBits_2 ) )
	{
		memset( gMemory, 0xFF, sizeof( gMemory ) );


		FILE* f = nullptr;
		const char* path = argv[1];
		fopen_s( &f, path, "rb" );
		if( f != nullptr )
		{
			int c = fgetc( f );
			while( !feof( f ) )
			{
				ProcessByte( c );
				c = fgetc( f );
			}
			fclose( f );
		}


		// Clear serial buffer
		char temp;
		while( gSerial.BytesWaiting() )
			gSerial.ReadData( &temp, 1 );


		printf( "Reseting\n" );
		if( ResetHC908(20*SECONDS) )
		{
		printf( "Sending Security Bytes\n" );
			if( SendSecurity(20*SECONDS) )
			{
		uint8_t security = ReadByte( 0x0040, 1*SECONDS );
		printf( "Security Byte = %02X : %s\n", security, (security&(1<<6))?"Passed":"Failed" );
		WriteByte( 0x0040, 0x55, 1*SECONDS );
		security = ReadByte( 0x0040, 1*SECONDS );
		printf( "Test RAM Changed to = %02X\n", security );
		WriteByte( 0x0040, 0xAA, 1*SECONDS );
		security = ReadByte( 0x0040, 1*SECONDS );
		printf( "Test RAM Changed to = %02X\n", security );


		uint8_t flashBlockProtect = ReadByte( 0xFE09, 1*SECONDS );
		printf( "Flash Block Protect Byte = %02X\n", flashBlockProtect );
		if( flashBlockProtect != 0xFF )
		{
			WriteByte( 0xFE09, 0xFF, 1*SECONDS ); // set the protection range so no part of flash is protected
			flashBlockProtect = ReadByte( 0xFE09, 1*SECONDS );
			printf( "Flash Block Protect Byte = %02X\n", flashBlockProtect );
		}




		// On the 68HC908, FLASH is programmed in 64 byte rows.
		// This means programming addresses have the lowest 6 bits at zero (i.e. addr & FFC0)
		// Programming can start anywhere in the row and it appears that the bytes don't even need to be written consecutively BUT...
		// When advancing to a new row, the PGM and HVEN bits need to be toggled in a specific sequence.
		printf( "Erasing\n" );
		MassErase(20*SECONDS);


		for( int i = 0; i < 65536; i += 64 )
		{
			if( RowHasData( i, 64 ) )
			{
				printf( "Programming Row Address %04X\n", i );
				ProgramRow( i, 64 );
			}
		}


		// Read back the chip
		bool verifySuccess = true;
		for( int i = 0; i < 65536; ++i )
		{
			if( gWritten[i] )
			{
				if( (i & 0x00FF) == 0 )
					printf( "Reading Address %04X\n", i );
				gVerify[i] = ReadByte( i, 1*SECONDS );

				if( gVerify[i] != gMemory[i] )
				{
					printf( "Verify failed at address %04X wrote %d but got %d\n", i, gMemory[i], gVerify[i] );
					verifySuccess = false;
				}
				else
				{
					printf( "Verify OK %04X\n", i );
				}
			}
		}

		if( verifySuccess == true )
		{
			printf( "Verify Succeeded!\n" );
		}
			}
		}

		gSerial.Close();
	}
	else
	{
		printf( "Unable to open COM%d\n", portIdx );
	}
}

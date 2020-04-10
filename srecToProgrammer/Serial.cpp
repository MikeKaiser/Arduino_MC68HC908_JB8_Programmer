/*============================================================
 * Copyright (c) 2011 - 2020  Procedural Worlds Ltd
 * Author : Mike Kaiser
 * 
 * Use and copying of this software and preparation of derivative works
 * based upon this software are not permitted without written consent
 * from Procedural Worlds Ltd.
 * Any copy of this software or of any derivative work must include the
 * this copyright notice. Any distribution of this software or
 * derivative works must comply with all applicable laws.
 * ==========================================================*/



#include "Serial.h"
#include <windows.h>		// Header File For Windows

Serial::Serial( int port, int baud, Serial::Parity parity, Serial::StopBits stopBits )
	: h( INVALID_HANDLE_VALUE )
{
	Open( port, baud, parity, stopBits );
}

Serial::Serial()
{
}

bool Serial::Open( int port, int baud, Serial::Parity parity, Serial::StopBits stopBits )
{
	if( h != INVALID_HANDLE_VALUE )
		Close();

	char sPort[256];
	sprintf_s( sPort, "\\\\.\\COM%u", port );
	h = CreateFileA( sPort, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	DCB dcb;
	memset(&dcb, 0, sizeof(dcb));
	dcb.DCBlength = sizeof(DCB);

	if( GetCommState(h, &dcb) == FALSE )
		return false;

	dcb.fBinary  = true;
	dcb.BaudRate = baud;
	dcb.ByteSize = 8;
	dcb.StopBits = stopBits;//ONESTOPBIT;
	dcb.Parity   = parity;//NOPARITY;
	dcb.fParity  = false;
	// Prevent reset of ardiuino
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;

	SetCommState(h, &dcb);
	return true;
}

void Serial::Close()
{
	CloseHandle( h );
	h = INVALID_HANDLE_VALUE;
}

Serial::~Serial()
{
	Close();
}


bool Serial::IsOpen()
{
	return h != INVALID_HANDLE_VALUE;
}

unsigned int Serial::BytesWaiting()
{
	DWORD errors = 0;
	COMSTAT stat;
	ClearCommError( h, &errors, &stat );
	return stat.cbInQue;
}

bool Serial::WriteData( const char * buffer, int bufferLen )
{
	DWORD	bytesWritten;
	BOOL retVal = WriteFile( h, buffer, bufferLen, &bytesWritten, nullptr );
	return retVal?true:false;
}

int Serial::ReadData( char * buffer, int bufferLen )
{
	DWORD bytesWaiting = BytesWaiting();
	if( 0 == bytesWaiting )
		return 0;

	DWORD bytesToRead = (bytesWaiting > (DWORD)bufferLen)?bufferLen:bytesWaiting;
	DWORD bytesRead = 0;
	BOOL ret = ReadFile( h, buffer, bytesToRead, &bytesRead, nullptr );

	return (int)bytesRead;
}

void Serial::Enum( std::vector<PortAvail>& devices )
{
	devices.clear();

	//Up to 255 COM ports are supported so we iterate through all of them seeing
	//if we can open them or if we fail to open them, get an access denied or general error error.
	//Both of these cases indicate that there is a COM port at that number. 
	for( UINT i = 1; i < 256; i++ )
	{
	    //Form the Raw device name
		char sPort[256];
		sprintf_s( sPort, "\\\\.\\COM%u", i );

		//Try to open the port
		bool bSuccess = false;
		bool bBusy = false;
		HANDLE port = CreateFileA( sPort, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr );
		if( port == INVALID_HANDLE_VALUE )
		{
			const DWORD dwError = GetLastError();

			//Check to see if the error was because some other app had the port open or a general failure
			if( (dwError == ERROR_ACCESS_DENIED) || (dwError == ERROR_GEN_FAILURE) || (dwError == ERROR_SHARING_VIOLATION) || (dwError == ERROR_SEM_TIMEOUT) )
			{
				bSuccess = true;
				bBusy = true;
			}
		}
		else
		{
		  //The port was opened successfully
			bSuccess = true;
			bBusy = false;
			CloseHandle( port );
		}

		//Add the port number to the array which will be returned
		if( bSuccess )
		{
			devices.push_back( PortAvail( i, bBusy ) );
		}
	}
}

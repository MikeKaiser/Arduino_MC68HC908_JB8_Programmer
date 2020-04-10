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



#pragma once

#ifndef SERIAL_H
#define SERIAL_H

#ifndef _WINDEF_
typedef void * HANDLE;
#endif

#include <vector>
#include <string>

class Serial
{
	HANDLE	h;

public:
    struct PortAvail
    {
        int port;
        bool busy; // port is open by another application
        PortAvail( int p, bool b )
            : port( p )
            , busy( b )
        {
        }
    };

    enum Parity
    {
		None = 0,
        Odd,
        Even,
        Mark,
        Space,
    };

    enum StopBits
    {
		StopBits_1 = 0,
        StopBits_15,
        StopBits_2,
    };

	// Serial Constructor
	//		port : port to open e.g. "COM1". If trying to reach a serial port higher than 9 you must use "\\\\.\\COM10"
	//		baud : data rate... You can use the windows constants e.g. CBR_9600
	Serial( int port, int baud, Parity parity, StopBits stopBits );
	Serial();

	~Serial();

    bool Open( int port, int baud, Parity parity, StopBits stopBits );
    void Close();
    unsigned int BytesWaiting();
    bool IsOpen();
    int  ReadData( char * buffer, int bufferLen );
	bool WriteData( const char * buffer, int bufferLen );
    void Enum( std::vector<PortAvail> & devices );
};

#endif

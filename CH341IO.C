#include    <windows.h>
#include    <tchar.h>
#include    <commctrl.h>
#include    "ch341dp.h"
#include    "CH341IO.H"
#include    "CH341DLL.H"
#include    "resource.h"
#include    "MSHOW.H"

const PTCHAR McuName[ ] = { _T("MEGA AUTO"), _T("MEGA8"), _T("MEGA16"), _T("MEGA32"), 
    _T("MEGA64"), _T("MEGA128"), _T("MEGA8515"), _T("MEGA8535"), _T("MEGA48"), 
    _T("MEGA88"), _T("MEGA168"), _T("AT89S51"), _T("AT89S52") };

/*==============================================================================

Function name: MegaSpiEnable

Function: Set the SPI of CH341 to D1 S_RST D3 to S_CLK D5 to S_MOSI D7 to S_MOSO

==============================================================================*/
BOOL    WINAPI  MegaSpiEnable( ULONG index )
{
UCHAR   mBuffer[ mCH341_PACKET_LENGTH ];
ULONG   i;

    i = 0;
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STREAM;            // Command code
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_OUT | 0x00;    // default status: all 0
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_DIR | 0x2A;    // D1 & D3 & D5 output, other input
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_US | 32;       // Time delay of 32 microseconds
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_END;           // The current command package ends early
    return( CH341WriteData( index, mBuffer, &i ) );     // Execute the data flow command
}

/*==============================================================================

Function name: S51SpiEnable

Function: Set the SPI of CH341 to D1 S_RST D3 to S_CLK D5 to S_MOSI D7 to S_MOSO

==============================================================================*/
BOOL    WINAPI  S51SpiEnable(
    ULONG       index )  // Specify the serial number of the CH341 device
{
    UCHAR   mBuffer[ mCH341_PACKET_LENGTH ];
    ULONG   i;
    i = 0;
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STREAM;            // Command code
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_OUT | 0x02;    // D1 =1 other status: all 0
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_DIR | 0x2A;    // D1 & D3 & D5 output, other input
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_US | 32;       // Time delay of 32 microseconds
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_END;           // The current command package ends early
    return( CH341WriteData( index, mBuffer, &i ) );     // Execute the data flow command
}

/*==============================================================================

Function name: SpiRunMega

What it does: Let the MEGA MCU run while the SPI pins are in a high-impedance state

==============================================================================*/
BOOL WINAPI SpiRunMega( ULONG   index )
{
UCHAR   mBuffer[ mCH341_PACKET_LENGTH ];
ULONG   i;

    if( MegaSpiEnable(index) == FALSE )
        return FALSE;

    i = 0;
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STREAM;            // Command code
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_OUT |0x02;     // D1=1
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_END;
    if(CH341WriteData( index, mBuffer, &i ) == FALSE )
        return FALSE;

    i = 0;
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STREAM;            // Command code
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_OUT |0x00;     // D1=0
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_END;
    if(CH341WriteData( index, mBuffer, &i ) == FALSE )
        return FALSE;

	i = 0;
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STREAM;            // Command code
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_OUT |0x02;     // D1=1
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_END;
    if(CH341WriteData( index, mBuffer, &i ) == FALSE )
        return FALSE;
	// Above generates a low level, resetting the MCU

    i = 0;
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STREAM;            // Command code
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_OUT | 0xff;    // all status: 1
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_DIR | 0x00;    // Enter them all
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_END;           // The current command package ends early
    return( CH341WriteData( index, mBuffer, &i ) );     // Execute the data flow command
}

/*==============================================================================

Function name: SpiRunAt89s

What the function does: Let the At89s MCU run while the SPI pin is in a high-impedance state

==============================================================================*/
BOOL WINAPI SpiRunAt89s( ULONG   index )
{
UCHAR   mBuffer[ mCH341_PACKET_LENGTH ];
ULONG   i;

    if( S51SpiEnable(index) == FALSE )
        return FALSE;

    i = 0;
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STREAM;            // Command code
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_OUT |0x00;     // D1=0
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_END;
    if(CH341WriteData( index, mBuffer, &i ) == FALSE )
        return FALSE;

    i = 0;
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STREAM;            // Command code
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_OUT |0x02;     // D1=1
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_END;
    if(CH341WriteData( index, mBuffer, &i ) == FALSE )
        return FALSE;

	i = 0;
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STREAM;            // Command code
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_OUT |0x00;     // D1=0
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_END;
    if(CH341WriteData( index, mBuffer, &i ) == FALSE )
        return FALSE;
	// The above generates a high level and resets the MCU

    i = 0;
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STREAM;            // Command code
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_OUT | 0xfD;    // all status: 1
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_DIR | 0x02;    // Enter them all
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_END;           // The current command package ends early
    return( CH341WriteData( index, mBuffer, &i ) );     // Execute the data flow command
}

/*==============================================================================

Function name: SpiResetAT89s

Function: The SPI outputs data to reset the AT89S
==============================================================================*/
BOOL    WINAPI  SpiResetAT89s(
    ULONG       index )  // Specify the serial number of the CH341 device
{
    UCHAR   mBuffer[ mCH341_PACKET_LENGTH ];
    ULONG   i;
    i = 0;
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STREAM;            // Command code
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_OUT | 0x00;    // status: all 0
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_DIR | 0x2A;    // D1 & D3 & D5 output, other input
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_END;           // The current command package ends early
    return( CH341WriteData( index, mBuffer, &i ) );     // Execute the data flow command
}

/*==============================================================================

Function name: SpiResetMega

What the function does: SPI outputs data to reset mega (generates a positive pulse (at least two machine cycles) to reset the MCU)

==============================================================================*/
BOOL    WINAPI  SpiResetMega( ULONG   index )
{
UCHAR  mBuffer[ mCH341_PACKET_LENGTH];
ULONG i;

    i = 0;
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STREAM;            // Command code
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_OUT |0x02;     // D1=1
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_END;
    if(CH341WriteData( index, mBuffer, &i ) == FALSE )
        return FALSE;

    i = 0;
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STREAM;            // Command code
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_OUT |0x00;     // D1=0
    mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_END;
    return (CH341WriteData( index, mBuffer, &i ) );
}

#define     DELAY_US        4
/*==============================================================================

Function name: MegaSpiOutInData

Function: Output data to the MCU and get the returned data at the same time

==============================================================================*/
BOOL    WINAPI  MegaSpiOutInData(
    ULONG   index,          // Specify the serial number of the CH341 device
    ULONG   OutLen,         // The length of the output data
    PVOID   DataBuf,        // The output data buffer is shared with the SPI data return
    BOOL    SpiBit )       // Whether to use SPI4
{
PUCHAR p;
ULONG i, j, k;
UCHAR c, tem;
UCHAR  mBuffer[ 512 ];

    // 202334066…œ + 6.95us - 5.85us
    //SpiBit = FALSE;
    if( !SpiBit )
    {
        p = DataBuf;
        k = 0;
        for( i = 0; i != OutLen; i++ )
        {
            tem = *p;
            mBuffer[ k++ ] = mCH341A_CMD_UIO_STREAM;            // Command code
            for( j = 0; j != 4; j++ )
            {
                c = tem & 0x80 ? 0x20 : 0;                              // D5 Data Output
                mBuffer[ k++ ] = mCH341A_CMD_UIO_STM_OUT | c;           // D3=0, I/O_CLOCK=LOW 
                mBuffer[ k++ ] =mCH341A_CMD_UIO_STM_US | DELAY_US;       
                mBuffer[ k++ ] = mCH341A_CMD_UIO_STM_OUT | c | 0x08;    // D3=1, I/O_CLOCK=HIGH
                mBuffer[ k++ ] = mCH341A_CMD_UIO_STM_IN;                // input from D7
                mBuffer[ k++ ] =mCH341A_CMD_UIO_STM_US | DELAY_US;
                tem <<= 1;
            }
            mBuffer[ k++ ] = mCH341A_CMD_UIO_STM_OUT | c | 0x00;        // D3=0, I/O_CLOCK=LOW
            mBuffer[ k++ ] = mCH341A_CMD_UIO_STM_END;                   // The current command package ends early
            k += mCH341_PACKET_LENGTH - 1;
            k &= ~ ( mCH341_PACKET_LENGTH - 1 );

            mBuffer[ k++ ] = mCH341A_CMD_UIO_STREAM;            // Command code
            for( j = 0; j != 4; j++ )
            {
                c = tem & 0x80 ? 0x20 : 0;                              // D5 Data Output
                mBuffer[ k++ ] = mCH341A_CMD_UIO_STM_OUT | c;           // D3=0, I/O_CLOCK=LOW        
                mBuffer[ k++ ] = mCH341A_CMD_UIO_STM_US | DELAY_US; 
                mBuffer[ k++ ] = mCH341A_CMD_UIO_STM_OUT | c | 0x08;    // D3=1, I/O_CLOCK=HIGH
                mBuffer[ k++ ] = mCH341A_CMD_UIO_STM_IN;                // input from D7
                mBuffer[ k++ ] = mCH341A_CMD_UIO_STM_US | DELAY_US;
                tem <<= 1;
            }
            mBuffer[ k++ ] = mCH341A_CMD_UIO_STM_OUT | c | 0x00;        // D3=0, I/O_CLOCK=LOW
            mBuffer[ k++ ] = mCH341A_CMD_UIO_STM_END;                   // The current command package ends early
            k += mCH341_PACKET_LENGTH - 1;
            k &= ~ ( mCH341_PACKET_LENGTH - 1 );
            p++;
        }

        i = CH341WriteRead( index, k, mBuffer, 4, OutLen*2, &j, mBuffer );
        if( i == FALSE )
            return( FALSE );

        if( j != OutLen * 8 )
            return( FALSE );

        k = 0;
        for ( i = 0; i != OutLen; i++ )
        {
            c = 0;
            for ( j = 0; j < 8; j ++ )
            {
                c <<= 1;
                if ( mBuffer[ k++ ] & 0x80 )        // input 8 bit
                    c ++;
            }

            *(PUCHAR)DataBuf = c;
            ((PUCHAR)DataBuf)++;
        }
        return TRUE;
    }
    else
    {
        return CH341StreamSPI4( index, 0x00, OutLen, DataBuf );
    }
}

/*==============================================================================

Function name: At89sISPoutput

Function: Output data to the MCU

==============================================================================*/
BOOL    WINAPI  At89sISPoutput(
    ULONG       index,      // Specify the serial number of the CH341 device
    ULONG   OutLen,         // The length of the output data
    LPCSTR  OutBuf )        // Output data buffers
{
UCHAR   mBuffer[ mCH341_PACKET_LENGTH * 128 ];
UCHAR   c, cmd, tem;
ULONG   cnt, i, j;
ULONG   ThisLen;
cmd = *OutBuf;

    while ( OutLen )
    {
        ThisLen = OutLen > 128 ? 128 : OutLen;
        OutLen -= ThisLen;
        i = 0;
        for ( cnt = 0; cnt < ThisLen; cnt++ )
        {
            mBuffer[ i++ ] = mCH341A_CMD_UIO_STREAM;  // Command code
            tem = *OutBuf;
            for ( j = 0; j < 8; j ++ )
            {  // output 8 bit
                c = tem & 0x80 ? 0x20 : 0;  // D5
                mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_OUT | c | SET_CH341_D1;            // D3=0, I/O_CLOCK=LOW
                mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_OUT | c | 0x08 | SET_CH341_D1;     // D3=1, I/O_CLOCK=HIGH
                tem <<= 1;
            }
            OutBuf ++;
            mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_OUT | c | 0x00 | SET_CH341_D1;         // D3=0, I/O_CLOCK=LOW
            // delay for program byte
            if ( cmd == 0x50 )
            { // page write command
                mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_US | 63;  // Time delay 63 microseconds, 0-63
                mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_US | 63;  // Time delay 63 microseconds
                mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_US | 63;  // Time delay 63 microseconds
                mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_US | 63;  // Time delay 63 microseconds
                mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_US | 63;  // Time delay 63 microseconds
                mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_US | 63;  // Time delay 63 microseconds
                mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_US | 63;  // Time delay 63 microseconds
                mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_US | 63;  // Time delay 63 microseconds
                mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_US | 63;  // Time delay 63 microseconds
                mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_US | 63;  // Time delay 63 microseconds
            }  // delay > 500us

            mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_END;  // The current command package ends early, Because one package can't fit, it's split into two packages
            i += mCH341_PACKET_LENGTH - 1;
            i &= ~ ( mCH341_PACKET_LENGTH - 1 );
        }
        if ( CH341WriteData( index, mBuffer, &i ) == FALSE )
            return( FALSE );
    }
    return( TRUE );  // Execute the data flow command
}

/*==============================================================================

Function name: At89sISPoutin

Function: Output data to the MCU to get the returned data

==============================================================================*/
BOOL    WINAPI  At89sISPoutin(
    ULONG           index,  // Specify the serial number of the CH341 device
    ULONG   OutLen,         //The length of the output data
    LPCSTR  OutBuf,         //Output data buffers
    ULONG   InLen,          //Prepare to read the data length
    PUCHAR  InBuf )         //Input data buffers
{
UCHAR   mBuffer[ mCH341_PACKET_LENGTH * 128 ];
UCHAR   c,cmd;
ULONG   cnt, i, j, mLength;
ULONG   ThisLen;

    if ( OutLen > sizeof( mBuffer ) / 32 - 1 ) return( FALSE ); // buffer over
    i = 0;
    for ( cnt = 0; cnt < OutLen; cnt++ )
    {
        mBuffer[ i++ ] = mCH341A_CMD_UIO_STREAM;  // Command code
        cmd = *OutBuf;
        for ( j = 0; j < 8; j ++ )
        {  // output 8 bit
            c = cmd & 0x80 ? 0x20 : 0;  // D5
            mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_OUT | c | SET_CH341_D1;  // D3=0, I/O_CLOCK=LOW
            mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_OUT | c | 0x08 | SET_CH341_D1;  // D3=1, I/O_CLOCK=HIGH
            cmd <<= 1;
        }
        OutBuf ++;
        mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_OUT | c | 0x00 | SET_CH341_D1;  // D3=0, I/O_CLOCK=LOW
        mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_END;  // The current command package ends early,Because one package can't fit, it's split into two packages
        i += mCH341_PACKET_LENGTH - 1;
        i &= ~ ( mCH341_PACKET_LENGTH - 1 );
    }
    while( InLen )
    {
        j = ( sizeof( mBuffer ) - i ) / 32;
        ThisLen = InLen > j ? j : InLen;
        InLen -= ThisLen;
        for ( cnt = 0; cnt < ThisLen; cnt++ )
        {
            mBuffer[ i++ ] = mCH341A_CMD_UIO_STREAM;  // Command code
            for ( j = 0; j < 8; j ++ )
            {  // input 8 bit
                mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_OUT | 0x08 | SET_CH341_D1;  // D3=1, I/O_CLOCK=HIGH
                mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_IN;  // input from D7
                mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_OUT | 0x00 | SET_CH341_D1;  // D3=0, I/O_CLOCK=LOW
            }
            mBuffer[ i++ ] = mCH341A_CMD_UIO_STM_END;  // The current command package ends early, Because one package can't fit, it's split into two packages
            i += mCH341_PACKET_LENGTH - 1;
            i &= ~ ( mCH341_PACKET_LENGTH - 1 );
        }
        mLength = 0;
        j = CH341WriteRead( index, i, mBuffer, 8, ThisLen, &mLength, mBuffer );
        // Execute the data flow command,Output before input, perform two inputs, each with a maximum of 8 bytes
        if ( j == FALSE ) return( FALSE );
        if ( mLength != ThisLen * 8 )
        {
            //printf("length=%d,%d\n",mLength,ThisLen);
            return( FALSE );  // size error
        }
        i = 0;
        for ( cnt = 0; cnt < ThisLen; cnt++ )
        {
            c = 0;
            for ( j = 0; j < 8; j ++ ) {  // input 8 bit
                c <<= 1;
                if ( mBuffer[ i++ ] & 0x80 ) c ++;
            }
            *InBuf = c;
            InBuf ++;
        }
        i = 0;
    }
    return( TRUE );
}

/*==============================================================================

Function name: IniMcuDown

Function: Initialize the MEGA series MCU programming download

==============================================================================*/
UCHAR WINAPI IniMcuDown( void )
{
UCHAR  CmdBuf[32];
UCHAR  tem8;

    if( (McuType >= AUTO_MCU_MODEL) && (McuType < ATMEL_AT89S51_MODEL) )
    {
        if( MegaSpiEnable( Ch341index ) == FALSE)   // Initialize the SPI of the CH341
            return DATA_TRANS_ERR;
        if( SpiResetMega(Ch341index ) == FALSE )
            return DATA_TRANS_ERR;                  // ResetMEGA8
        Sleep( 200 );                                // The reset delay is at least 20ms

        CmdBuf[0] = 0xAC;                           // Programming Enable
        CmdBuf[1] = 0x53;
        CmdBuf[2] = 0xff;
        if( MegaSpiOutInData( Ch341index, 4, CmdBuf, FALSE ) == FALSE )
            return DATA_TRANS_ERR;
         if( CmdBuf[2] != 0x53 )
            return NO_MCU_ERR;                      // This is not a recognized MCU

        CmdBuf[0] = 0x30;                           // Read Signature Bytes
        CmdBuf[1] = 0x00;
        CmdBuf[2] = 0x00;
        if( MegaSpiOutInData( Ch341index, 4, CmdBuf, FALSE ) == FALSE )
            return DATA_TRANS_ERR;
        //MHEX( CmdBuf, 4, "s0" );
        if( CmdBuf[3] != 0x1E )                     // IE stands for Manufactured by Atmel Corporation
            return MCU_MODEL_ERR;

        CmdBuf[0] = 0x30;                           // Read Signature Bytes
        CmdBuf[1] = 0x00;
        CmdBuf[2] = 0x01;
        if( MegaSpiOutInData( Ch341index, 4, CmdBuf, FALSE ) == FALSE )
            return DATA_TRANS_ERR;
        //MHEX( CmdBuf, 4, "s1" );
        if( McuType == AUTO_MCU_MODEL )             // Automatic selection MCU, the FLASH space is not detected
        {
            tem8 = CmdBuf[3];
            goto P_NEXT;
        }
        if( McuType == ATMEL_MEGA8_MODEL )
        {
            if( CmdBuf[3] != 0x93 )                 // Indicates that the chip contains 8KB Flash storage
                return MCU_MODEL_ERR;
        }
        else if( McuType == ATMEL_MEGA16_MODEL )
        {
            if( CmdBuf[3] != 0x94 )                 // Indicates that the chip contains 16KB Flash storage
                return MCU_MODEL_ERR;
        }
        else if( McuType == ATMEL_MEGA32_MODEL )
        {
            if( CmdBuf[3] != 0x95 )                 // Indicates that the chip contains 32KB Flash storage
                return MCU_MODEL_ERR;
        }
        else if( McuType == ATMEL_MEGA64_MODEL )
        {
            if( CmdBuf[3] != 0x96 )                 // Indicates that the chip contains 64KB Flash storage
                return MCU_MODEL_ERR;
        }
        else if( McuType == ATMEL_MEGA128_MODEL )
        {
            if( CmdBuf[3] != 0x97 )                 // Indicates that the chip contains 128KB Flash storage
                return MCU_MODEL_ERR;
        }
        else if( McuType == ATMEL_MEGA8515_MODEL )
        {
            if( CmdBuf[3] != 0x93 )                 // Indicates that the chip contains 8KB Flash storage
                return MCU_MODEL_ERR;
        }
        else if( McuType == ATMEL_MEGA8535_MODEL )
        {
            if( CmdBuf[3] != 0x93 )                 // Indicates that the chip contains 8KB Flash storage
                return MCU_MODEL_ERR;
        }
        else if( McuType == ATMEL_MEGA48_MODEL )
        {
            if( CmdBuf[3] != 0x92 )                 // Indicates that the chip contains 4KB Flash storage
                return MCU_MODEL_ERR;
        }
        else if( McuType == ATMEL_MEGA88_MODEL )
        {
            if( CmdBuf[3] != 0x93 )                 // Indicates that the chip contains 8KB Flash storage
                return MCU_MODEL_ERR;
        }
        else if( McuType == ATMEL_MEGA168_MODEL )
        {
            if( CmdBuf[3] != 0x94 )                 // Indicates that the chip contains 16KB Flash storage
                return MCU_MODEL_ERR;
        }
        else
        {
            return MCU_MODEL_ERR;                   // This model of MCU is not supported
        }

        P_NEXT:
        CmdBuf[0] = 0x30;                           //°°Model inquiry
        CmdBuf[1] = 0x00;
        CmdBuf[2] = 0x02;
        if( MegaSpiOutInData( Ch341index, 4, CmdBuf, FALSE ) == FALSE )
            return DATA_TRANS_ERR;
        //MHEX( CmdBuf, 4, "s2" );
        if( McuType == AUTO_MCU_MODEL )
        {
            if( ( CmdBuf[3] == 0x07 ) && ( tem8 == 0x93) )
                McuType = ATMEL_MEGA8_MODEL;
            else if( ( CmdBuf[3] == 0x03 ) && ( tem8 == 0x94 ) )
                McuType = ATMEL_MEGA16_MODEL;
            else if( ( CmdBuf[3] == 0x02 ) && ( tem8 == 0x95 ) )
                McuType = ATMEL_MEGA32_MODEL;
            else if( ( CmdBuf[3] == 0x02 ) && ( tem8 == 0x96 ) )
                McuType = ATMEL_MEGA64_MODEL;
            else if( ( CmdBuf[3] == 0x02 ) && ( tem8 == 0x97 ) )
                McuType = ATMEL_MEGA128_MODEL;
            else if( ( CmdBuf[3] == 0x06 ) && ( tem8 == 0x93 ) )
                McuType = ATMEL_MEGA8515_MODEL;
            else if( ( CmdBuf[3] == 0x08 ) && ( tem8 == 0x93 ) )
                McuType = ATMEL_MEGA8535_MODEL;
            else if( ( CmdBuf[3] == 0x05 ) && ( tem8 == 0x92 ) )
                McuType = ATMEL_MEGA48_MODEL;
            else if( ( CmdBuf[3] == 0x0a ) && ( tem8 == 0x93 ) )
                McuType = ATMEL_MEGA88_MODEL;
            else if( ( CmdBuf[3] == 0x06 ) && ( tem8 == 0x94 ) )
                McuType = ATMEL_MEGA168_MODEL;
            else
                return NO_SUPORT_ERR;
        }
        else if( McuType == ATMEL_MEGA8_MODEL )
        {
            if( CmdBuf[3] != 0x07 )                 // The model is: MEGA8
                return MCU_MODEL_ERR;
        }
        else if( McuType == ATMEL_MEGA16_MODEL )
        {
            if( CmdBuf[3] != 0x03 )                 // The model is: MEGA16
                return MCU_MODEL_ERR;
        }
        else if( McuType == ATMEL_MEGA32_MODEL )
        {
            if( CmdBuf[3] != 0x02 )                 // The model is: MEGA32
                return MCU_MODEL_ERR;
        }
        else if( McuType == ATMEL_MEGA64_MODEL )
        {
            if( CmdBuf[3] != 0x02 )                 // The model is: MEGA64
                return MCU_MODEL_ERR;
        }
        else if( McuType == ATMEL_MEGA128_MODEL )
        {
            if( CmdBuf[3] != 0x02 )                 // The model is: MEGA128
                return MCU_MODEL_ERR;
        }
        else if( McuType == ATMEL_MEGA8515_MODEL )
        {
            if( CmdBuf[3] != 0x06 )                 // The model is: MEGA8515
                return MCU_MODEL_ERR;
        }
        else if( McuType == ATMEL_MEGA8535_MODEL )
        {
            if( CmdBuf[3] != 0x08 )                 // The model is: MEGA8535
                return MCU_MODEL_ERR;
        }
        else if( McuType == ATMEL_MEGA48_MODEL )
        {
            if( CmdBuf[3] != 0x05 )                 // The model is: MEGA48
                return MCU_MODEL_ERR;
        }
        else if( McuType == ATMEL_MEGA88_MODEL )
        {
            if( CmdBuf[3] != 0x0a )                 // The model is: MEGA48
                return MCU_MODEL_ERR;
        }
        else if( McuType == ATMEL_MEGA168_MODEL )
        {
            if( CmdBuf[3] != 0x06 )                 // The model is: MEGA168
                return MCU_MODEL_ERR;
        }
        else
        {
            return MCU_MODEL_ERR;                   // This model of MCU is not supported
        }
    }
    else                                            // AT89Sseries
    {
		if( SpiResetAT89s( Ch341index ) == FALSE )  // ResetAT89S
            return DATA_TRANS_ERR;
        Sleep( 1 );

        if( S51SpiEnable( Ch341index ) == FALSE )   // Enter programming mode
            return DATA_TRANS_ERR;
        Sleep( 100 );

        CmdBuf[0] =0XAC;
        CmdBuf[1] =0x53;
        CmdBuf[2] =0x00;                            // 89S51 Chip programmable query
        if( At89sISPoutin( Ch341index, 3, CmdBuf, 1, CmdBuf ) == FALSE )
            return DATA_TRANS_ERR;
        if( CmdBuf[0] != 0x69)
            return NO_MCU_ERR;

        Sleep( 20 );

        CmdBuf[0] =0X28;
        CmdBuf[1] =0x00;
        CmdBuf[2] =0x00;                            // Read AT89S series MCU flag
        if( At89sISPoutin( Ch341index, 3, CmdBuf, 1, CmdBuf ) == FALSE )
            return DATA_TRANS_ERR;
        if( CmdBuf[0] != 0x1E)
            return NO_MCU_ERR;
        // ºÏ≤‚ flag

        CmdBuf[0] =0X28;
        CmdBuf[1] =0x01;
        CmdBuf[2] =0x00;                            // Read AT89S series MCU flag
        if( At89sISPoutin( Ch341index, 3, CmdBuf, 1, CmdBuf ) == FALSE )
            return DATA_TRANS_ERR;

        if( McuType == ATMEL_AT89S51_MODEL )
        {
            if( CmdBuf[0] != 0x51 )
                return MCU_MODEL_ERR;
        }
        else if( McuType == ATMEL_AT89S52_MODEL )
        {
            if( CmdBuf[0] != 0x52 )
                return MCU_MODEL_ERR;
        }

        CmdBuf[0] =0X28;
        CmdBuf[1] =0x02;
        CmdBuf[2] =0x00;
        if( At89sISPoutin( Ch341index, 3, CmdBuf, 1, CmdBuf ) == FALSE )
            return DATA_TRANS_ERR;
        if( CmdBuf[0] != 0x06)
            return MCU_MODEL_ERR;
    }
    return NO_ERR;                              // The initialization is successful
}

/*==============================================================================

Function name: FindOneInNumber

What the function does: Find the number of 1s in a number

==============================================================================*/
UCHAR FindOneInNumber( ULONG Num )
{
UCHAR n;

    for( n=0; Num; n++ )
        Num &= Num-1;
    return n;
}

/*==============================================================================

Function name: DownPrgToMcu

What it does: Download the program to the MCU

==============================================================================*/
UCHAR WINAPI DownPrgToMcu( PVOID pBinBuf, ULONG FileLong )
{
UCHAR CmdBuf[CMD_BUF_LEN];  //  Command buffers 
UCHAR InBuf[BUF_LEN], OutBuf[BUF_LEN];
ULONG len;                  // Read  Take the data offset 

ULONG StopCount;            // The progress bar step length
ULONG cc;                  // Assisted stepping
ULONG BufLen;              // Buffer length

ULONG LineAdr;             //  Download FLASH'S LINE ADDRESS
UCHAR ErrorCount;           // Verification failed error counting
ULONG PageCount;

ULONG i, k, f;
ULONG index, offset;

// AT89S51
UCHAR FileDataBuf[ 258 ];       // command and file data buffers
UCHAR ReadDataBuf[ 256 ];       // Read returns a data buffer

    len = FileLong;
    SendMessage( HwndProg, PBM_GETRANGE,        //Get the range of the progress bar
        (WPARAM)TRUE,                           //TRUE INDICATES THAT THE RETURNED VALUE IS THE MINIMUM VALUE OF THE RANGE, AND FALSE INDICATES THAT THE MAXIMUM VALUE IS RETURNED
        (LPARAM)&DownProgRange );

    if( (McuType >AUTO_MCU_MODEL ) && (McuType < ATMEL_AT89S51_MODEL) )
    {
        if( McuType == ATMEL_MEGA8_MODEL )
        {
            if( len > 8192 )               // Maximum supported 8K
                return DATA_LEN_ERR;
            BufLen = 64;                        // MEGA8 Each page is 64bytes
        }
        else if( McuType == ATMEL_MEGA16_MODEL )
        {
            if( len > 16384 )              // Maximum supported 16K
                return DATA_LEN_ERR;
            BufLen = 128;                       // MEGA16 Each page is 128bytes
        }
        else if( McuType == ATMEL_MEGA32_MODEL )
        {
            if( len > 32768 )              // Maximum supported 32K
                return DATA_LEN_ERR;
            BufLen = 128;                       // MEGA32 Each page is 128bytes
        }
        else if( McuType == ATMEL_MEGA64_MODEL )
        {
            if( len > 65536 )              // Maximum supported 64K
                return DATA_LEN_ERR;
            BufLen = 256;                       // MEGA64 Each page is 256bytes
        }
        else if( McuType == ATMEL_MEGA128_MODEL )
        {
            if( len > 131072 )             // Maximum supported 128K
                return DATA_LEN_ERR;
            BufLen = 256;                       // MEGA128 Each page is 256bytes
        }
        else if( McuType == ATMEL_MEGA8515_MODEL )
        {
            if( len > 8192 )               // Maximum supported 8K
                return DATA_LEN_ERR;
            BufLen = 64;                        // MEGA16 Each page is 64bytes
        }
        else if( McuType == ATMEL_MEGA8535_MODEL )
        {
            if( len > 8192 )               // Maximum supported 8K
                return DATA_LEN_ERR;
            BufLen = 64;                        // MEGA16 Each page is 64bytes
        }
        else if( McuType == ATMEL_MEGA48_MODEL )
        {
            if( len > 4096 )               // Maximum supported 4K
                return DATA_LEN_ERR;
            BufLen = 64;                        // MEGA48 Each page is 64bytes
        }
        else if( McuType == ATMEL_MEGA88_MODEL )
        {
            if( len > 8192 )               // Maximum supported 4K
                return DATA_LEN_ERR;
            BufLen = 64;                        // MEGA88 Each page is 64bytes
        }
        else if( McuType == ATMEL_MEGA168_MODEL )
        {
            if( len > 16384 )              // Maximum supported 16K
                return DATA_LEN_ERR;
            BufLen = 128;                       // MEGA168 Each page is 128bytes
        }
        else
        {
            //MessageBox( NULL, "Error", "Prompt", MB_OK );
            return NO_SUPORT_ERR;
        }

        #if 1
        CmdBuf[0] = 0xAC;                       // Chip Erase
        CmdBuf[1] = 0x80;
        CmdBuf[2] = 0xFF;
        if( MegaSpiOutInData( Ch341index, 4, CmdBuf, FALSE ) == FALSE )
            return DATA_TRANS_ERR;
        #endif
        Sleep( 50 );                            // Minimal wait for chip to settle down 9ms

        if( len == 0 )
            return NO_ERR;                      // 0 returns success(Indicates the erase chip)

        PageCount = 0;
        offset = 0;
        cc = 0;
        ErrorCount = 0;
        StopCount = (len + 255) / 256;     // Take 256bytes as the standard for a single download
        while( 1 )
        {
            cc++;
            if( cc == (256 / BufLen) )
            {
                SendMessage( HwndProg, PBM_DELTAPOS,
                    (WPARAM)( MAX_PROG_LEN / StopCount), (LPARAM)0 );
                cc = 0;
            }

            if( len > BufLen )
                len -= BufLen;
            else
                len = 0;

            memcpy( OutBuf, &((PUCHAR)pBinBuf)[offset], BufLen );
            for( i=0; i!=BufLen; i++ )
            {
                 if( OutBuf[i] != 0xFF )
                     goto P_DOWN;
            }
            goto P_PASS;        // If it is all 0XFF, it will not be downloaded into the MCU

            P_DOWN:
            k = 0;
            f = i;                                              // Record the first location that is not 0XFF
            //  Determine the chip version number 
            if( !CH341SPIBitSet )
            {
                for( i=0; i!=(BufLen / 2); i++ )                // Load page data
                {
                    CmdBuf[0] = 0x40;                           // The low position of the word LSB bytes
                    CmdBuf[1] = 0x00;
                    CmdBuf[2] = (UCHAR)i;                       // Address under the load page
                    CmdBuf[3] = OutBuf[k++];                    // Loaded data
                    if( MegaSpiOutInData( Ch341index, 4, CmdBuf, CH341SPIBitSet ) == FALSE )
                        return DATA_TRANS_ERR;

                    CmdBuf[0] = 0x48;                           // The high bytes of the word
                    CmdBuf[1] = 0x00;
                    CmdBuf[2] = (UCHAR)i;                       // Address under the load page
                    CmdBuf[3] = OutBuf[k++];                    // Loaded data
                    if( MegaSpiOutInData( Ch341index, 4, CmdBuf, CH341SPIBitSet ) == FALSE )
                        return DATA_TRANS_ERR;
                }
            }
            else
            {
                index = 0;
                for( i=0; i!=(BufLen / 2); i++ )                // Load page data
                {
                    CmdBuf[ index++ ] = 0x40;                   // The low position of the word LSB bytes
                    CmdBuf[ index++ ] = 0x00;
                    CmdBuf[ index++ ] = (UCHAR)i;               // Address under the load page
                    CmdBuf[ index++ ] = OutBuf[k++];            // Loaded data

                    CmdBuf[ index++ ] = 0x48;                   // The high bytes of the word
                    CmdBuf[ index++ ] = 0x00;
                    CmdBuf[ index++ ] = (UCHAR)i;               // Address under the load page
                    CmdBuf[ index++ ] = OutBuf[k++];            // Loaded data
                }
                if( MegaSpiOutInData( Ch341index, index, CmdBuf, CH341SPIBitSet ) == FALSE )
                    return DATA_TRANS_ERR;
            }

            i = FindOneInNumber( BufLen/2 - 1);
            CmdBuf[0] = 0x4C;                               // Write data to the programspace page
            CmdBuf[1] = (UCHAR)(PageCount>> (8 -i) );       // Bytes in the high position
            CmdBuf[2] = (UCHAR)(PageCount<< i );            // Bytes in the lower bits
            if( MegaSpiOutInData( Ch341index, 4, CmdBuf, CH341SPIBitSet ) == FALSE )
                return DATA_TRANS_ERR;                      // The above writes page data
            
            // At least wait 4.5ms
            #if 1
            // Cancel the delay and add Read to get the judgment 2008-7-22
            LineAdr = PageCount * BufLen / 2 + f/2;
            i = 0;
            while( 1 )
            {
                if( f % 2 == 0 )
                    CmdBuf[0] = 0x20;                      // Read Take the low position bytes
                else
                    CmdBuf[0] = 0x28;

                CmdBuf[1] = (UCHAR)( LineAdr>>8 );         // High 8BIT ADDRESSES
                CmdBuf[2] = (UCHAR)LineAdr;                // LOW 8BIT ADDRESSES
                if( MegaSpiOutInData( Ch341index, 4, CmdBuf, CH341SPIBitSet ) == FALSE )
                    return DATA_TRANS_ERR;
                if( CmdBuf[3] != 0xff )
                    break;
                // Multiple failures A data transfer error is returned
                if( i++ >= 100 )
                    return DATA_CMP_ERR;     
            }
            #endif

            P_PASS:
            PageCount++;                                    // Go to the next page
            offset += BufLen;
            if( len == 0 )
                break;
            if( StopBit )
                return REQ_STOP_ERR;
        }

        cc = 0;
        PageCount = 0; 
        offset = 0;
        len = FileLong;
        SetDlgItemText( MainDialog, IDC_STATIC_PROMPT, _T("Begin Checking Download Data...") );
        SendMessage( HwndProg, PBM_SETPOS, (WPARAM)DownProgRange.iLow, (LPARAM)0 );
        while( 1 )
        {
            cc++;
            if( cc == (256 / BufLen) )
            {
                SendMessage( HwndProg, PBM_DELTAPOS,
                    (WPARAM)( MAX_PROG_LEN / StopCount), (LPARAM)0 );
                cc = 0;
            }

            if( len > BufLen )
                len -= BufLen;
            else
                len = 0;

            k = 0;
            LineAdr = PageCount * BufLen / 2;                   // Linear address
            if( !CH341SPIBitSet )
            {
                for( i=0; i!= (BufLen / 2); i++ )
                {
                    CmdBuf[0] = 0x20;                           // Read Take the low position bytes
                    CmdBuf[1] = (UCHAR)( LineAdr>>8 );          // High 8BIT ADDRESSES
                    CmdBuf[2] = (UCHAR)LineAdr;                 // LOW 8BIT ADDRESSES
                    if( MegaSpiOutInData( Ch341index, 4, CmdBuf, CH341SPIBitSet ) == FALSE )
                        return DATA_TRANS_ERR;
                    InBuf[k] = CmdBuf[3];
                    k++;

                    CmdBuf[0] = 0x28;                           // Read Take the high position bytes
                    CmdBuf[1] = (UCHAR)( (LineAdr>>8) );        // High 8BIT ADDRESSES
                    CmdBuf[2] = (UCHAR)LineAdr;                 // LOW 8BIT ADDRESSES
                    if( MegaSpiOutInData( Ch341index, 4, CmdBuf, CH341SPIBitSet ) == FALSE )
                        return DATA_TRANS_ERR;
                    InBuf[k] = CmdBuf[3];
                    k++;
                    LineAdr++;
                }

                for( i=0; i!=BufLen; i++ )
                {
                    if( ((PUCHAR)pBinBuf)[offset + i] != InBuf[i] )
                        return DATA_CMP_ERR;
                }
            }
            else
            {
                index = 0;
                for( i=0; i!= (BufLen / 2); i++ )
                {
                    CmdBuf[index++] = 0x20;                     // Read Take the low position bytes
                    CmdBuf[index++] = (UCHAR)( LineAdr>>8 );    // High 8BIT ADDRESSES
                    CmdBuf[index++] = (UCHAR)LineAdr;           // LOW 8BIT ADDRESSES
                    index++;

                    CmdBuf[index++] = 0x28;                     // Read Take the high position bytes
                    CmdBuf[index++] = (UCHAR)( (LineAdr>>8) );  // High 8BIT ADDRESSES
                    CmdBuf[index++] = (UCHAR)LineAdr;           // LOW 8BIT ADDRESSES
                    index++;

                    LineAdr++;
                }
                if( MegaSpiOutInData( Ch341index, index, CmdBuf, CH341SPIBitSet ) == FALSE )
                    return DATA_TRANS_ERR;

                for( i=0; i!=BufLen; i++ )
                {
                    if( ((PUCHAR)pBinBuf)[offset + i] != CmdBuf[ i*4 + 3 ] )
                        return DATA_CMP_ERR;                // Data comparison errors
                }
            }

            PageCount++;
            offset += BufLen;
            if( len == 0 )
                break;
            if( StopBit )
                return REQ_STOP_ERR;
        }
    }
    else
    {
        // Add AT89S52 picker and progress bar indication
        if( McuType == ATMEL_AT89S51_MODEL )
        {
            if( FileLong > 4096 )
                return DATA_LEN_ERR;
        }
        else if( McuType == ATMEL_AT89S52_MODEL )
        {
            if( FileLong > 8192 )
                return DATA_LEN_ERR;
        }

        CmdBuf[0] =0xac;                                    // Erase chip data
        CmdBuf[1] =0x80;
        if ( At89sISPoutput( Ch341index, 4, CmdBuf ) == FALSE )
        {
            return DATA_TRANS_ERR;
        }
        Sleep( 500 );                                       //  delay 500ms Wait for the command to complete 

        if( FileLong == 0 )
            return NO_ERR;                                  // 0 returns success

        PageCount = 0;
        len = FileLong;
        offset = 0;
        cc = 0;
        StopCount = (FileLong + 255) / 256;                 //Take it once Download 256bytes as standard 
        while( 1 )
        {
            cc++;
            if( cc == 1  )
            {
                SendMessage( HwndProg, PBM_DELTAPOS,
                    (WPARAM)( MAX_PROG_LEN / StopCount), (LPARAM)0 );
                cc = 0;
            }
            if( len > 256 )
                len -= 256;
            else
                len = 0;

            memcpy( &FileDataBuf[2], &((PUCHAR)pBinBuf)[offset], 256 );

            FileDataBuf[0] = 0x50;                          //  write 89S51MCU data 
            FileDataBuf[1] = (UCHAR)PageCount;              // Page Mode pages 
            if ( At89sISPoutput( Ch341index, 256+2, FileDataBuf ) == FALSE )
                return DATA_TRANS_ERR;

            offset += 256;
            if( len == 0 )
                break;
            if( StopBit )
                return REQ_STOP_ERR;
            PageCount++;
        }

        PageCount = 0;
        len = FileLong;
        cc = 0;
        offset = 0;
        StopCount = (FileLong + 255) / 256;
        SetDlgItemText( MainDialog, IDC_STATIC_PROMPT, _T("Being Download  data ...") );
        SendMessage( HwndProg, PBM_SETPOS, (WPARAM)DownProgRange.iLow, (LPARAM)0 );
        while( 1 )
        {
            cc++;
            if( cc == 1  )
            {
                SendMessage( HwndProg, PBM_DELTAPOS,
                    (WPARAM)( MAX_PROG_LEN / StopCount), (LPARAM)0 );
                cc = 0;
            }
            if( len > 256 )
                len -= 256;
            else
                len = 0;        

            FileDataBuf[0] = 0x30;                          // Read Program Memory page
            FileDataBuf[1] = (UCHAR)PageCount;
            if ( At89sISPoutin( Ch341index, 2, FileDataBuf, 256, ReadDataBuf ) == FALSE )                                          // Read  ß∞‹
                return DATA_TRANS_ERR;

            for( i = 0; i != 255; i++ )                     // verify write  data 
            {
                if( ((PUCHAR)pBinBuf)[ offset + i ] != ReadDataBuf[ i ] )
                    return DATA_CMP_ERR;
            }

            offset += 256;
            if( len == 0 )
                break;
            if( StopBit )
                return REQ_STOP_ERR;
            PageCount++;
        }
    }
    SendMessage( HwndProg, PBM_DELTAPOS, (WPARAM)(MAX_PROG_LEN - 1), (LPARAM)0 );
    return NO_ERR;                                          //  data  Download  finish 
}

/*==============================================================================

Function name: DownDataToMcu

Function: Download data to EEPROM (MEGA)

==============================================================================*/
UCHAR WINAPI DownDataToMcu( PVOID pDataBuf, ULONG FileLong )
{
USHORT i;
UCHAR tem8;
USHORT cc;                  // Assisted stepping
ULONG StopCount;
UCHAR CmdBuf[32];           //  Command buffers 

    SendMessage( HwndProg, PBM_GETRANGE,    // Get the range of the progress bar
        (WPARAM)TRUE,                       // TRUE INDICATES THAT THE RETURNED VALUE IS THE MINIMUM VALUE OF THE RANGE, AND FALSE INDICATES THAT THE MAXIMUM VALUE IS RETURNED
        (LPARAM)&DownProgRange );

    if( McuType == ATMEL_MEGA8_MODEL )
    {
        if( FileLong > 512 )
            return DATA_LEN_ERR;
    }
    else if( McuType == ATMEL_MEGA16_MODEL )
    {
        if( FileLong > 512 )
            return DATA_LEN_ERR;
    }
    else if( McuType == ATMEL_MEGA32_MODEL )
    {
        if( FileLong > 1024 )
            return DATA_LEN_ERR;
    }
    else if( McuType == ATMEL_MEGA64_MODEL )
    {
        if( FileLong > 2048 )
            return DATA_LEN_ERR;
    }
    else if( McuType == ATMEL_MEGA128_MODEL )
    {
        if( FileLong > 4096 )
            return DATA_LEN_ERR;
    }
    else if( McuType == ATMEL_MEGA8515_MODEL )
    {
        if( FileLong > 512 )
            return DATA_LEN_ERR;
    }
    else if( McuType == ATMEL_MEGA8535_MODEL )
    {
        if( FileLong > 512 )
            return DATA_LEN_ERR;
    }
    else if( McuType == ATMEL_MEGA48_MODEL )
    {
        if( FileLong > 256 )
            return DATA_LEN_ERR;
    }
    else if( McuType == ATMEL_MEGA88_MODEL )
    {
        if( FileLong > 512 )
            return DATA_LEN_ERR;
    }
    else if( McuType == ATMEL_MEGA168_MODEL )
    {
        if( FileLong > 512 )
            return DATA_LEN_ERR;
    }
    else
    {
        //MessageBox( NULL, "Error", "Prompt", MB_OK );
        return NO_SUPORT_ERR;
    }

    cc = 0;
    StopCount = ( FileLong + 15 ) / 16;       // 16bytes plus one more step
    for( i=0; i!=FileLong; i++)
    {
        cc++;
        if( cc == 16 )
        {
            SendMessage( HwndProg, PBM_DELTAPOS,
                (WPARAM)(MAX_PROG_LEN / StopCount), (LPARAM)0 );
            cc = 0;
        }

        CmdBuf[0] = 0xC0;                   // The low position of the word LSB bytes
        CmdBuf[1] = (UCHAR)(i>>8);
        CmdBuf[2] = (UCHAR)i;               // Address under the load page
        CmdBuf[3] = ((PUCHAR)pDataBuf)[i];  // Loaded data
        tem8 = CmdBuf[3];
        if( MegaSpiOutInData( Ch341index, 4, CmdBuf, CH341SPIBitSet ) == FALSE )
            return DATA_TRANS_ERR;
        Sleep( 20 );                        //  min wait time 9ms

        if( StopBit )
            return REQ_STOP_ERR;
    }
    
    cc = 0;
    SetDlgItemText( MainDialog, IDC_STATIC_PROMPT, _T("Being Download  data ...") );
    SendMessage( HwndProg, PBM_SETPOS, (WPARAM)DownProgRange.iLow, (LPARAM)0 );
    for( i=0; i!=FileLong; i++)
    {
        cc++;
        if( cc == 16 )
        {
            SendMessage( HwndProg, PBM_DELTAPOS,
                (WPARAM)(MAX_PROG_LEN / StopCount), (LPARAM)0 );
            cc = 0;
        }

        CmdBuf[0] = 0xA0;                   // The low position of the word LSB bytes
        CmdBuf[1] = (UCHAR)(i>>8);
        CmdBuf[2] = (UCHAR)i;               // Address under the load page
        CmdBuf[3] = ((PUCHAR)pDataBuf)[i];  // Loaded data
        if( MegaSpiOutInData( Ch341index, 4, CmdBuf, CH341SPIBitSet ) == FALSE )
            return DATA_TRANS_ERR;
        if( ((PUCHAR)pDataBuf)[i] != CmdBuf[3] )
            return DATA_CMP_ERR;            // Data comparison errors

        if( StopBit )
            return REQ_STOP_ERR;
    }
    SendMessage( HwndProg, PBM_DELTAPOS, (WPARAM)(MAX_PROG_LEN - 1), (LPARAM)0 );
    return NO_ERR;
}
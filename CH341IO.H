#ifndef     _CH341IO_H
#define     _CH341IO_H

#define     BUF_LEN             256
#define     CMD_BUF_LEN         ((BUF_LEN / 2) * 8)
#define     MAX_BUF_LEN         ( 2 * mCH341_PACKET_LENGTH * CMD_BUF_LEN )

#define SET_CH341_D1 0X02 // Set D1 Pin Output High (for at89s)

extern ULONG Ch341index;                //Share the serial number of the CH341
extern LONG McuType;                   // Shared MCU types
extern HWND HwndProg;                   //Share progress bar handles
extern PBRANGE DownProgRange;          // Share the progress bar range

#define MAX_PROG_LEN 	1024 // Maximum progress bar length

//Pin settings
#define	 	SET_SPI_RST 	0X01 // The reset foot is connected to D1
#define	 	SET_SPI_CLK 	0X08 // The clock is connected to the D3 pin
#define 	SET_SPI_MOSI 	0X20 // The data output pin is connected to D5
#define		SET_SPI_MOSO 	0X80 // The data input pin is connected to D7


//Initialize the MCU programming return
#define 	NO_ERR 	0X00 // Returns correctly
#define 	DATA_TRANS_ERR 	0X01 // Data transfer error
#define 	NOT_PROG_ERR 	0X02 // Non-programmable error
#define 	MCU_MODEL_ERR 	0X03 // Wrong MCU model selection
#define 	REQ_STOP_ERR 	0X04 // Request stop error
#define 	NO_SUPORT_ERR 	0X05 // This MCU model is not supported

// Define the return of downloaded data
#define 	DATA_LEN_ERR 	0X10 // Wrong data length
#define 	DATA_CMP_ERR 	0X11 // Data validation error
#define 	NO_MEM_ERR 		0X12 // Memory allocation failed
#define 	NO_MCU_ERR 		0X13 // Unrecognized MCU

//Define the number of MCUs
#define MCU_COUNT 0X0D // Total number of MCUs

//Define the MCU type
#define     AUTO_MCU_MODEL          0X00    // The MCU model is automatically recognized
#define     ATMEL_MEGA8_MODEL       0x01    // MEGA8
#define     ATMEL_MEGA16_MODEL      0x02    // MEGA16
#define     ATMEL_MEGA32_MODEL      0x03    // MEGA32
#define     ATMEL_MEGA64_MODEL      0x04    // MEGA64
#define     ATMEL_MEGA128_MODEL     0x05    // MEGA128
#define     ATMEL_MEGA8515_MODEL    0x06    // MEGA8515
#define     ATMEL_MEGA8535_MODEL    0x07    // MEGA8535
#define     ATMEL_MEGA48_MODEL      0x08    // MEGA48
#define     ATMEL_MEGA88_MODEL      0x09    // MEGA88
#define     ATMEL_MEGA168_MODEL     0X0A    // MEGA168
#define     ATMEL_AT89S51_MODEL     0X0B    // AT89S51
#define     ATMEL_AT89S52_MODEL     0X0C    // AT89S52

// Externally provided function calls
BOOL    WINAPI  SpiEnable(
    ULONG       index );                // Specify the serial number of the CH341 device

BOOL    WINAPI  At89sISPoutput(
    ULONG       index,                  // Specify the serial number of the CH341 device
    ULONG   OutLen,                     // The length of the output data
    LPCSTR  OutBuf );                   // Output data buffers

BOOL    WINAPI  At89sISPoutin(
    ULONG           index,              // Specify the serial number of the CH341 device
    ULONG   OutLen,                     // The length of the output data
    LPCSTR  OutBuf,                     // Output data buffers
    ULONG   InLen,                      // Prepare to read the data length
    PUCHAR  InBuf );                    // Input data buffers

BOOL    WINAPI  MegaSpiOutInData(
    ULONG   index,                      // Specify the serial number of the CH341 device
    ULONG   OutLen,                     // The length of the output data
    PVOID   DataBuf,                    // The output data buffer is shared with the SPI data return
    BOOL    SpiBit );

UCHAR WINAPI IniMcuDown( void );        // Initialize the MCU download work
UCHAR WINAPI DownPrgToMcu(
    PVOID pBinBuf,                      // BIN file start buffer
    ULONG FileLong );                   // The length of the BIN file

UCHAR WINAPI DownDataToMcu(
    PVOID pDataBuf,                     // The start address of the MEGA download data (EEPROM).
    ULONG FileLong );                   // Effective length
#endif
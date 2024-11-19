//Coded Date: 2007-6
//Program function: Use CH341 to program the MCU with simulated SPI timing
//Compilation Environment: VC6.0

#ifndef __CH341DP_H__
#define __CH341DP_H__

#define     APPNAME             "CH341DP"
#define     APPVER              "V1.3"            

// Maximum file length
#define     MAX_FILE_LEN        (0X100000)

#define     CH341DP_SETTINGS_KEY   TEXT("Software\\WinChipHead\\ch341dp")

//2007-10-30 Fixed that the handle was not closed after creating a thread
//2009-1-5 Rewritten to ANSI code can be used under 98

#define 	CMD_NORMAL 	0 // No command input, the program starts normally
#define 	CMD_START 	1 // Start the program as a command
#define 	CMD_ERROR 	2 // The entered command is incorrect

#define 	WRITE_FLASH_CMD 	'WF' // PROGRAM THE FLASH OF THE MEGA AND AT89S
#define 	WRITE_EEPROM_CMD 	'WE' // PROGRAM MEGA'S FLASH
#define 	RESET_MCU_CMD 		'RM' // Reset MCU

//Calculate the array size
#define SIZE(Array) (sizeof(Array) / sizeof(Array[0]))

//The bit operation defines the macro as follows:
#define SET_DWORD_BIT( DW, N ) ((DW) |= (0x00000001<<(N))) // SET THE NTH DIGIT OF WORD TO 1
#define CLR_DWORD_BIT( DW, N ) ((DW) &=~(0x00000001<<(N))) // SET THE NTH DIGIT OF WORD TO 0
#define CPL_DWORD_BIT( DW, N ) ((DW) ^= (0x00000001<<(N))) // SET THE NTH DIGIT OF WORD TO BE NEGATED
#define GET_DWORD_BIT( DW, N ) (((DW) & (0x00000001<<(N)))==0?0:1) // GET THE NTH DIGIT OF WORD

//Declare the interface function externally
BOOL HexToBin( PVOID Hexbuf, ULONG iHexBufLen, PVOID Binbuf, PULONG iBinLen );

BOOL WINAPI SpiRunMega( ULONG   index );
BOOL WINAPI SpiRunAt89s( ULONG   index );

int MessageBoxCenter(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption,UINT uType );

extern const PTCHAR McuName[ ];          // Share the MCU name

extern ULONG       CH341ChipVer;

extern HWND        MainDialog;

extern BOOL        StopBit;

extern BOOL        CH341SPIBit; 

extern BOOL        CH341SPIBitSet;      // The SPI flag is supported for backup

//#define     CH341DP_HELP_STR        L"1.显示帮助\nCH341DP /? \n\n2.下载文件\nCH341DP %1 %2 %3\n参数1: 参考下面复位的参数1\n参数2: WF 对FLASH编程 WE 对EEPROM编程\n参数3: 下载文件路径\n例: CH341DP MEGA8  WF \"D:\\down\\down.hex\"\n以上实现把down.hex下载到MEGA8的FLASH\n\n3.复位\nCH341DP %1 %2\n参数1:可能取值如下:\nMEGA , MEGA8, MEGA16, MEGA32, MEGA64, MEGA128, MEGA8515,\nMEGA8535, MEGA48 MEGA88,MEGA168, AT89S51,AT89S52\n参数2: RM\n例: CH341DP AT89S51 RM\n以上实现对AT89S51的复位"
//#define 		CH341DP_HELP_STR 		L"1. Show Help\nCH341DP/? \n\n2.Download file\nCH341DP %1 %2 %3\nParameter 1: Refer to the following reset parameter 1\nParameter 2: WF Programming for FLASH WE Programming EEPROM\nParameter 3: Download file path\nExample: CH341DP MEGA8 WF "D:downdown.hex"\nThe above implementation downloads down.hex to MEGA8 FLASH\n\n3.Reset\nCH341DP % 1 %2\nParameter 1: The possible values are as follows:nMEGA , MEGA8, MEGA16, MEGA32, MEGA64, MEGA128, MEGA8515,\nMEGA8535, MEGA48 MEGA88,MEGA168, AT89S51,AT89S52\nParameter 2: RMnExample: CH341DP AT89S51 RM\nReset the AT89S51 above"
#endif

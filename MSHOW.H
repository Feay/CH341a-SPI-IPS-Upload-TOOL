//编码时间: 2007-5-19 2007-5-25 2007-6-3 2007-9-4 2007-9-19
//Program function: Debug function for printing memory data
//Compilation Environment: VC6.0

#ifndef __MSHOW_H__
#define __MSHOW_H__

// Calculate the array size
#define SIZE(Array) (sizeof(Array) / sizeof(Array[0]))

void ShowProgBulid( void );
ULONG GetLastResult( void *p );
void ShowLastResult( void );
void PrintStrA( PVOID p, BOOL en );
void PrintArray( PVOID p, ULONG length, BOOL en );
void PrintStrToFile( PVOID p, BOOL en, LPCSTR FileName );
void PrintArryToFile( PVOID p, ULONG length, BOOL en, LPCSTR FileName );
void MessageStr(void *p );
void MessageArry( PVOID p, ULONG length,  LPCSTR lpCaption);
void SendArryToNotepad( PVOID p, ULONG length, BOOL en );
void SendStrToNotepad( PVOID p, BOOL en );


/*/ 便于程序键入
#define GERR    GetLastResult       // 得到错误提示消息
#define SERR    ShowLastResult      // 显示API函数完成后的错误
#define PSTR    PrintStrA           // 在控制台中输出字符串
#define PHEX    PrintArray          // 在控制台中输出HEX
#define FSTR    PrintStrToFile      // 在指定的文件中输出字符串
#define FHEX    PrintArryToFile     // 在指定的文件中输出HEX
#define MSTR    MessageStr          // 显示字符串提示对话框
#define MHEX    MessageArry         // 在对话框中输出HEX
#define SSTR    SendStrToNotepad    // 发送字符串到记事本(发送频率不能太快!!)
#define SHEX    SendArryToNotepad   // 发送HEX到记事本
*/
// Easy to type in the program
#define GERR	GetLastResult // get an error message
#define SERR	ShowLastResult // Shows the error when the API function completes
#define PSTR	PrintStrA // Output string in the console
#define PHEX	PrintArray // Output HEX in the console
#define FSTR	PrintStrToFile // Outputs a string in the specified file
#define FHEX	PrintArryToFile // Outputs HEX in the specified file
#define MSTR	MessageStr // Displays a string hint dialog box
#define MHEX	MessageArry // Output HEX in the dialog box
#define SSTR	SendStrToNotepad // Send string to Notepad (can't send too often!!)
#define SHEX	SendArryToNotepad // Send HEX to Notepad
/*
#define     MSHOW_TITLE         "CH341DP"           // 定义弹出对话框标题
#define     SAVE_FILE_STR       "MSHOW.TXT"         // 定义输出文件名
#define     SEND_CLASS_NAME     "Notepad"           // 查找类为记事本
*/
#define 	MSHOW_TITLE 		"CH341DP" // Define the pop-up dialog title
#define 	SAVE_FILE_STR 		"MSHOW.TXT" // Define the output file name
#define	 	SEND_CLASS_NAME 	"Notepad" // Find class as Notepad

#endif

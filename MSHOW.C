//#define     UNICODE             // 是否启用UNICOUDE编码

#ifdef      UNICODE
#define     _UNICODE
#endif

#include    <stdio.h>
#include    <tchar.h>
#include    <windows.h>
#include    <WINERROR.H>
#include	<locale.h>
#include    "MSHOW.H"


/*==============================================================================

Function Name: ShowProgBulid

Function: Displays the program compilation time

==============================================================================*/
void ShowProgBulid( void )
{
TCHAR DateStr[] = _T(__DATE__);
TCHAR TimeStr[] = _T(__TIME__);

    PSTR( DateStr, 0 );
    PSTR( " ", 0 );
    PSTR( TimeStr, 1 );
}

/*==============================================================================

Function Name: GetLastResult

Function: Put the error message in the buffer that the pointer points to and return the error code

==============================================================================*/
ULONG GetLastResult( void *p )
{
ULONG LastResult;
TCHAR szSysMsg[128];

    LastResult = GetLastError();
    FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, 0, LastResult,
        MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), szSysMsg, sizeof(szSysMsg), 0);

    __try
    {
        _stprintf( p, _T("Error code = 0x%08x: %s"), LastResult, szSysMsg );
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        return 0xFFFFFFFF;
    }
    return LastResult;
}

/*==============================================================================

Function Name: ShowLastResult

Function: Displays the last run error

==============================================================================*/
void ShowLastResult( void )
{
ULONG LastResult = 0;
TCHAR szSysMsg[128], tem[128];

    LastResult = GetLastError();
    FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, 0, LastResult,
        MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), szSysMsg, sizeof(szSysMsg), 0);

    _stprintf( tem, _T("Err code = 0x%08x: %s"), LastResult, szSysMsg );
    MessageBox( NULL, tem, _T( MSHOW_TITLE ), MB_OK );
}

/*==============================================================================

Function Name: PrintStrA

Function: Print ANSI-encoded strings in the console, and wrap lines if en is not 0

==============================================================================*/
void PrintStrA( PVOID p, BOOL en )
{
	__try                                           // Prevent exceptions from occurring (return of an exception function once)
	{
		_tprintf( p );
	}

	__except( EXCEPTION_EXECUTE_HANDLER )           // An anomaly has occurred
	{
		UINT code = GetExceptionCode( );
		_tprintf( _T("\nException Code = 0X%08X\n"), code );
		return;
	}

    if( en )                                            // Add a carriage return
    {
		_tprintf( _T("\n") );
    }
}

/*==============================================================================

Function Name: PrintArray

Function: Print continuous data in memory space in the console

(*p is the start address, length is the number of bytes sent, and en is not 0 if it is a line break)

==============================================================================*/
void PrintArray( PVOID p, ULONG length, BOOL en )
{
PUCHAR temp;

    temp = p;
    while( length )
    {
        __try
        {
            printf( "%02X", *temp );
        }

        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            UINT code = GetExceptionCode( );
            printf( "\nException Code = 0X%08X\n", code );
            return;
        }
        putchar( 32 );                                  // Add spaces
        temp++;
        --length;
    }

    if( en )
    {
        putchar( 13 );
        putchar( 10 );
    }
}

/*==============================================================================

Function Name: PrintStrToFile

Function: Print the string (ANSI encoded) to the file

*p is the address where the memory starts, en is whether to wrap at the end of the string, and *FileName is the file where the data is stored

==============================================================================*/
void PrintStrToFile( PVOID p, BOOL en, LPCSTR FileName )
{
HANDLE FileHandle;
ULONG WriteCout;
UCHAR enbuf[ ] ={13,10};        // enter
UCHAR exbuf[30];                // Abnormal characters are stored in a buffer

    FileHandle = CreateFileA( FileName, GENERIC_WRITE, FILE_SHARE_DELETE, NULL,
            OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );
    if( FileHandle == INVALID_HANDLE_VALUE )        // Failed to create a file
    {
        ShowLastResult( );
        return;
    }
    SetFilePointer( FileHandle, 0, 0, FILE_END );   // Move the file pointer to the end of the file
    __try                                           // Prevent exceptions from occurring when accessing memory
    {
        WriteFile( FileHandle, (LPCVOID)p, strlen( ( LPCSTR )p ), (LPDWORD)&WriteCout, NULL );
        if( WriteCout != strlen( ( LPCSTR )p ) )
        {
            ShowLastResult();                       // Write file error
            return;
        }
    }

    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        UINT code = GetExceptionCode( );
        sprintf( exbuf, "Exception Code = 0X%08X", code );
        WriteFile( FileHandle, (LPCVOID)exbuf,
            strlen( exbuf ), (LPDWORD)&WriteCout, NULL );
        WriteFile( FileHandle, (LPCVOID)enbuf, 2,
            (LPDWORD)&WriteCout, NULL );
        return;
    }

    if( en )
    {                                               // Add a carriage return at the end of the string
        WriteFile( FileHandle, (LPCVOID)enbuf, 2, (LPDWORD)&WriteCout, NULL );
    }
    CloseHandle( FileHandle );                      // Close the file
}

/*==============================================================================

Function Name: PrintArrayToFile

Function: Prints memory space data to a file

*p is the memory start address, length is the print length, en is whether to wrap at the end of the string, and *FileName is the data storage file

==============================================================================*/
void PrintArryToFile( PVOID p, ULONG length, BOOL en, LPCSTR FileName )
{
HANDLE FileHandle;
USHORT i;
ULONG WriteCout, temp32;
PVOID p1, p2;
UCHAR enbuf[ ] ={13,10};                            // Enter the storage buffer
UCHAR exbuf[30];                                    // 异常字符存放缓冲区

    FileHandle = CreateFileA( FileName, GENERIC_WRITE, FILE_SHARE_DELETE, NULL,
        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );
    SetFilePointer( FileHandle, 0, 0, FILE_END );   // 移动文件指针到文件末尾

    temp32 = length*2 + length +2;                  // 加空格和回车
    p1 = malloc( temp32 );
    if( p1 == NULL)                                 // 内存分配失败
        return;

    p2 = p1;                                        // 保存内存分配开始地址
    for( i = 0; i != length; i++ )
    {
        __try                                               // 防止访问内存发生异常
        {
            sprintf( (PUCHAR)p2, "%02X ", *(PUCHAR)p );   // 实现字符转换
        }

        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            UINT code = GetExceptionCode( );
            sprintf( exbuf, "Exception Code = 0X%08X", code );
            WriteFile( FileHandle, (LPCVOID)exbuf,
                strlen( exbuf ), (LPDWORD)&WriteCout, NULL );
            WriteFile( FileHandle, (LPCVOID)enbuf, 2,
                (LPDWORD)&WriteCout, NULL );
            return;
        }

        ( (PUCHAR)p )++;
        ( (PUCHAR)p2 ) += 3;
    }

    WriteFile( FileHandle, (LPCVOID)p1, (temp32 -2), (LPDWORD)&WriteCout, NULL );
    if( WriteCout != (temp32 -2) )
    {
            ShowLastResult();                       // 写文件错误
    }

    if( en )
    {                                               // 在字符串末尾添加回车
        WriteFile( FileHandle, (LPCVOID)enbuf, 2, (LPDWORD)&WriteCout, NULL );
    }
    CloseHandle( FileHandle );
    free( p1 );
}

/*==============================================================================

Function Name: MessageStrA

Function: Displays the string prompt dialog box

==============================================================================*/
void MessageStr(void *p )
{
   MessageBox( NULL, (TCHAR *)p, _T(MSHOW_TITLE), MB_OK);
}

/*==============================================================================

Function Name: MessageArry

Function: The dialog prompts for memory data (*p is the start address, length is the number of bytes sent, and lpCaption is the display title)

==============================================================================*/
void MessageArry( PVOID p, ULONG length,  LPCSTR lpCaption )
{
PVOID tp1;  //临时指针
PVOID tp2;
ULONG i;

    tp1 = malloc( length * 2 + length + 1); // ANSI编码,一个字节转换为2个字符加空格加结束码
    if( tp1 == NULL )
        return;

    tp2 = tp1;
    for( i = 0; i !=length; i++ )
    {
        __try
        {
            sprintf( tp2, "%02X ", *( PUCHAR )p );
        }

        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            UINT code = GetExceptionCode( );
            printf( "\nException Code = 0X%08X\n", code );
            return;
        }
        ( (PUCHAR)p )++;
        (PUCHAR)tp2 += 3;                  // 两个字符加一个空格
    }
    *( (PUCHAR)tp2 ) = 0;
    MessageBoxA( NULL, (LPCSTR)tp1, lpCaption, MB_OK );
    free( tp1 );                            // 释放内存
}

/*==============================================================================

Function Name: SendStrToNotepad

Function: Send string to Notepad (*p is the start address, en is not 0 or new)

==============================================================================*/
void SendStrToNotepad( PVOID p, BOOL en )
{
HWND oldhwnd = NULL;
HWND Notehwnd;
POINT Point;
UCHAR exbuf[30];        // 异常字符存放缓冲区
BOOL ExSign;

    ExSign = FALSE;     // 没有发生异常
    Point.x = 20;
    Point.y = 20;
    Notehwnd = FindWindowA( SEND_CLASS_NAME, NULL );
    if( Notehwnd)
    {
        oldhwnd = ChildWindowFromPoint( Notehwnd, Point );
        if( !oldhwnd )
        {
            SERR();                         //显示错误信息
            return;
        }

        while( 1 )
        {
            __try
            {
                if( *(PUCHAR)p == 0 )
                    break;
                PostMessage( oldhwnd, WM_CHAR, (UINT)(*( UCHAR *)p ), 1 );
            }

            __except( EXCEPTION_EXECUTE_HANDLER )
            {
                UINT code = GetExceptionCode( );
                sprintf( exbuf, "Exception Code = 0X%08X", code );
                p = exbuf;
                continue;
            }
            ( (PUCHAR)p )++;
        }

        if( en )
        {
            PostMessage( oldhwnd, WM_CHAR, (UINT)'\n', 1 );         // 加入回车
        }
    }
    else
    {
        MessageBoxA( NULL," 请打开记事本", MSHOW_TITLE, MB_OK );
    }
}

/*==============================================================================

Function Name: SendArryToNotepad

Function: Send hexadecimal numbers to notepad (*p is the start address, length is the number of bytes sent, and en is not 0 to wrap)

It is important to note that the sending frequency should not be too fast!!, and you should not operate on Notepad while sending

==============================================================================*/
void SendArryToNotepad( PVOID p, ULONG length, BOOL en )
{
HWND oldhwnd = NULL;
HWND Notehwnd;
POINT Point;
UCHAR Buf[3];
UCHAR exbuf[30];        // 异常字符存放缓冲区
BOOL ExSign;

    ExSign = FALSE;     // 没有发生异常
    Point.x = 20;
    Point.y = 20;
    Notehwnd = FindWindowA( SEND_CLASS_NAME, NULL );
    if( Notehwnd)
    {
        oldhwnd = ChildWindowFromPoint( Notehwnd, Point );
        if( !oldhwnd )
        {
            SERR();                         //显示错误信息
            return;
        }

        while( length )
        {
            __try
            {
                sprintf( Buf, "%02X ", *(UCHAR *)p );
            }
            __except( EXCEPTION_EXECUTE_HANDLER )
            {
                UINT code = GetExceptionCode( );
                sprintf( exbuf, "Exception Code = 0X%08X", code );
                p = exbuf;
                ExSign = TRUE;
                length = ( strlen( p ) + 1) / 2;                    // 得到打印异常字符串长度
            }
            if( ExSign )
            {
                Buf[0] = *((UCHAR *)p)++;
                Buf[1] = *( UCHAR *)p;
            }
            PostMessage( oldhwnd, WM_CHAR, (UINT)Buf[0], 1 );
            PostMessage( oldhwnd, WM_CHAR, (UINT)Buf[1], 1 );
            if( !ExSign )
            {
                PostMessage( oldhwnd, WM_CHAR, (UINT)0x20, 1 );     // 加入空格
            }
            ( (UCHAR *)p )++;
            length--;
        }
        if( en )
        {
            PostMessage( oldhwnd, WM_CHAR, (UINT)'\n', 1 );         // 加入回车
        }
    }
    else
    {
        MessageBoxA( NULL," 请打开记事本", MSHOW_TITLE, MB_OK );
    }
}
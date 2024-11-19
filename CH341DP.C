// 程序版本: V1.3
// 编码时间: 2008-5-23  2009-1-12
// 程序功能:
// 编译环境: VC6.0
// 程序设计: rgw@wch.cn
//#define     UNICODE
//#ifdef      UNICODE
//#define     _UNICODE
//#endif
#include    <windows.h>
#include    <tchar.h>
#include    <stdio.h>
#include    <string.h>
#include    <locale.h>
#include    <stdlib.h>
#include    <commctrl.h>                // 提供进度条控件函数
#include    "CH341DP.H"
#include    "resource.h"                // 资源字体采用UNICODE编码
#include    "CH341DLL.H"                // CH341的动态链接库
#include    "CH341IO.H"                 // 调用接口函数
#include    "MSHOW.H"                   // 内存数据读取函数(用于调试)

#pragma comment(lib, "comctl32")

// 程序使用全局变量
HINSTANCE   DownExeHins;                        // 程序实例
HWND        MainDialog;                         // 主对话框句柄
TCHAR       DownFileBuf[260];                   // 存放Download文件路径缓冲区
HANDLE      Downhandle;                         // Download线程句柄
HWND        HwndProg;                           // Download进度条句柄
PBRANGE     DownProgRange;                      // Download进度条的范围

BOOL        Ch341State;                         // TURE表示设备是好的
HANDLE      Ch341Handle;                        // CH341的句柄
ULONG       Ch341index;                         // 打开CH341的序列号
LONG        McuType;                            // MCU类型全局变量
BOOL        IsEepromData;                       // TURE是EEPROM数据默认是程序

BOOL        CmdMode;                            // 命令模式
USHORT      McuCmd;                             // 存放命令字
ULONG       CH341ChipVer;                       // 芯片版本号

BOOL        StopBit;                            // 停止标志位
BOOL        CH341SPIBitSet;                     // 备份支持SPI标志位

static      BOOL        CH341SPIBit;            // 是否支持SPI
static      HMENU       hSysMenu;               // 系统菜单

static      UCHAR       pBinBuf[MAX_FILE_LEN];  // bin数据存放缓冲区
static      UCHAR       pFileBuf[MAX_FILE_LEN]; // 文件数据存放缓冲区
static      BOOL        FileOpenBit;            // 文件打开标志位

// 显示CMD帮助字符串
PTCHAR     CH341DP_HELP_STR = _T("1.显示帮助\nCH341DP /? \n\n2.Download文件\nCH341DP %1 \
%2 %3\n参数1: 参考下面复位的参数1\n参数2: WF 对FLASH编程 WE 对EEPROM编程\n参数3: \
Download文件路径\n例: CH341DP MEGA8  WF \"D:\\down\\down.hex\"\n以上实现把down.hexDownload\
到MEGA8的FLASH\n\n3.复位\nCH341DP %1 %2\n参数1:可能取值如下:\nMEGA , MEGA8, MEGA16,\
 MEGA32, MEGA64, MEGA128, MEGA8515,\nMEGA8535, MEGA48 MEGA88,MEGA168, AT89S51,AT89S52\
\n其 MEGA 为自动识别所支持的MEGA系列MCU\n参数2: RM\n例: CH341DP AT89S51 RM\n以上实现对\
AT89S51的复位");


/*==============================================================================

函数名: SetSpiSysMenu

函数作用: 是否支持SPI

==============================================================================*/
void SetSpiSysMenu( void )
{
ULONG i;

    // 默认不支持SPI4
    i = MF_UNCHECKED;
    CheckMenuItem( hSysMenu, IDC_SET_SPI, i );

    if( CH341SPIBit )
        i = MFS_ENABLED;
    else
        i = MFS_GRAYED;
    EnableMenuItem( hSysMenu, IDC_SET_SPI, i );
}

/*==============================================================================

函数名: CheckSysMenu

函数作用:

==============================================================================*/
void CheckSysMenu( void )
{
ULONG i;

    if( CH341SPIBitSet )
        i = MF_CHECKED;
    else
        i = MF_UNCHECKED;
    CheckMenuItem( hSysMenu, IDC_SET_SPI, i );
}

/*==============================================================================

函数名: SetSpiRegConfig

函数作用: 写SPI配置函数

==============================================================================*/
void SetSpiRegConfig( void )
{
HKEY    hKey;

    RegCreateKey( HKEY_CURRENT_USER, CH341DP_SETTINGS_KEY,	&hKey );
    RegSetValueEx( hKey, TEXT("SPIBit"), 0, REG_DWORD, (CONST BYTE *)&CH341SPIBitSet,
        sizeof(CH341SPIBitSet) );
    RegCloseKey( hKey );
}

/*==============================================================================

函数名: ReadSpiRegConfig

函数作用: 读SPI配置函数

==============================================================================*/
void ReadSpiRegConfig( void )
{
HKEY    hKey;
ULONG   ParamSize, type, i;

    RegCreateKey( HKEY_CURRENT_USER, CH341DP_SETTINGS_KEY,	&hKey );

    ParamSize = sizeof( CH341SPIBitSet );
    type = REG_DWORD;
    i = RegQueryValueEx( hKey, TEXT("SPIBit"), NULL, &type,
        (LPBYTE)&CH341SPIBitSet, &ParamSize );
    if( i==0x02 )
    {   //没有存在,选择1
        CH341SPIBitSet = 1;
    }
    CheckSysMenu( );
    RegCloseKey( hKey );
}

/*==============================================================================

函数名: SetMcuIndexReg

函数作用: 写配置索引

==============================================================================*/
void SetMcuIndexReg( ULONG index )
{
HKEY    hKey;

    RegCreateKey( HKEY_CURRENT_USER, CH341DP_SETTINGS_KEY,	&hKey );
    RegSetValueEx( hKey, TEXT("IniMcuIndex"), 0, REG_DWORD, (CONST BYTE *)&index,
        sizeof( index ) );
    RegCloseKey( hKey );
}

/*==============================================================================

函数名: ReadMcuIndexReg

函数作用: 读配置索引

==============================================================================*/
ULONG ReadMcuIndexReg( void )
{
HKEY    hKey;
ULONG   ParamSize, type, i, IniMcuIndex;

    RegCreateKey( HKEY_CURRENT_USER, CH341DP_SETTINGS_KEY,	&hKey );

    ParamSize = sizeof( IniMcuIndex );
    type = REG_DWORD;
    i = RegQueryValueEx( hKey, TEXT("IniMcuIndex"), NULL, &type,
        (LPBYTE)&IniMcuIndex, &ParamSize );
    if( i==0x02 )
    {   //没有存在,则设为0
        IniMcuIndex = 0;
    }
    RegCloseKey( hKey );
    return IniMcuIndex;
}

/*==============================================================================

函数名: ReadWindowPosReg

函数作用: 读窗口位置索引

==============================================================================*/
void ReadWindowPosReg( void )
{
HKEY    hKey;
ULONG   ParamSize, type, i;
LONG   x, y;

    RegCreateKey( HKEY_CURRENT_USER, CH341DP_SETTINGS_KEY,	&hKey );

    ParamSize = sizeof( x );
    type = REG_DWORD;
    i = RegQueryValueEx( hKey, TEXT("x_pos"), NULL, &type,
        (LPBYTE)&x, &ParamSize );
    if( i==0x02 )
        return;

    ParamSize = sizeof( y );
    type = REG_DWORD;
    i = RegQueryValueEx( hKey, TEXT("y_pos"), NULL, &type,
        (LPBYTE)&y, &ParamSize );
    if( i==0x02 )
        return;

    // 判断是否在屏幕范围内
    if( ( (x< 0) || (x > GetSystemMetrics( SM_CXFULLSCREEN ) ))  
        || ( (y< 0) || (y > GetSystemMetrics( SM_CYFULLSCREEN )) ) )
        return;
    MoveWindow( MainDialog, x, y, 474, 244, TRUE );
}

/*==============================================================================

函数名: WriteWindowPosReg

函数作用: 写窗口位置索引

==============================================================================*/
void WriteWindowPosReg( void )
{
HKEY    hKey;
RECT    rt;
 
    // 最小化时不保存窗口位置
    if( IsIconic( MainDialog ) )
        return;
    GetWindowRect( MainDialog, &rt );
    RegCreateKey( HKEY_CURRENT_USER, CH341DP_SETTINGS_KEY,	&hKey );
    RegSetValueEx( hKey, TEXT("x_pos"), 0, REG_DWORD, (CONST BYTE *)&rt.left,
        sizeof( rt.left ) );
    RegSetValueEx( hKey, TEXT("y_pos"), 0, REG_DWORD, (CONST BYTE *)&rt.top,
        sizeof( rt.top ) );
    RegCloseKey( hKey );
}

/*==============================================================================

函数名: SetWindowCenter

函数作用: 让子窗口位于主窗口的中间位置

==============================================================================*/
static  void SetWindowCenter( HWND hSonWindow )
{
RECT DesktopRect, NewRect, OldMainRect;
POINT pCenter, pStart;
LONG width, height;

    // 检查主窗口是否最小化
    if( IsIconic( MainDialog ) )
        ShowWindow( MainDialog, SW_SHOWNORMAL );

    GetWindowRect( MainDialog, &OldMainRect );
    GetWindowRect( GetDesktopWindow(), &DesktopRect );
    GetWindowRect( hSonWindow, &NewRect );          // 得到子窗口的长与宽

    width = NewRect.right - NewRect.left;
    height = NewRect.bottom - NewRect.top;

    // calculate message box starting point
    pCenter.x = OldMainRect.left+((OldMainRect.right - OldMainRect.left)/2);
    pCenter.y = OldMainRect.top+((OldMainRect.bottom - OldMainRect.top)/2);

    // adjust if message box is off desktop
    pStart.x = (pCenter.x - (width/2) );
    pStart.y = (pCenter.y - (height/2) );

    if(pStart.x < 0)
        pStart.x = 0;
    if(pStart.y < 0)
        pStart.y = 0;

    if(pStart.x + width > DesktopRect.right)
        pStart.x = DesktopRect.right - width;
    if(pStart.y + height > DesktopRect.bottom)
        pStart.y = DesktopRect.bottom - height;

    MoveWindow( hSonWindow, pStart.x, pStart.y, width, height, TRUE );
}

static HHOOK   hHook;
/*==============================================================================

函数名:  CBTProc

函数作用: hook MessageBoxCenter 使其处在应用程序中间位置

==============================================================================*/
LRESULT
CALLBACK
CBTProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    if(nCode == HCBT_ACTIVATE)
    {
        SetWindowCenter( (HWND)wParam );
        UnhookWindowsHookEx( hHook );
    }
    return CallNextHookEx( hHook, nCode, wParam, lParam );
}

/*==============================================================================

函数名:  SetWindowHook

函数作用: 设置挂钩

==============================================================================*/
VOID SetWindowHook( VOID )
{
   hHook = SetWindowsHookEx( WH_CBT, CBTProc, GetModuleHandle(NULL),
       GetCurrentThreadId() );
}

/*==============================================================================

函数名: MessageBoxCenter

函数作用: 让对话框位于主窗口的中间位置

==============================================================================*/
int MessageBoxCenter(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption,UINT uType )
{
    SetWindowHook( );
    return MessageBox( hWnd, lpText, lpCaption,uType );
}

/*==============================================================================

函数名: IniCh341Device

函数作用: 初始化CH341设备

==============================================================================*/
void IniCh341Device( void )
{
    Ch341Handle = CH341OpenDevice( Ch341index );
    if ( Ch341Handle  == INVALID_HANDLE_VALUE )
    {
        EnableWindow( GetDlgItem( MainDialog, IDC_BUTTON_DOWN ), 0 );
        EnableWindow( GetDlgItem( MainDialog, IDC_SET_CONFIG), 0);
        EnableWindow( GetDlgItem( MainDialog, IDC_RUNING ), 0 );
        EnableWindow( GetDlgItem( MainDialog, IDC_BUTTON_BROWER ), 0 );
        SetDlgItemText( MainDialog, IDC_STATIC_PROMPT, _T("Lost connection with the CH341 device") );
        Ch341State = FALSE;
    }
    else
    {
        CH341SetExclusive( Ch341index, TRUE );
        CH341ChipVer = CH341GetVerIC( Ch341index );
        CH341SPIBit = FALSE;
        if( CH341ChipVer >= 0x30 )
        {
            CH341SPIBit = TRUE;
            // 下面设置串口流模式
            CH341SetStream( Ch341index, 0x81 );
        }

        // 不支持 则禁止
        SetSpiSysMenu( );
        // 如果支持SPI 则读配置参数
        if( CH341SPIBit )
            ReadSpiRegConfig( );

        EnableWindow( GetDlgItem( MainDialog, IDC_SET_CONFIG), 1 );
        EnableWindow( GetDlgItem( MainDialog, IDC_RUNING ), 1 );
        EnableWindow( GetDlgItem( MainDialog, IDC_BUTTON_BROWER ), 1 );
        EnableWindow( GetDlgItem( MainDialog, IDC_BUTTON_DOWN ), FileOpenBit );
        SetDlgItemText( MainDialog, IDC_STATIC_PROMPT, _T("The CH341 device is turned on successfully") );
        Ch341State = TRUE;
    }
}

/*==============================================================================

函数名: CH341DP_NOTIFY_ROUTINE

函数作用: 监视CH341设备插拔中断回调函数

==============================================================================*/
void CALLBACK CH341DP_NOTIFY_ROUTINE(ULONG iDevIndexAndEvent)
{
PVOID pName;
UCHAR i;

    if( iDevIndexAndEvent == 3 )        // 设备插入事件,已经插入
    {
        if( Ch341State == FALSE )       // 当前设备不可用则打开设备
        {
            Ch341index = 0;             // 从0设备开始扫描设备,找到序号较小的设备
            for( i=0; i!=mCH341_MAX_NUMBER; i++)
            {
                Ch341index = i;
                IniCh341Device( );
                if( Ch341State == TRUE )
                    break;
            }
        }
    }
    else if( CH341_DEVICE_REMOVE == 0 ) // 设备拔出事件,已经拔出
    {
        if( Ch341State == TRUE )
        {
            pName = CH341GetDeviceName( Ch341index );
            if( pName == NULL )         // 得到设备名出错,重新初始化设备
            {
                IniCh341Device( );
            }
        }
    }
}

/*==============================================================================

函数名: ShowTitleName

函数作用: 在Title上显示文件名

==============================================================================*/
void ShowTitleName( void )
{
PCHAR   p;
TCHAR   BakDownFileBuf[260+32];

    p = strrchr( DownFileBuf, '\\' );
    if( p )
        _stprintf( BakDownFileBuf, _T (APPNAME" "APPVER " - [%s]"), p+1 );
    else
        _stprintf( BakDownFileBuf, _T (APPNAME" "APPVER " - [%s]"), DownFileBuf );
    SetWindowText( MainDialog, BakDownFileBuf );
}

/*==============================================================================

函数名: InitRunGui

函数作用: 初始化运行时界面

==============================================================================*/
void InitRunGui( BOOL RunBit )
{
ULONG i;

    EnableWindow( GetDlgItem( MainDialog, IDC_EPRAM ), !RunBit );
    if( !RunBit )
    {   // 启用EEPROM窗口
        i = SendDlgItemMessage( MainDialog, IDC_MCU_MODEL, CB_GETCURSEL, 0, 0 ); 
        if(i >= ATMEL_AT89S51_MODEL)
            EnableWindow( GetDlgItem( MainDialog, IDC_EPRAM ), 0 );
    }

    EnableWindow( GetDlgItem( MainDialog, IDC_SET_CONFIG ), !RunBit );
    EnableWindow( GetDlgItem( MainDialog, IDC_BUTTON_BROWER ), !RunBit );
    EnableWindow( GetDlgItem( MainDialog, IDC_RUNING ), !RunBit );
    if( RunBit )
        SetDlgItemText( MainDialog, IDC_BUTTON_DOWN, _T( "Halt") );
    else
        SetDlgItemText( MainDialog, IDC_BUTTON_DOWN, _T( "Download" ) );
    if( !RunBit )
    {
        if( MainDialog != GetForegroundWindow( ) )
            FlashWindow( MainDialog, TRUE );
    }
}

/*==============================================================================

函数名: _DownThread

函数作用: Download程序的线程

==============================================================================*/
void _DownThread( HANDLE hDialog )
{
HANDLE FileHandle;              // 存放打开文件的句柄
ULONG FileLong;                 // 存放Download文件长度
LPVOID pFile;                   // 文件内存映射数据指针
TCHAR CharBuf[256];             // 字符缓冲区
ULONG len;
UCHAR i;

    InitRunGui( TRUE );
    SendMessage( HwndProg, PBM_SETPOS, (WPARAM)DownProgRange.iLow, (LPARAM)0 );
                            // 初始化Download工作(分析此MCU是否可编程,设为可编程状态)
    SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("The MCU model is being identified...") );
    i = IniMcuDown( );
    if( i == DATA_TRANS_ERR )
    {
        SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("The data transfer failed") );
        goto PRG_EXIT;
    }
    else if( i == NO_MCU_ERR )                  // This MCU is not recognized
    {
        SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("This MCU is not recognized") );
        goto PRG_EXIT;
    }
    else if( i == MCU_MODEL_ERR )               // 型号错误
    {
        SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("Wrong MCU model") );
        goto PRG_EXIT;
    }
    else if( i == NO_SUPORT_ERR )
    {
        SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("This MCU is not supported") );
    }

    FileHandle = CreateFile( DownFileBuf, GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
    if( FileHandle == INVALID_HANDLE_VALUE )
    {
        SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("Failed to open the file") );
        goto PRG_EXIT;
    }
                                // 得到文件长度
    FileLong = GetFileSize( FileHandle, NULL );
    if( FileLong == 0  ||  FileLong > MAX_FILE_LEN )
    {
        SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("The file size is wrong") );
        CloseHandle( FileHandle );
        goto PRG_EXIT;
    }

    if( !ReadFile( FileHandle, pFileBuf, FileLong, &FileLong, NULL ) )
    {
        SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("Read file error") );
        CloseHandle( FileHandle );
        goto PRG_EXIT;
    }
    CloseHandle( FileHandle );

    pFile = pFileBuf;
    len = _tcslen( DownFileBuf );    // 得到此文件路径长度
    if( !memicmp( &DownFileBuf[len-3], _T("HEX"), 3 ) )
    {                               // 是HEX文件吗?是则先转换为BIN文件
        SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("File conversion is in progress...") );
        if( HexToBin( pFile, FileLong, pBinBuf, &FileLong ) == FALSE )
        {
            SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("Wrong HEX file") );
            goto PRG_EXIT;
        }
        pFile = pBinBuf;            // 把文件指针替换为BIN缓冲区指针
    }

    if( IsEepromData == FALSE )
    {                               // Download程序数据到MCU
        _stprintf( CharBuf, _T("FLASH for %s is being programmed..."), McuName[McuType] );
        SetDlgItemText( hDialog, IDC_STATIC_PROMPT, CharBuf );
        i = DownPrgToMcu( pFile, FileLong );
    }
    else
    {                               // Download the data to the EEPROM
        _stprintf( CharBuf, _T("The EEPROM for %s is being programmed..."), McuName[McuType] );
        SetDlgItemText( hDialog, IDC_STATIC_PROMPT, CharBuf );
        i = DownDataToMcu( pFile, FileLong );
    }

    if( i == NO_ERR )
    {
        _stprintf( CharBuf, _T("The download is successful, and a total of %d bytes are downloaded"), FileLong );
        //MessageBoxCenter( hDialog, CharBuf,_T(APPNAME), MB_OK | MB_ICONINFORMATION);
        SetDlgItemText( hDialog, IDC_STATIC_PROMPT, CharBuf );
        goto PRG_EXIT;           // 用于在弹出信息框对文件进行关闭
    }
    else if( i == DATA_TRANS_ERR )
    {
        //SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("The data transfer failed,重新刷新设备" );
    }
    else if( i == DATA_CMP_ERR )
    {
        SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("The data verification error is incorrect, please download it again") );
    }
    else if( i == DATA_LEN_ERR )
    {
        SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("The valid length of the download file exceeds the chip capacity") );
    }
    else if( i == REQ_STOP_ERR )
    {
        SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("Termination from User") );
    }
    else
    {
        SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("Unknown error") );
    }

    PRG_EXIT:
    SendMessage( HwndProg, PBM_SETPOS, (WPARAM)DownProgRange.iLow, (LPARAM)0 );
    InitRunGui( FALSE );
    Downhandle = INVALID_HANDLE_VALUE;
    return;
}

/*==============================================================================

函数名: SetMcuConfig

函数作用:  配置对话框回调函数

==============================================================================*/
LRESULT CALLBACK MegaSetMcuConfig( HWND hDialog, UINT uMessage, WPARAM wParam, LPARAM lParam )
{
INT wmId, wmEvent;
UCHAR CmdBuf[256];
TCHAR CharBUf[256];

    switch( uMessage )
    {
        case WM_INITDIALOG:
        {
            // 改变标题
            if( ( McuType == ATMEL_MEGA16_MODEL ) || ( McuType == ATMEL_MEGA32_MODEL )
                || ( McuType == ATMEL_MEGA64_MODEL ) || ( McuType == ATMEL_MEGA128_MODEL ) )
            {
                SetDlgItemText( hDialog, IDC_CHECK_RSTD, _T("OCDEN") );
                SetDlgItemText( hDialog, IDC_CHECK_WDTON, _T("JTAGEN") );
            }
            else if( McuType == ATMEL_MEGA8515_MODEL )
            {
                SetDlgItemText( hDialog, IDC_CHECK_RSTD, _T("S8515C") );
            }
            else if( McuType == ATMEL_MEGA8535_MODEL )
            {
                SetDlgItemText( hDialog, IDC_CHECK_RSTD, _T("S8535C") );
            }
            else if( ( McuType == ATMEL_MEGA48_MODEL )
                || ( McuType == ATMEL_MEGA88_MODEL ) || ( McuType == ATMEL_MEGA168_MODEL ) )
            {
                SetDlgItemText( hDialog, IDC_CHECK_WDTON, _T("DWEN") );
                SetDlgItemText( hDialog, IDC_CHECK_CKOPT, _T("WDTON") );
                SetDlgItemText( hDialog, IDC_CHECK_BOOTSZ1, _T("BLEVEL2") );
                SetDlgItemText( hDialog, IDC_CHECK_BOOTSZ0, _T("BLEVEL1") );
                SetDlgItemText( hDialog, IDC_CHECK_BOOTRST, _T("BLEVEL0") );

                SetDlgItemText( hDialog, IDC_CHECK_BODLEVEL, _T("CKDIV8") );
                SetDlgItemText( hDialog, IDC_CHECK_BODEN, _T("CKOUT") );
            }

            SetDlgItemText( hDialog, IDC_CHECK_WDTON1, _T("0") );
            SetDlgItemText( hDialog, IDC_CHECK_M103C, _T("1") );
            SetDlgItemText( hDialog, IDC_EXPAND_SET2, _T("2") );
            if( McuType == ATMEL_MEGA48_MODEL )
            {
                SetDlgItemText( hDialog, IDC_CHECK_WDTON1, _T("SPMEN") );
                EnableWindow( GetDlgItem( hDialog, IDC_CHECK_M103C ), 0 );
                EnableWindow( GetDlgItem( hDialog, IDC_EXPAND_SET2 ), 0 );
            }
            else if( ( McuType == ATMEL_MEGA128_MODEL ) || ( McuType == ATMEL_MEGA64_MODEL ) )
            {
                SetDlgItemText( hDialog, IDC_CHECK_WDTON1, _T("WDTON") );
                SetDlgItemText( hDialog, IDC_CHECK_M103C, _T("M103C") );
                EnableWindow( GetDlgItem( hDialog, IDC_EXPAND_SET2 ), 0 );
            }
            else if( ( McuType == ATMEL_MEGA88_MODEL ) || ( McuType == ATMEL_MEGA168_MODEL ) )
            {
                SetDlgItemText( hDialog, IDC_CHECK_WDTON1, _T("BTRST") );
                SetDlgItemText( hDialog, IDC_CHECK_M103C, _T("BTSZ0") );
                SetDlgItemText( hDialog, IDC_EXPAND_SET2, _T("BTSZ1") );
            }
            else
            {
                EnableWindow( GetDlgItem( hDialog, IDC_CHECK_M103C ), 0 );
                EnableWindow( GetDlgItem( hDialog, IDC_CHECK_WDTON1 ), 0 );
                EnableWindow( GetDlgItem( hDialog, IDC_EXPAND_SET2 ), 0 );
            }
            // 默认开始读配置
            SendMessage( hDialog, WM_COMMAND, MAKELONG(IDC_READ_CONG, 0xffff), 0 );
            SetWindowCenter( hDialog );
        }
        return TRUE;

        case WM_COMMAND:
        {
            wmId = LOWORD(wParam);
            wmEvent = HIWORD(wParam);
            switch( wmId )
            {
                case IDC_SET_ALL:
                {
                    CheckDlgButton( hDialog, IDC_CHECK_BLOCk12, BST_CHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_BLOCk11, BST_CHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_BLOCk02, BST_CHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_BLOCk01, BST_CHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_BLOCk01, BST_CHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_LOCK2, BST_CHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_LOCK1, BST_CHECKED );

                    CheckDlgButton( hDialog, IDC_CHECK_RSTD, BST_CHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_WDTON, BST_CHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_SPIEN, BST_CHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_CKOPT, BST_CHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_EESAVE, BST_CHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_BOOTSZ1, BST_CHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_BOOTSZ0, BST_CHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_BOOTRST, BST_CHECKED );

                    CheckDlgButton( hDialog, IDC_CHECK_BODLEVEL, BST_CHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_BODEN, BST_CHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_SUT1, BST_CHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_SUT0, BST_CHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_CKSEL3, BST_CHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_CKSEL2, BST_CHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_CKSEL1, BST_CHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_CKSEL0, BST_CHECKED );

                    if( McuType == ATMEL_MEGA128_MODEL )
                    {
                        CheckDlgButton( hDialog, IDC_CHECK_M103C, BST_CHECKED );
                        CheckDlgButton( hDialog, IDC_CHECK_WDTON1, BST_CHECKED );
                    }
                    else if( McuType == ATMEL_MEGA48_MODEL )
                    {
                        CheckDlgButton( hDialog, IDC_CHECK_WDTON1, BST_CHECKED );
                    }
                    else if( ( McuType == ATMEL_MEGA88_MODEL ) || McuType == ATMEL_MEGA168_MODEL )
                    {
                        CheckDlgButton( hDialog, IDC_EXPAND_SET2, BST_CHECKED );
                        CheckDlgButton( hDialog, IDC_CHECK_M103C, BST_CHECKED );
                        CheckDlgButton( hDialog, IDC_CHECK_WDTON1, BST_CHECKED );
                    }

                }
                return TRUE;

                case IDC_CLR_ALL:
                {
                    CheckDlgButton( hDialog, IDC_CHECK_BLOCk12, BST_UNCHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_BLOCk11, BST_UNCHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_BLOCk02, BST_UNCHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_BLOCk01, BST_UNCHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_BLOCk01, BST_UNCHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_LOCK2, BST_UNCHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_LOCK1, BST_UNCHECKED );

                    CheckDlgButton( hDialog, IDC_CHECK_RSTD, BST_UNCHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_WDTON, BST_UNCHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_SPIEN, BST_UNCHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_CKOPT, BST_UNCHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_EESAVE, BST_UNCHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_BOOTSZ1, BST_UNCHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_BOOTSZ0, BST_UNCHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_BOOTRST, BST_UNCHECKED );

                    CheckDlgButton( hDialog, IDC_CHECK_BODLEVEL, BST_UNCHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_BODEN, BST_UNCHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_SUT1, BST_UNCHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_SUT0, BST_UNCHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_CKSEL3, BST_UNCHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_CKSEL2, BST_UNCHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_CKSEL1, BST_UNCHECKED );
                    CheckDlgButton( hDialog, IDC_CHECK_CKSEL0, BST_UNCHECKED );

                    if( McuType == ATMEL_MEGA128_MODEL )
                    {
                        CheckDlgButton( hDialog, IDC_CHECK_M103C, BST_UNCHECKED );
                        CheckDlgButton( hDialog, IDC_CHECK_WDTON1, BST_UNCHECKED );
                    }
                    else if( McuType == ATMEL_MEGA48_MODEL )
                    {
                        CheckDlgButton( hDialog, IDC_CHECK_WDTON1, BST_UNCHECKED );
                    }
                    else if( ( McuType == ATMEL_MEGA88_MODEL ) || McuType == ATMEL_MEGA168_MODEL )
                    {
                        CheckDlgButton( hDialog, IDC_EXPAND_SET2, BST_UNCHECKED );
                        CheckDlgButton( hDialog, IDC_CHECK_M103C, BST_UNCHECKED );
                        CheckDlgButton( hDialog, IDC_CHECK_WDTON1, BST_UNCHECKED );
                    }
                }
                return TRUE;

                case IDC_READ_CONG:
                {
                    CmdBuf[0] = 0x58;                           // 读锁定位
                    CmdBuf[1] = 0x00;
                    CmdBuf[2] = 0x00;
                    if( MegaSpiOutInData( Ch341index, 4, CmdBuf, FALSE ) == FALSE )
                    {
                        MessageBoxCenter( hDialog, _T("The data transfer failed"), _T(APPNAME),
                            MB_OK | MB_ICONEXCLAMATION );
                        return TRUE;
                    }
                    //MHEX( CmdBuf, 4, "命令1" );

                    CheckDlgButton( hDialog, IDC_CHECK_BLOCk12, !GET_DWORD_BIT( CmdBuf[3], 5) );
                    CheckDlgButton( hDialog, IDC_CHECK_BLOCk11, !GET_DWORD_BIT( CmdBuf[3], 4) );
                    CheckDlgButton( hDialog, IDC_CHECK_BLOCk02, !GET_DWORD_BIT( CmdBuf[3], 3) );
                    CheckDlgButton( hDialog, IDC_CHECK_BLOCk01, !GET_DWORD_BIT( CmdBuf[3], 2) );
                    CheckDlgButton( hDialog, IDC_CHECK_LOCK2, !GET_DWORD_BIT( CmdBuf[3], 1) );
                    CheckDlgButton( hDialog, IDC_CHECK_LOCK1, !GET_DWORD_BIT( CmdBuf[3], 0) );

                    if( McuType == ATMEL_MEGA128_MODEL || ( McuType == ATMEL_MEGA48_MODEL )
                        || ( McuType == ATMEL_MEGA88_MODEL ) || ( McuType == ATMEL_MEGA128_MODEL ))
                    {
                        CmdBuf[0] = 0x50;                       // 读扩展熔丝位
                        CmdBuf[1] = 0x08;
                        CmdBuf[2] = 0x00;
                        if( MegaSpiOutInData( Ch341index, 4, CmdBuf, FALSE ) == FALSE )
                        {
                            MessageBoxCenter( hDialog, _T("The data transfer failed"),
                                _T(APPNAME), MB_OK | MB_ICONEXCLAMATION );
                            return TRUE;
                        }
                        //MHEX( CmdBuf, 4, "命令3" );
                        CheckDlgButton( hDialog, IDC_EXPAND_SET2, !GET_DWORD_BIT( CmdBuf[3], 2) );
                        CheckDlgButton( hDialog, IDC_CHECK_M103C, !GET_DWORD_BIT( CmdBuf[3], 1) );
                        CheckDlgButton( hDialog, IDC_CHECK_WDTON1, !GET_DWORD_BIT( CmdBuf[3], 0) );
                    }

                    CmdBuf[0] = 0x58;                          // 读高熔丝位
                    CmdBuf[1] = 0x08;
                    CmdBuf[2] = 0x00;
                    if( MegaSpiOutInData( Ch341index, 4, CmdBuf, FALSE ) == FALSE )
                    {
                        MessageBoxCenter( hDialog, _T("The data transfer failed"), _T(APPNAME),
                            MB_OK | MB_ICONEXCLAMATION);
                        return TRUE;
                    }
                    //MHEX( CmdBuf, 4, "命令2" );
                    CheckDlgButton( hDialog, IDC_CHECK_RSTD,  !GET_DWORD_BIT( CmdBuf[3], 7) );
                    CheckDlgButton( hDialog, IDC_CHECK_WDTON, !GET_DWORD_BIT( CmdBuf[3], 6) );
                    CheckDlgButton( hDialog, IDC_CHECK_SPIEN, !GET_DWORD_BIT( CmdBuf[3], 5) );
                    CheckDlgButton( hDialog, IDC_CHECK_CKOPT, !GET_DWORD_BIT( CmdBuf[3], 4) );
                    CheckDlgButton( hDialog, IDC_CHECK_EESAVE,  !GET_DWORD_BIT( CmdBuf[3], 3) );
                    CheckDlgButton( hDialog, IDC_CHECK_BOOTSZ1, !GET_DWORD_BIT( CmdBuf[3], 2) );
                    CheckDlgButton( hDialog, IDC_CHECK_BOOTSZ0, !GET_DWORD_BIT( CmdBuf[3], 1) );
                    CheckDlgButton( hDialog, IDC_CHECK_BOOTRST, !GET_DWORD_BIT( CmdBuf[3], 0) );

                    CmdBuf[0] = 0x50;                       // 读低熔丝位
                    CmdBuf[1] = 0x00;
                    CmdBuf[2] = 0x00;
                    if( MegaSpiOutInData( Ch341index, 4, CmdBuf, FALSE ) == FALSE )
                    {
                        MessageBoxCenter( hDialog, _T("The data transfer failed"), _T(APPNAME),
                            MB_OK | MB_ICONEXCLAMATION );
                        return TRUE;
                    }
                    //MHEX( CmdBuf, 4, "命令3" );
                    CheckDlgButton( hDialog, IDC_CHECK_BODLEVEL, !GET_DWORD_BIT( CmdBuf[3], 7) );
                    CheckDlgButton( hDialog, IDC_CHECK_BODEN, !GET_DWORD_BIT( CmdBuf[3], 6) );
                    CheckDlgButton( hDialog, IDC_CHECK_SUT1,  !GET_DWORD_BIT( CmdBuf[3], 5) );
                    CheckDlgButton( hDialog, IDC_CHECK_SUT0,  !GET_DWORD_BIT( CmdBuf[3], 4) );
                    CheckDlgButton( hDialog, IDC_CHECK_CKSEL3, !GET_DWORD_BIT( CmdBuf[3], 3) );
                    CheckDlgButton( hDialog, IDC_CHECK_CKSEL2, !GET_DWORD_BIT( CmdBuf[3], 2) );
                    CheckDlgButton( hDialog, IDC_CHECK_CKSEL1, !GET_DWORD_BIT( CmdBuf[3], 1) );
                    CheckDlgButton( hDialog, IDC_CHECK_CKSEL0, !GET_DWORD_BIT( CmdBuf[3], 0) );
                    if( wmEvent != 0xffff )
                    {   // 第一次打开不弹出对话框
                        _stprintf( CharBUf, _T("%s The read configuration is successful!"), McuName[McuType] );
                        MessageBoxCenter( hDialog, CharBUf, _T(APPNAME), MB_OK | MB_ICONINFORMATION );
                    }
                    else
                    {
                        _stprintf( CharBUf, _T("%s Encryption settings and configurations"), McuName[McuType] );
                        SetWindowText( hDialog, CharBUf );
                    }
                }
                return TRUE;

                case IDC_WRITE_CONG:
                {
                    //　读取标志位写入MCU
                    CmdBuf[3] = 0XFF;       //默认全为1为未编程
                    if( IsDlgButtonChecked( hDialog, IDC_CHECK_BLOCk12 ) == BST_CHECKED )
                        CLR_DWORD_BIT( CmdBuf[3], 5 );
                    if( IsDlgButtonChecked( hDialog, IDC_CHECK_BLOCk11 ) == BST_CHECKED )
                        CLR_DWORD_BIT( CmdBuf[3], 4 );
                    if( IsDlgButtonChecked( hDialog, IDC_CHECK_BLOCk02 ) == BST_CHECKED )
                        CLR_DWORD_BIT( CmdBuf[3], 3 );
                    if( IsDlgButtonChecked( hDialog, IDC_CHECK_BLOCk01 ) == BST_CHECKED )
                        CLR_DWORD_BIT( CmdBuf[3], 2 );
                    if( IsDlgButtonChecked( hDialog, IDC_CHECK_LOCK2 )== BST_CHECKED )
                        CLR_DWORD_BIT( CmdBuf[3], 1 );
                    if( IsDlgButtonChecked( hDialog, IDC_CHECK_LOCK1 )== BST_CHECKED )
                        CLR_DWORD_BIT( CmdBuf[3], 0 );

                    CmdBuf[0] = 0xAC;                       // 写锁定位
                    CmdBuf[1] = 0xE0;
                    CmdBuf[2] = 0x00;
                    //MHEX( CmdBuf, 4, "命令1" );
                    if( MegaSpiOutInData( Ch341index, 4, CmdBuf, FALSE ) == FALSE )
                    {
                        MessageBoxCenter( hDialog, _T("The data transfer failed"), _T(APPNAME),
                            MB_OK | MB_ICONEXCLAMATION);
                        return TRUE;
                    }
                    Sleep( 20 );                            // 最少等待4.5ms

                    if( McuType == ATMEL_MEGA128_MODEL || ( McuType == ATMEL_MEGA48_MODEL )
                        || ( McuType == ATMEL_MEGA88_MODEL ) || ( McuType == ATMEL_MEGA128_MODEL ))
                    {
                        CmdBuf[3] = 0XFF;                   // 默认全为1为未编程
                        if( IsDlgButtonChecked( hDialog, IDC_EXPAND_SET2 ) == BST_CHECKED )
                            CLR_DWORD_BIT( CmdBuf[3], 2 );
                        if( IsDlgButtonChecked( hDialog, IDC_CHECK_M103C ) == BST_CHECKED )
                            CLR_DWORD_BIT( CmdBuf[3], 1 );
                        if( IsDlgButtonChecked( hDialog, IDC_CHECK_WDTON1 ) == BST_CHECKED )
                            CLR_DWORD_BIT( CmdBuf[3], 0 );

                        CmdBuf[0] = 0xAC;                   // 写扩展熔丝位
                        CmdBuf[1] = 0xA4;
                        CmdBuf[2] = 0x00;
                        //MHEX( CmdBuf, 4, "命令2" );
                        if( MegaSpiOutInData( Ch341index, 4, CmdBuf, FALSE ) == FALSE )
                        {
                            MessageBoxCenter( hDialog, _T("The data transfer failed"), _T(APPNAME),
                                MB_OK | MB_ICONEXCLAMATION);
                            return TRUE;
                        }
                        Sleep( 20 );
                    }

                    CmdBuf[3] = 0XFF;                       // 默认全为1为未编程
                    if( IsDlgButtonChecked( hDialog, IDC_CHECK_RSTD ) == BST_CHECKED )
                        CLR_DWORD_BIT( CmdBuf[3], 7 );
                    if( IsDlgButtonChecked( hDialog, IDC_CHECK_WDTON ) == BST_CHECKED )
                        CLR_DWORD_BIT( CmdBuf[3], 6 );
                                                            // 没有对SPIEN位进行设置
                    if( IsDlgButtonChecked( hDialog, IDC_CHECK_CKOPT ) == BST_CHECKED )
                        CLR_DWORD_BIT( CmdBuf[3], 4 );
                    if( IsDlgButtonChecked( hDialog, IDC_CHECK_EESAVE ) == BST_CHECKED )
                        CLR_DWORD_BIT( CmdBuf[3], 3 );
                    if( IsDlgButtonChecked( hDialog, IDC_CHECK_BOOTSZ1 ) == BST_CHECKED )
                        CLR_DWORD_BIT( CmdBuf[3], 2 );
                    if( IsDlgButtonChecked( hDialog, IDC_CHECK_BOOTSZ0 ) == BST_CHECKED )
                        CLR_DWORD_BIT( CmdBuf[3], 1 );
                    if( IsDlgButtonChecked( hDialog, IDC_CHECK_BOOTRST ) == BST_CHECKED )
                        CLR_DWORD_BIT( CmdBuf[3], 0 );

                    CmdBuf[0] = 0xAC;                       // 读高熔丝位
                    CmdBuf[1] = 0xA8;
                    CmdBuf[2] = 0x00;
                    //MHEX( CmdBuf, 4, "命令2" );
                    if( MegaSpiOutInData( Ch341index, 4, CmdBuf, FALSE ) == FALSE )
                    {
                        MessageBoxCenter( hDialog, _T("The data transfer failed"), _T(APPNAME),
                            MB_OK | MB_ICONINFORMATION );
                        return TRUE;
                    }
                    Sleep( 20 );                                 // 最少等待4.5ms

                    CmdBuf[3] = 0XFF;                           // 默认全为1为未编程
                    if( IsDlgButtonChecked( hDialog, IDC_CHECK_BODLEVEL ) == BST_CHECKED )
                        CLR_DWORD_BIT( CmdBuf[3], 7 );
                    if( IsDlgButtonChecked( hDialog, IDC_CHECK_BODEN ) == BST_CHECKED )
                        CLR_DWORD_BIT( CmdBuf[3], 6 );
                    if( IsDlgButtonChecked( hDialog, IDC_CHECK_SUT1 ) == BST_CHECKED )
                        CLR_DWORD_BIT( CmdBuf[3], 5 );
                    if( IsDlgButtonChecked( hDialog, IDC_CHECK_SUT0 ) == BST_CHECKED )
                        CLR_DWORD_BIT( CmdBuf[3], 4 );
                    if( IsDlgButtonChecked( hDialog, IDC_CHECK_CKSEL3 ) == BST_CHECKED )
                        CLR_DWORD_BIT( CmdBuf[3], 3 );
                    if( IsDlgButtonChecked( hDialog, IDC_CHECK_CKSEL2 ) == BST_CHECKED )
                        CLR_DWORD_BIT( CmdBuf[3], 2 );
                    if( IsDlgButtonChecked( hDialog, IDC_CHECK_CKSEL1 ) == BST_CHECKED )
                        CLR_DWORD_BIT( CmdBuf[3], 1 );
                    if( IsDlgButtonChecked( hDialog, IDC_CHECK_CKSEL0 ) == BST_CHECKED )
                        CLR_DWORD_BIT( CmdBuf[3], 0 );

                    CmdBuf[0] = 0xAC;                           // 读低熔丝位
                    CmdBuf[1] = 0xA0;
                    CmdBuf[2] = 0x00;
                    //MHEX( CmdBuf, 4, "命令3" );
                    if( MegaSpiOutInData( Ch341index, 4, CmdBuf, FALSE ) == FALSE )
                    {
                        MessageBoxCenter( hDialog, _T("The data transfer failed"), _T(APPNAME),
                            MB_OK | MB_ICONINFORMATION );
                        return TRUE;
                    }
                    Sleep( 20 );                                 // 最少等待4.5ms

                    _stprintf( CharBUf, _T("%s The write configuration is successful!"), McuName[McuType] );
                    MessageBoxCenter( hDialog, CharBUf, _T(APPNAME), MB_OK | MB_ICONINFORMATION );
                }
                return TRUE;

                case IDC_CLOSE:
                {
                     EndDialog( hDialog, 0);
                }
                return TRUE;
            }
        }
        return TRUE;

        case WM_CLOSE:
            EndDialog( hDialog, 0);                         // 关闭对话框,返回0
        return TRUE;
    }
    return FALSE;
}


/*==============================================================================

函数名: S51SetMcuConfig

函数作用: S51配置对话框回调函数

==============================================================================*/
LRESULT CALLBACK S51SetMcuConfig( HWND hDialog, UINT uMessage, WPARAM wParam, LPARAM lParam )
{
INT wmId, wmEvent;
UCHAR CmdBuf[32];
TCHAR CharBUf[32];
UINT i, k;

    switch( uMessage )
    {
        case WM_INITDIALOG:
        {
            // 默认开始读配置
            SendMessage( hDialog, WM_COMMAND, MAKELONG(IDC_READ_CONG, 0XFFFF), 0 );
            SetWindowCenter( hDialog );
        }
        return TRUE;

        case WM_COMMAND:
        {
            wmId = LOWORD(wParam);
            wmEvent = HIWORD(wParam);
            switch( wmId )
            {
                case IDC_READ_CONG:
                {
                    CmdBuf[0] =0X24;                // Read Lock Bits
                    if( At89sISPoutin( Ch341index, 3, CmdBuf, 1, CmdBuf ) == FALSE )
                    {
                        MessageBoxCenter( hDialog, _T("The data transfer failed"), _T(APPNAME),
                            MB_OK | MB_ICONINFORMATION );
                        return TRUE;
                    }
                    //MHEX( CmdBuf, 1, "CmdBuf[0]" );
                    if( ((CmdBuf[0] >> 2) & 0x07) & 0x04 )
                    {
                        CheckRadioButton( hDialog, IDC_RADIO1, IDC_RADIO4,
                        IDC_RADIO4 );
                    }
                    else  if( ((CmdBuf[0] >> 2) & 0x07) & 0x02 )
                    {
                        CheckRadioButton( hDialog, IDC_RADIO1, IDC_RADIO4,
                        IDC_RADIO3);
                    }
                    else  if( ((CmdBuf[0] >> 2) & 0x07) & 0x01 )
                    {
                        CheckRadioButton( hDialog, IDC_RADIO1, IDC_RADIO4,
                        IDC_RADIO2);
                    }
                    else
                    {
                        CheckRadioButton( hDialog, IDC_RADIO1, IDC_RADIO4,
                        IDC_RADIO1 );
                    }

                    if( wmEvent != 0xffff )
                    {   // 第一次打开不弹出对话框
                        _stprintf( CharBUf, _T("%s The read configuration is successful!"), McuName[McuType] );
                        MessageBoxCenter( hDialog, CharBUf, _T(APPNAME), MB_OK | MB_ICONINFORMATION );
                    }
                    else
                    {
                        _stprintf( CharBUf, _T("%s Encryption settings and configurations"), McuName[McuType] );
                        SetWindowText( hDialog, CharBUf );
                    }
                }
                return TRUE;

                case IDC_WRITE_CONG:
                {
                    for( i=0; i!=4; i++ )
                    {
                        if( IsDlgButtonChecked( hDialog, (IDC_RADIO1 + i) ) == BST_CHECKED )
                            break;
                    }
                    // 根据i值设定标志位
                    for( k=0; k<=i; k++ )
                    {
                        CmdBuf[0] = 0xAC;               // Write Lock Bits
                        CmdBuf[1] = ( 0xE0 | (UCHAR)k );
                        //MHEX( &CmdBuf[1], 1, "CmdBuf" );
                        if ( At89sISPoutput( Ch341index, 4, CmdBuf ) == FALSE )
                        {
                            MessageBoxCenter( hDialog, _T("The data transfer failed"), _T(APPNAME),
                                MB_OK | MB_ICONINFORMATION);
                            return TRUE;
                        }
                        Sleep( 5 );
                    }
                    _stprintf( CharBUf, _T("Write %s configuration successfully!"), McuName[McuType] );
                    MessageBoxCenter( hDialog, CharBUf, _T(APPNAME), MB_OK | MB_ICONINFORMATION );
                }
                return TRUE;

                case IDC_CLOSE:
                {
                     EndDialog( hDialog, 0);
                }
                return TRUE;
            }
        }
        return TRUE;

        case WM_CLOSE:
            EndDialog( hDialog, 0);                         // 关闭对话框,返回0
        return TRUE;
    }
    return FALSE;
}

/*==============================================================================

函数名: mDialogMain

函数作用: 主对话框回调函数

==============================================================================*/
LRESULT CALLBACK mDialogMain( HWND hDialog, UINT uMessage, WPARAM wParam, LPARAM lParam )
{
INT wmId, wmEvent;
ULONG i;
MENUITEMINFO mi;

    switch( uMessage )
    {
        case WM_INITDIALOG:
        {
            MainDialog = hDialog;
            // 初始化系统菜单
            hSysMenu = GetSystemMenu( hDialog, FALSE );
            RtlZeroMemory( &mi, sizeof(mi) );
            mi.cbSize       = sizeof( mi );
            mi.fMask        = MIIM_ID | MIIM_TYPE;
            mi.wID          = IDC_SET_SPI;
            mi.dwTypeData   = "MEGA fast mode (&S)";
            InsertMenuItem( hSysMenu, SC_CLOSE, FALSE, &mi );

            RtlZeroMemory( &mi, sizeof(mi) );
            mi.cbSize       = sizeof( mi );
            mi.fMask        = MIIM_ID | MIIM_TYPE;
            mi.wID          = IDC_CMD_HELP;
            mi.dwTypeData   = "Command help(&H)";
            InsertMenuItem( hSysMenu, SC_CLOSE, FALSE, &mi );

            RtlZeroMemory( &mi, sizeof(mi) );
            mi.cbSize       = sizeof( mi );
            mi.fMask        = MIIM_TYPE;
            mi.fType        = MFT_SEPARATOR;
            InsertMenuItem( hSysMenu, SC_CLOSE, FALSE, &mi );

            // 设置标题栏图标
            SendMessage( hDialog, WM_SETICON, ICON_BIG, 
                (LPARAM)LoadIcon( DownExeHins, (LPCTSTR)IDI_ICON1 ) );
            // 设置文件名显示
            if( FileOpenBit )
                ShowTitleName( );
            else
                SetWindowText( hDialog, _T( APPNAME " "APPVER " - http:\\\\wch.cn" ) );

            // 初始化进度条
            HwndProg = GetDlgItem( hDialog, IDC_DOWN_PROGRESS );
            SendMessage( HwndProg, PBM_SETRANGE,
                (WPARAM)0, (LPARAM)(MAKELPARAM( 0, MAX_PROG_LEN )) );
            SendMessage( HwndProg, PBM_SETPOS, (WPARAM)DownProgRange.iLow, (LPARAM)0 );
            
            // 初始化下拉菜单
            for( i=0; i!=MCU_COUNT; i++ )
                SendDlgItemMessage( hDialog, IDC_MCU_MODEL, CB_ADDSTRING, 0, (LPARAM)McuName[i] );
            SendDlgItemMessage( hDialog, IDC_MCU_MODEL, CB_SETCURSEL,ReadMcuIndexReg( ), 0 );
            if( CmdMode )
                SendDlgItemMessage( hDialog, IDC_MCU_MODEL, CB_SETCURSEL, McuType, 0 );
            // 检查EEPRAM选项
            EnableWindow( GetDlgItem( hDialog, IDC_EPRAM ), 1 );
            i = SendDlgItemMessage( hDialog, IDC_MCU_MODEL, CB_GETCURSEL, 0, 0 ); 
            if(i >= ATMEL_AT89S51_MODEL)
            {
                CheckDlgButton( hDialog, IDC_EPRAM, BST_UNCHECKED );
                EnableWindow( GetDlgItem( hDialog, IDC_EPRAM ), 0 );
            }

            // 读窗口位置
            ReadWindowPosReg( );

            // 检测设备连接
            for( i=0; i!=mCH341_MAX_NUMBER; i++)
            {
                Ch341index = i;
                IniCh341Device( );
                if( Ch341State == TRUE )
                {
                    if( CmdMode )
                    {
                        if( McuCmd == WRITE_FLASH_CMD )
                            SendMessage( hDialog, WM_COMMAND, MAKELONG(IDC_BUTTON_DOWN, 0), 0 );
                        else if( McuCmd == WRITE_EEPROM_CMD )
                        {
                            CheckDlgButton( hDialog, IDC_EPRAM, BST_CHECKED );
                            SendMessage( hDialog, WM_COMMAND, MAKELONG(IDC_BUTTON_DOWN, 0), 0 );
                        }
                        else if( McuCmd == RESET_MCU_CMD )
                            SendMessage( hDialog, WM_COMMAND, MAKELONG(IDC_RUNING, 0), 0 );
                    }
                    break;
                }
            }

            if( CH341SetDeviceNotify( 0, (PCHAR)0,  CH341DP_NOTIFY_ROUTINE ) )
            {
                if( Ch341State == FALSE)                        // 设备没有被打开，开始监视设备插入
                    SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("Monitoring for CH341 device plugging and unplugging...") );
            }
            else
                SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("Failed to set up CH341 device monitoring") );
        }
        return TRUE;

        case WM_SYSCOMMAND:
        {
            if( wParam == IDC_SET_SPI )
            {
                i = GetMenuState( hSysMenu, IDC_SET_SPI, MF_BYCOMMAND );
                if( i == MF_CHECKED )
                {
                    i = MF_UNCHECKED;
                    CH341SPIBitSet = FALSE;
                }
                else
                {
                    i = MF_CHECKED;
                    CH341SPIBitSet = TRUE;
                }
                CheckMenuItem( hSysMenu, IDC_SET_SPI, i );
                SetSpiRegConfig( );
                return TRUE;
            }
            else if( wParam == IDC_CMD_HELP )
            {
                MessageBoxCenter( hDialog, CH341DP_HELP_STR, _T(APPNAME), MB_OK | MB_ICONINFORMATION );
                return TRUE;
            }
            return FALSE;
        }

        case WM_COMMAND:
        {
            wmId = LOWORD(wParam);
            wmEvent = HIWORD(wParam);
            switch( wmId )
            {
                case IDC_MCU_MODEL:
                {
                    if( Downhandle == INVALID_HANDLE_VALUE)
                    {
                        McuType = SendDlgItemMessage( hDialog, IDC_MCU_MODEL, CB_GETCURSEL, 0, 0 ); 
                        EnableWindow( GetDlgItem( hDialog, IDC_EPRAM ), 1 );
                        if(McuType >= ATMEL_AT89S51_MODEL)
                        {                                           // AT89S系列没有EEPROM
                            CheckDlgButton( hDialog, IDC_EPRAM, BST_UNCHECKED );
                            EnableWindow( GetDlgItem( hDialog, IDC_EPRAM ), 0 );
                        }
                    }
                }
                return TRUE;

                case IDC_SET_CONFIG:
                {
                    if( Downhandle == INVALID_HANDLE_VALUE)
                    {
                        McuType = SendDlgItemMessage( hDialog, IDC_MCU_MODEL,
                            CB_GETCURSEL, 0, 0 );                   // 得到MCU的类型选择
                        i = IniMcuDown( );
                        if( i == DATA_TRANS_ERR )
                        {
                            SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("The data transfer failed") );
                            return TRUE;
                        }
                        else if( i == NO_MCU_ERR )                  // This MCU is not recognized
                        {
                            SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("This MCU is not recognized") );
                            return TRUE;
                        }
                        else if( i == MCU_MODEL_ERR )                // 此MCU不可编程
                        {
                            SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("Wrong MCU model") );
                            return TRUE;
                        }
                        else if( i == NO_SUPORT_ERR )
                        {
                            SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("This MCU is not supported") );
                            return TRUE;
                        }

                        SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("Complete the identification of the MCU model") );
                        if( ( McuType >AUTO_MCU_MODEL ) && ( McuType < ATMEL_AT89S51_MODEL) )
                        {
                            DialogBox( DownExeHins, (LPCTSTR)IDD_SETCON_DIALOG,
                                hDialog, MegaSetMcuConfig );
                        }
                        else                                        // 否则为AT89S系列
                        {
                            DialogBox( DownExeHins, (LPCTSTR)IDD_AT89SX_SET,
                                hDialog, S51SetMcuConfig );
                        }
                    }
                }
                return TRUE;

                case IDC_RUNING:
                {
                    if( Downhandle == INVALID_HANDLE_VALUE)
                    {
                        SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("Resetting the chip...") );
                        McuType = SendDlgItemMessage( hDialog, IDC_MCU_MODEL,
                            CB_GETCURSEL, 0, 0 );                   // 得到MCU的类型选择
                        i = IniMcuDown( );
                        if( i == DATA_TRANS_ERR )
                        {
                            SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("The data transfer failed") );
                            return TRUE;
                        }
                        else if( i == NO_MCU_ERR )                  // This MCU is not recognized
                        {
                            SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("This MCU is not recognized") );
                            return TRUE;
                        }
                        else if( i == MCU_MODEL_ERR )                // 此MCU不可编程
                        {
                            SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("Wrong MCU model") );
                            return TRUE;
                        }
                        else if( i == NO_SUPORT_ERR )
                        {
                            SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("This MCU is not supported") );
                            return TRUE;
                        }

                        if( (McuType >AUTO_MCU_MODEL ) && (McuType < ATMEL_AT89S51_MODEL) )
                            i = SpiRunMega( Ch341index );
                        else
                            i = SpiRunAt89s( Ch341index );

                        if( i == TRUE )
                            SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("The MCU was successfully reset") );
                        else
                            SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("The data transfer failed") );
                    }
                }
                return TRUE;

                case IDC_BUTTON_BROWER:
                {
                OPENFILENAME mOpenFile;

                    if( Downhandle == INVALID_HANDLE_VALUE)
                    {
                        RtlZeroMemory( &mOpenFile, sizeof( mOpenFile ) );
                        mOpenFile.lStructSize       = sizeof(OPENFILENAME);
                        mOpenFile.hwndOwner         = hDialog;
                        mOpenFile.hInstance         = DownExeHins;
                        mOpenFile.lpstrFilter       = _T("Program files(*.HEX *.BIN)\0*.HEX;*.BIN\0All files(*.*)\0*.*\0");
                        mOpenFile.lpstrFile         = DownFileBuf;
                        mOpenFile.nMaxFile          = sizeof(DownFileBuf);
                        mOpenFile.lpstrTitle        = _T("Select the target program file that will be downloaded");
                        mOpenFile.Flags             = OFN_EXPLORER | OFN_READONLY | OFN_FILEMUSTEXIST;
                        if( GetOpenFileName( &mOpenFile ) )     //打开文件路径收集完成
                        {                                       //提示框中显示
                            SetDlgItemText( hDialog, IDC_STATIC_PROMPT, DownFileBuf );
                            ShowTitleName( );
                            EnableWindow( GetDlgItem( MainDialog, IDC_BUTTON_DOWN ), 1 );
                            FileOpenBit = TRUE;
                        }
                    }
                }
                return TRUE;

                case IDC_EPRAM:
                {
                    if( Downhandle == INVALID_HANDLE_VALUE)
                    {
                        if( IsDlgButtonChecked( hDialog, IDC_EPRAM ) == BST_CHECKED )
                            SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("Download the data to the EEPROM") );
                        else
                            SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("DOWNLOAD THE DATA TO FLASH") );
                    }
                }
                return TRUE;

                case IDC_BUTTON_DOWN:
                {
                    if( Downhandle == INVALID_HANDLE_VALUE)
                    {
                        McuType = SendDlgItemMessage( hDialog, IDC_MCU_MODEL,
                            CB_GETCURSEL, 0, 0 );               // 得到MCU的类型选择

                        IsEepromData = FALSE;
                        if( IsDlgButtonChecked( hDialog, IDC_EPRAM ) == BST_CHECKED )
                            IsEepromData = TRUE;

                        StopBit = FALSE;
                        Downhandle = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)_DownThread,
                            hDialog, 0, &i );
                        if(Downhandle == NULL)
                        {
                            SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("Failed to create a download thread") );
                            Downhandle = INVALID_HANDLE_VALUE;
                        }
                        else
                            CloseHandle( Downhandle );
                    }
                    else //停止Download
                    {
                        StopBit = TRUE;
                        SetDlgItemText( hDialog, IDC_BUTTON_DOWN, _T( "Download" ) );
                    }
                }
                return TRUE;

                case IDCANCEL:
                case IDC_QUIT:
                {
                    SendMessage( hDialog, WM_CLOSE, 0, 0 );
                    return TRUE;
                }
            }
        }
        return FALSE;       // 没有处理的消息返回错误

        case WM_CLOSE:
        {
            if( Downhandle != INVALID_HANDLE_VALUE)
            {
                if(IDNO == MessageBoxCenter( hDialog, _T("Downloading, are you sure to quit?"),
                    _T(APPNAME), MB_YESNO | MB_ICONQUESTION  ) )
                    return TRUE;
            }
            if( Ch341State == TRUE )
                CH341CloseDevice( Ch341index );             // 关闭设备

            #if 1
            StopBit = TRUE;
            SetDlgItemText( hDialog, IDC_STATIC_PROMPT, _T("Wait for Download to stop...") );
            //Sleep( 10 );

            WriteWindowPosReg( );
            McuType = SendDlgItemMessage( hDialog, IDC_MCU_MODEL, CB_GETCURSEL, 0, 0 );
            SetMcuIndexReg( McuType );
            #endif

            EndDialog( hDialog, 0);                         // 关闭对话框,返回0
            return TRUE;
        }

        default:
            return FALSE;
    }
}

/*==============================================================================

函数名: AnalyseCmdLine

函数作用:  分析命令行参数

函数返回 0 正常启动界面， 1命令模式读取成功 2 参数错误则不启动

==============================================================================*/
ULONG  AnalyseCmdLine( LPSTR lpCmdLine )
{
ULONG len;
PVOID p1, p2;
CHAR  *d;
UCHAR CmdBuf[256];
UCHAR FileBuf[256];             // 存放Download文件路径
UCHAR MCUbuf[16];               // 存放MCU名字缓冲区
TCHAR MCUbufW[16];              // 转换后的UNICODE存放缓冲区
BOOL   Fbit;                    // Download文件路径给出标志

    len = strlen( lpCmdLine );
    if( len == 0 )                  // 没有命令输入
        return CMD_NORMAL;
    if( len > sizeof(CmdBuf) - 1  )
        goto PERR;

    McuType = MCU_COUNT;            // 无效的MCU类型
    Fbit = FALSE;
    d = " ";
    memcpy( CmdBuf, lpCmdLine, strlen(lpCmdLine) + 1 );
    strupr( CmdBuf );               // 把小写字母转换成大写

    p1 = strchr( CmdBuf , '\"' );   // 查找",读取Download文件字符传
    if( p1 != NULL )
    {
        p2 = strchr( (PUCHAR)p1 + 1 , '\"' );
        if( p2 != NULL )
        {
            *(PUCHAR)p2 = 0;
            *(PUCHAR)p1 = 0;
            memcpy( FileBuf, (PUCHAR)p1 + 1, strlen((PUCHAR)p1 + 1) + 1 );
            #ifdef  UNICODE
            MultiByteToWideChar( CP_ACP, 0, FileBuf, strlen(FileBuf) + 1,
                &DownFileBuf[0], SIZE(DownFileBuf) );
            #else
            strcpy( DownFileBuf, FileBuf );
            #endif
            FileOpenBit = TRUE;
            Fbit = TRUE;
            //MessageBoxCenterW( NULL, DownFileBuf, _T("调试", MB_OK );
        }
        else
            goto PERR;
    }

    // 把第一个空格设为0
    p1 = strtok( CmdBuf, d );
    if( p1 != NULL )
    {
        len = strlen( p1 );
        if( len < ( sizeof(MCUbuf) - 1 )  )
        {
            memcpy( MCUbuf, p1, strlen(p1) + 1 );
            #ifdef  UNICODE
            MultiByteToWideChar( CP_ACP, 0, MCUbuf, strlen(MCUbuf) + 1,
                MCUbufW, SIZE(MCUbufW) );
            #else
            strcpy( MCUbufW, MCUbuf );
            #endif

            if( strcmp( MCUbufW, _T("MEGA") )  == 0 )
                McuType = 0;
            else
            {
                for( len=1; len<MCU_COUNT; len++ )
                {
                    if( strcmp( McuName[len], MCUbufW ) == 0 )
                    {
                        McuType = len;
                        break;
                    }
                }
            }

            if( McuType == MCU_COUNT )
                goto PERR;
            //MessageBoxCenterW( NULL, MCUbufW, _T("调试", MB_OK );
        }
        else
           goto PERR;
        p1 = strtok( NULL,d );
    }
    else
       goto PERR;

    // 把第二个空格设为0
    if( p1 != NULL )
    {
        len = strlen( p1 );
        if( len == sizeof(McuCmd) )
        {
            McuCmd = MAKEWORD( (*((PUCHAR)p1+1)), (*(PUCHAR)p1) );
            if( McuCmd == WRITE_FLASH_CMD )
            {
                if( Fbit == FALSE )
                    goto PERR;
            }
            else if( McuCmd == WRITE_EEPROM_CMD )
            {
                if( Fbit == FALSE )
                    goto PERR;
                
                if( McuType >= ATMEL_AT89S51_MODEL )    // AT89S没有EEPROM编程为FLASH
                    goto PERR;
            }
            else if( McuCmd == RESET_MCU_CMD )
            {
                if( Fbit == TRUE )
                    goto PERR;
            }
            else
                goto PERR;
        }
        else
            goto PERR;
        p1 = strtok( NULL,d );
    }
    else
        goto PERR;

    if( p1 != NULL )            // 不应该还有空格出现
        goto PERR;

    CmdMode = TRUE;
    return CMD_START;

    PERR:
    MessageBox( NULL, CH341DP_HELP_STR, _T(APPNAME), MB_OK | MB_ICONINFORMATION );
    return CMD_ERROR;
}

/*==============================================================================

函数名: WinMain

函数作用:  主程序入口

==============================================================================*/
INT APIENTRY    WinMain( HINSTANCE hInstance, HINSTANCE hPrevInst,
                        LPSTR lpCmdLine, INT nShowCmd )
{
ULONG i;

    Downhandle = INVALID_HANDLE_VALUE;              // 初始化全局变量
    DownExeHins = hInstance;                        // 保存程序实例句柄
    Ch341index = 0;                                 // 默认从0号设备开始
    McuType = 0;                                    // 默认选择MEGA AUTO
    CmdMode = FALSE;                                // 默认不是命令模式
    FileOpenBit = FALSE;                            // 文件打开标志位
    
    //lpCmdLine = "mega WF \"Video_Module.hex\" ";
    //lpCmdLine = "at89s51 wf \"0.BIN\" ";
    //lpCmdLine = "at89s51 rm ";
    
    i = AnalyseCmdLine( lpCmdLine );
    if( i == CMD_ERROR )
        return 1;
    InitCommonControls( );
    DialogBox( hInstance, (LPCTSTR)IDD_CH341DP_DIALOG, NULL, mDialogMain );
    return 0;
}

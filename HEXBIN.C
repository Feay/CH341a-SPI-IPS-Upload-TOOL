//Coded Date: 2006-6-2 2006-6-21 2007-8-3 (sscanf function causes data overflow)
//Program Functions: Convert HEX files (with extended Intel Hex file format supported) to BIN files
//Compilation Environment: VC6.0
#include    <stdio.h>
#include    <windows.h>

/*==============================================================================

Function name: HexToBin

Function: Convert HEX files to BIN files

*Hexbuf is the HEX file buffer, iHexBufLen is the HEX buffer length,

*Binbuf is the BIN file buffer (not less than 1M), and iBinLen is the effective length of the converted BIN

==============================================================================*/
BOOL HexToBin( PVOID Hexbuf, ULONG iHexBufLen, PVOID Binbuf, PULONG iBinLen )
{
PUCHAR hp;              //Staging Hexbuf pointers
ULONG StartPos;         //Record the section start location
UCHAR DataBuf[256];     //Temporary data storage buffer
USHORT ExtAdr;         // Extended address
USHORT SegAdr;          //Segment address
USHORT OfsAdr;         // Offset address
ULONG WriteAdr;        // Write address

    __try
    {                   // 把1M的BIN文件缓冲区置0XFF
        memset( Binbuf, 0xff, 0xfffff);
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        return FALSE;   // 访问发生异常,转换失败
    }
                        // 检查HEX文件缓冲区是否可读
    if( IsBadReadPtr( Hexbuf, iHexBufLen ) )
        return FALSE;

    StartPos = 0;
    ExtAdr = 0;
    SegAdr = 0;
    hp = Hexbuf;        // 暂存HEX缓冲区开始指针
    *iBinLen = 0;
    while( 1 )
    {
        if( (StartPos + 2) > iHexBufLen )
            return FALSE;
        if( hp[StartPos] == ':' )
        {
        UCHAR i;
        ULONG Len;              // 有效数据长度
        UCHAR CheckSum;         // 校验和
                                                // 得到有效数据长度
            sscanf( hp + StartPos + 1,"%2x", &Len );
            if( (StartPos + Len*2 + 13) > iHexBufLen )
                return FALSE;                   // 防止非法访问内存

            for( CheckSum=(UCHAR)Len, i=0; i < (Len+4); ++i )
            {                                   // 将节数据转换到缓冲区里
                sscanf( hp+StartPos+3+i+i, "%2x", DataBuf+i );
                CheckSum += DataBuf[i];
            }
            if( CheckSum != 0 )                 // 校验和失败,错误的HEX文件
                return FALSE;

            switch ( DataBuf[2] )
            {
                case  0:
                {
                    OfsAdr = DataBuf[0]*256 + DataBuf[1];
                    WriteAdr = ExtAdr*65536 + SegAdr*16 + OfsAdr;
                    for( i=0; i!=Len; i++ )         // 把数据复制到BIN缓冲区里
                    {
                        __try
                        {
                            ((PUCHAR)Binbuf)[WriteAdr + i] = DataBuf[i + 3];
                        }
                        __except( EXCEPTION_EXECUTE_HANDLER )
                        {
                            //MHEX( &WriteAdr, 4, "WriteAdr" );
                            return FALSE;           // 访问发生异常,转换失败
                        }
                    }
                    StartPos += Len*2 + 13;         // 到下一个节的开始

                    if( (WriteAdr + Len) > *iBinLen )
                        *iBinLen = WriteAdr + Len;  // 得到最大结束地址
                }
                break;

                case  2:
                {
                    SegAdr = DataBuf[3]*256 + DataBuf[4];
                    StartPos += 17;
                }
                break;

                case  4:
                {
                    ExtAdr = DataBuf[3]*256 + DataBuf[4];
                    StartPos += 17;
                }
                break;

                case  1:                            // HEX文件结束标志
                    return TRUE;

                default:
                    return FALSE;
            }
        }
        else
        {
            return FALSE;                           // 不可能出现
        }
    }
}
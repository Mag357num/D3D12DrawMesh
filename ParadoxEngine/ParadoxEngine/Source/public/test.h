#pragma once
#include "stdafx.h"

namespace Test
{
#define MAXDUGBUFF 20
#define OutError(ErrorEvent)    {　　\
    　　wchar_t DebugChar[MAXDUGBUFF];    \
   　　 ZeroMemory(DebugChar, MAXDUGBUFF * sizeof(wchar_t));    \
   　　 wsprintfW(DebugChar, L"%s  Error(%d) at line(%d)\n", ErrorEvent, GetLastError(), __LINE__);    \
   　　 OutputDebugStringW(DebugChar);    \
    }

#define MAXNUMBUFF 10
#define OutErrorNum(Number)    {    \
        wchar_t NumberChar[MAXNUMBUFF];    \
        ZeroMemory(NumberChar, MAXNUMBUFF * sizeof(wchar_t));    \
        wsprintfW(NumberChar, L"%d\n", Number);    \
        OutputDebugStringW(NumberChar);    \
    }
}

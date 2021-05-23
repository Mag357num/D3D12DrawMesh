//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>

#include <fstream>

#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <wrl.h>
#include <shellapi.h>
#include <assert.h>

#include "string.h"
#include <cstdint>

#include <cmath>
#include "MathExtend.h"

using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

using std::shared_ptr;
using std::make_shared;
using std::dynamic_pointer_cast;
using std::vector;
using std::array;
using std::string;
using std::wstring;
using std::unordered_map;

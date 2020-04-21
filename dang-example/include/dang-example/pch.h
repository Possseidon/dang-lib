#pragma once

#ifdef ENABLE_VLD
// vld.h checks for either _DEBUG or VLD_FORCE_ENABLE
// this way, ENABLE_VLD can also be used in non-debug builds
#define VLD_FORCE_ENABLE
#include <vld.h>
#endif

#include "dang-gl/pch.h"

#include <iostream>

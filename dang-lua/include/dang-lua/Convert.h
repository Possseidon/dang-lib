#pragma once

// This header simply provides all default convert specializations.
// It is highly recommended to always include all convert specializations at once.
// Adding partial specializations after an instantiation is not allowed.

#include "dang-lua/convert/Boolean.h"
#include "dang-lua/convert/CFunction.h"
#include "dang-lua/convert/Class.h"
#include "dang-lua/convert/Enum.h"
#include "dang-lua/convert/Fail.h"
#include "dang-lua/convert/Integer.h"
#include "dang-lua/convert/Nil.h"
#include "dang-lua/convert/Number.h"
#include "dang-lua/convert/Optional.h"
#include "dang-lua/convert/Reference.h"
#include "dang-lua/convert/State.h"
#include "dang-lua/convert/String.h"
#include "dang-lua/convert/Tuple.h"
#include "dang-lua/convert/Variant.h"

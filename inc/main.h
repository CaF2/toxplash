#pragma once

typedef char* GStrArr;

G_DEFINE_AUTOPTR_CLEANUP_FUNC(GStrArr,g_strfreev)

#include "gen/main.h"

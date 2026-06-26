//
// Copyright 2025 Autodesk
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once
#include "maxtypes.h"
#include "maxusd_banned.h"
#include "plugapi.h"
#include "windows.h"
#include "windowsdefines.h"

#ifdef MAX_2022
// The fmt library bundled in the 3ds Max 2022 devkit's spdlog (fmt 10.x) declares the
// is_char<wchar_t> specialization in <fmt/xchar.h>. Pull it in up-front so the
// specialization is visible before any translation unit instantiates is_char<wchar_t>
// through the primary template (via USD/spdlog headers), which would otherwise raise
// C2908/C2766 ("explicit specialization after instantiation").
#include <spdlog/fmt/bundled/xchar.h>
#endif

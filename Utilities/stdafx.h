/*******************************************************************************
SPDX-License-Identifier: GPL-2.0-or-later
Copyright 2010-2025 Karlsruhe Institute of Technology (KIT)
Contact: stephan.allgeier∂kit.edu,
         Institute of Automation and Applied Informatics

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, 51 Franklin Street,
Fifth Floor, Boston, MA 02110-1301, USA.
*******************************************************************************/



// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently

#pragma once

#ifdef _WIN32
// Windows stuff
// This include could be replaced by the single define
//     #define _WIN32_WINNT 0x0A00
// This should theoretically allow the Utilities project to be built on
// non-Windows systems, but we haven't tried it.
#include <SDKDDKVer.h>
#endif

// standard libraries
#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <codecvt>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

// Boost libraries
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/asio.hpp>
#include <boost/date_time.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/common.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/process/v1/async_system.hpp>
#include <boost/process/v1/child.hpp>
#include <boost/process/v1/io.hpp>
#ifdef _WIN32
#include <boost/process/v1/windows.hpp>
#endif
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>


// other external libraries
#ifdef _WIN32
#include <intrin.h>
#include <ppl.h>
#endif
#include <sqlite3.h>

// alglib project
#include "interpolation.h"
#include "solvers.h"

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



// stdafx.h : Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#pragma once

// Windows stuff
#include <SDKDDKVer.h>

// It would be nice to remove this include eventually. This may not be so easy
// to achieve. At the moment, what prevents the omission of Windows headers is
// the usage of the StlImage class. That class is being used for good reason,
// but in its currentl implementation, it is usable only on Windows, because it
// explicitly tampers with the Windows memory management (specifically, the
// inclusion chain is over Win32HeapRAII.h, heapapi.h and minwindef.h to
// winnt.h). So in this situation, we could either try to make StlImage portable
// (by guarding the Windows-specific parts with ifdefs), or we could potentially
// decide to use an external, open-source image class, or of course we could
// also simply leave it as it is and stick with the Windows-dependency. After
// all, it may turn out that this is not the last problem on the long way to
// true portability.
#include <afxwin.h>         // MFC core and standard components

// Standard library
#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

using std::array;
using std::vector;
using std::shared_ptr;
using std::string;
using std::wstring;
using std::wstring_view;
using std::stringstream;
using std::istringstream;
using std::ostringstream;
using std::wstringstream;
using std::wistringstream;
using std::wostringstream;
using std::fstream;
using std::ifstream;
using std::ofstream;
using std::wfstream;
using std::wifstream;
using std::wofstream;

// Boost libraries
#include <boost/format.hpp>

// Utilities project
#include "ArrayUtilities.h"
#include "FileUtilities.h"
#include "Log.h"
#include "MathTools.h"
#include "StringUtilities.h"

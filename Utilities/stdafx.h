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

// Windows stuff
#include <SDKDDKVer.h>

#include <algorithm>
using std::min;
using std::max;
#include <cassert>
#include <ctime>
#include <map>
#include <memory>
#include <string>

#include <boost/asio.hpp>
#include <boost/format.hpp>

#include <afxwin.h>         // MFC core and standard components

///////////////////////////////////////////////////////////////////////////////
// This section would completely replace the #include <afxwin.h>, so it      //
// would take us a step further towards the complete removal of MFC code in  //
// here. Basically, the most effort would be to completely replace CString,  //
// ideally with std::wstring (or with Win32 API usage, where necessary).     //
//                                                                           //
// If we also wanted to reduce the Win32 API, then all of the file utility   //
// functions now have (often pretty much identical) substitutes in the C++   //
// standard library. ShellExecute() is a bit harder, bit sin't actually used //
// at all in HIT.                                                            //
//                                                                           //
// In theory, the open source HIT does not really rely on a Windows          //
// environment, but making it completely Windows.independent would be a  lot //
// of effort, removing all Win32 types and functions. This does not really   //
// seem feasible.                                                            //
//                                                                           //
//#include <windows.h>
//
//// This is required for
//// - PathIsDirectory()
//// - PathRelativePathTo()
//// - PathIsRelative()
//// - PathRemoveFileSpec()
//// - PathAppend()
//// all of which are used in CFileUtilities methods
//#include <shlwapi.h>
//
//// This is required for
//// - ShellExecute()
//// which is used only once, in CUtilities::RestartWithAdminRights().
//#include <shellapi.h>
//
//// This is required because of the usage of CRect, which is only used in
//// CUtilities::ValidateROI(), which in turn is unused. We could simply remove
//// the method.
//#include <atltypes.h>
//
//// This is required because of the usage of CString. This would be some work to
//// remove from the code, because it is used relatively frequently.
//#include <atlstr.h>
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////
// This single include is the only remainder of the previous                 //
//     #include <afxwin.h>         // MFC core and standard components       //
//                                                                           //
// We are now very close to having the Utilities pretty much entirely        //
// independent of the underlying OS.                                         //
//                                                                           //
// In theory, the open source HIT does not really rely on a Windows          //
// environment, but making it completely Windows.independent would be a  lot //
// of effort, removing all Win32 types and functions. This does not really   //
// seem feasible.                                                            //
//                                                                           //
//
//// This is required for
//// - PathIsDirectory()
//// - PathRelativePathTo()
//// - PathIsRelative()
//// - PathRemoveFileSpec()
//// - PathAppend()
//// all of which are used in CFileUtilities methods
#include <shlwapi.h>
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

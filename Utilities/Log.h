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



#pragma once
#include <string>
#include <string_view>
/**	\brief Die Klasse CLog implementiert ein Applikationsprotokoll mit
 *	Dateiausgabe.
 *
 *	Die öffentlichen Methoden dieser Klasse sind alle statisch, so dass
 *	applikationsweit auf das Protokoll zugegriffen werden kann.
 *
 *	\htmlonly
 *		Die Klasse CLog setzt zur Protokollierung auf der freien BOOST auf.
 *	\endhtmlonly
 */
class  CLog
{

public:

	enum ELogLevel
	{
		eEmergency,		// = 0
		eAlert,			// = 1
		eCritical,		// = 2
		eError,			// = 3
		eWarning,		// = 4
		eNotice,		// = 5
		eInformational,	// = 6
		eDebug			// = 7
	};

public:

	static void InitLogging(const std::wstring_view sFilePath);
	//Do not change the const LPCTSTR to CString or a reference without reading the Comment at CStringUtilities::FormatString
	static void Log(ELogLevel nLogLevel, const LPCTSTR sModule, const LPCTSTR sMessage, ...);
	static void SetSeverityCeiling(ELogLevel ceiling);
	static std::wstring SeverityToString(ELogLevel nLogLevel);
};


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



#include "stdafx.h"
#include "Utilities.h"

#include <cstdio>

#include <boost/asio.hpp>
#include <boost/process/v1/child.hpp>
#include <boost/process/v1/io.hpp>
#include <boost/process/v1/windows.hpp>
#include <boost/process/v1/async_system.hpp>

#include "StringUtilities.h"



bool CUtilities::IsCurrentProcessElevated()
{
#ifdef _WIN32
	HANDLE hToken;
	OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hToken);

	//This only works with UAC on. With UAC off this returns TokenElevationTypeLimited, even we have full rights.
	DWORD infoLen;
	//TOKEN_ELEVATION_TYPE elevationType;
	//GetTokenInformation(hToken, TokenElevationType,&elevationType, sizeof(elevationType), &infoLen);

	//This works with UAC off, as it returns a value >0 if we run as admin or uac is off
	TOKEN_ELEVATION elevation;
	GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &infoLen);
	CloseHandle(hToken);

	return elevation.TokenIsElevated;
#else
	return geteuid() == 0;
#endif
}

std::wstring CUtilities::Exec(std::wstring command)
{
	boost::asio::io_context ios;

	std::future<std::string> data;

	boost::process::v1::child c(command, boost::process::v1::std_in.close(), (boost::process::v1::std_out & boost::process::v1::std_err) > data, ios, ::boost::process::v1::windows::hide);

	ios.run();

	return CStringUtilities::ConvertToStdWstring(data.get());

	//std::shared_ptr<FILE> pipe(_wpopen(cmd.c_str(), L"r"), _pclose);
	//if (!pipe) throw new std::exception("Cant open pipe");
	//wchar_t buffer[128];
	//std::wstring result = L"";
	//while (!feof(pipe.get())) {
	//	if (fgetws(buffer, 128, pipe.get()) != NULL)
	//		result += buffer;
	//}
	//return result;
}

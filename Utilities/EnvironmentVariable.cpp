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
#include "EnvironmentVariable.h"


std::wstring CEnvironmentVariable::Get(std::wstring name)
{
	const size_t size = 10000;
	std::wstring buffer;
	buffer.resize(size);
	GetEnvironmentVariable(name.c_str(), buffer.data(), size);
	return buffer;
}
std::wstring CEnvironmentVariable::GetUnitTestDataPath()
{
	HMODULE h;
	static TCHAR c;
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, &c, &h);
	TCHAR p[_MAX_PATH + 1];
	GetModuleFileName(h, p, _MAX_PATH + 1);
	std::wstring path(p);
	path = path.substr(0, path.find_last_of(L"\\"));
	path = path.substr(0, path.find_last_of(L"\\"));
	path += L"\\UnitTestData";
	return path;
}
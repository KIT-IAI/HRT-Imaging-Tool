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

#include "StringUtilities.h"

class SettingsStore
{
private:
	SettingsStore(std::wstring product);
	~SettingsStore();

	class Impl;
	std::shared_ptr<Impl> pimpl;

	static SettingsStore* m_cInstance;

	bool ReadValueString(std::wstring path, std::wstring name, std::wstring& value);
	bool WriteValueCurrentUserString(std::wstring path, std::wstring name, std::wstring value);
	bool WriteValueLocalMachineString(std::wstring path, std::wstring name, std::wstring value);

public:
	static SettingsStore* GetInstance(std::wstring product);

	//This can only be called if at least one call with the product name has already been done!
	static SettingsStore* GetInstance();
	static void DestroyInstance();

	template<typename T> bool ReadValue(std::wstring path, std::wstring name, T& value);
	template<typename T> bool ReadValue(std::wstring path, std::wstring name, T& value, T default_value);
	template<typename T> bool WriteValueCurrentUser(std::wstring path, std::wstring name, T value);
	template<typename T> bool WriteValueLocalMachine(std::wstring path, std::wstring name, T value);

	bool DeleteValue(std::wstring path, std::wstring name);

	static bool LocalMachineWriteable();

	void MakeEverythingSystemDefault();
};

template<typename T>
inline bool SettingsStore::ReadValue(std::wstring path, std::wstring name, T& value)
{
	std::wstring svalue;
	bool ret = ReadValueString(path, name, svalue);
	if (!ret) return false;
	return CStringUtilities::ParseString(svalue, value);
}
template<typename T>
inline bool SettingsStore::ReadValue(std::wstring path, std::wstring name, T& value, T default_value)
{
	std::wstring svalue;
	bool ret = ReadValueString(path, name, svalue);
	if (!ret)
	{
		value = default_value;
		return true;
	}
	if (!CStringUtilities::ParseString(svalue, value))
	{
		value = default_value;
	}
	return true;
}

template<typename T>
inline bool SettingsStore::WriteValueCurrentUser(std::wstring path, std::wstring name, T value)
{
	return WriteValueCurrentUserString(path, name, CStringUtilities::ToString(value));
}

template<typename T>
inline bool SettingsStore::WriteValueLocalMachine(std::wstring path, std::wstring name, T value)
{
	return WriteValueLocalMachineString(path, name, CStringUtilities::ToString(value));
}
SettingsStore* GetGSS();

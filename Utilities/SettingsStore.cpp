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
#include "SettingsStore.h"
#include "FileUtilities.h"
#include "StringUtilities.h"
#include "Utilities.h"
#include <sqlite3.h>


SettingsStore* SettingsStore::m_cInstance = nullptr;

SettingsStore* GetGSS()
{
	return SettingsStore::GetInstance();
}

class SettingsStore::Impl
{
public:
	std::string UserPath;
	std::string MachinePath;
	sqlite3* UserDB;
	sqlite3* MachineDB;
public:
	Impl(std::wstring product) :
		UserDB(nullptr)
		, MachineDB(nullptr)
	{
		std::string configFileName = CStringUtilities::ConvertToStdString(product) + ".sqlite";

#ifdef _WIN32
		auto userConfigDir = std::filesystem::path(*CStringUtilities::getenv("AppData")) / "kit-iai";
		auto systemConfigDir = std::filesystem::path(*CStringUtilities::getenv("ProgramData")) / "kit-iai";
#else  // #ifdef _WIN32
		auto userConfigDir = std::filesystem::path("~/.config") / "kit-iai";
		auto systemConfigDir = std::filesystem::path("/etc") / "kit-iai";
#endif // #ifdef _WIN32

		if (!CFileUtilities::PathExists(userConfigDir))
		{
			CFileUtilities::MakeDirectory(userConfigDir);
		}
		UserPath = userConfigDir / configFileName;
		sqlite3_open_v2(UserPath.c_str(), &UserDB, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);

		if (LocalMachineWriteable() && !CFileUtilities::PathExists(systemConfigDir))
		{
			CFileUtilities::MakeDirectory(systemConfigDir);
		}
		MachinePath = systemConfigDir / configFileName;
		if (LocalMachineWriteable())
		{
			sqlite3_open_v2(MachinePath.c_str(), &MachineDB, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
		}
		else if (CFileUtilities::FileExists(MachinePath))
		{
			sqlite3_open_v2(MachinePath.c_str(), &MachineDB, SQLITE_OPEN_READONLY, nullptr);
		}
		else
		{
			MachinePath = "";
		}

		InitDB(UserDB);
		InitDB(MachineDB);
	}

	~Impl()
	{
		sqlite3_close(UserDB);
		if (MachineDB) sqlite3_close(MachineDB);
	}
	void InitDB(sqlite3* db)
	{
		if (!db) return;
		sqlite3_exec(db, "PRAGMA journal_mode = WAL", nullptr, nullptr, nullptr);
		sqlite3_exec(db, "PRAGMA synchronous = NORMAL", nullptr, nullptr, nullptr);
		sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS parameter (param_id INTEGER PRIMARY KEY AUTOINCREMENT, path TEXT NOT NULL, name TEXT NOT NULL, value TEXT NOT NULL);", nullptr, nullptr, nullptr);
		//Create Tables if not exist
	}
	bool DeleteValue(std::wstring path, std::wstring name, sqlite3* db)
	{
		sqlite3_stmt* stmt;
		sqlite3_prepare_v2(db, "DELETE FROM parameter WHERE path=? AND name=?", -1, &stmt, nullptr);
		sqlite3_bind_text(stmt, 1, CStringUtilities::ConvertToStdString(path).c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 2, CStringUtilities::ConvertToStdString(name).c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_step(stmt);
		sqlite3_reset(stmt);
		sqlite3_finalize(stmt);
		return true;
	}
};


SettingsStore::SettingsStore(std::wstring product) : pimpl(new SettingsStore::Impl(product))
{
}


SettingsStore::~SettingsStore()
{

}

bool SettingsStore::ReadValueString(std::wstring path, std::wstring name, std::wstring& value)
{
	bool ret;
	sqlite3_stmt* stmt;
	sqlite3_prepare_v2(pimpl->UserDB, "SELECT value FROM  parameter WHERE path=? AND name=? ORDER BY param_id DESC", -1, &stmt, nullptr);
	sqlite3_bind_text(stmt, 1, CStringUtilities::ConvertToStdString(path).c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, CStringUtilities::ConvertToStdString(name).c_str(), -1, SQLITE_TRANSIENT);
	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		value = CStringUtilities::ConvertToStdWstring(std::string((char*)sqlite3_column_text(stmt, 0)));
		ret = true;
	}
	else
	{
		if (pimpl->MachineDB)
		{
			sqlite3_reset(stmt);
			sqlite3_finalize(stmt);
			sqlite3_prepare_v2(pimpl->MachineDB, "SELECT value FROM  parameter WHERE path=? AND name=? ORDER BY param_id DESC", -1, &stmt, nullptr);
			sqlite3_bind_text(stmt, 1, CStringUtilities::ConvertToStdString(path).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, CStringUtilities::ConvertToStdString(name).c_str(), -1, SQLITE_TRANSIENT);
			if (sqlite3_step(stmt) == SQLITE_ROW)
			{
				value = CStringUtilities::ConvertToStdWstring(std::string((char*)sqlite3_column_text(stmt, 0)));
				ret = true;
			}
			else
			{
				ret = false;
			}
		}
		else
		{
			ret = false;
		}
	}
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
	return ret;
}

bool SettingsStore::WriteValueCurrentUserString(std::wstring path, std::wstring name, std::wstring value)
{
	pimpl->DeleteValue(path, name, pimpl->UserDB);
	sqlite3_stmt* stmt;
	sqlite3_prepare_v2(pimpl->UserDB, "INSERT INTO parameter (path, name, value) VALUES (?,?,?)", -1, &stmt, nullptr);
	sqlite3_bind_text(stmt, 1, CStringUtilities::ConvertToStdString(path).c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, CStringUtilities::ConvertToStdString(name).c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, CStringUtilities::ConvertToStdString(value).c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_step(stmt);
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
	return true;
}

bool SettingsStore::WriteValueLocalMachineString(std::wstring path, std::wstring name, std::wstring value)
{
	if (!pimpl->MachineDB) return WriteValueCurrentUserString(path, name, value);
	if (!LocalMachineWriteable()) return WriteValueCurrentUserString(path, name, value);
	pimpl->DeleteValue(path, name, pimpl->MachineDB);
	sqlite3_stmt* stmt;
	sqlite3_prepare_v2(pimpl->MachineDB, "INSERT INTO parameter (path, name, value) VALUES (?,?,?)", -1, &stmt, nullptr);
	sqlite3_bind_text(stmt, 1, CStringUtilities::ConvertToStdString(path).c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, CStringUtilities::ConvertToStdString(name).c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, CStringUtilities::ConvertToStdString(value).c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_step(stmt);
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);
	return true;
}

bool SettingsStore::DeleteValue(std::wstring path, std::wstring name)
{
	std::wstring value;
	if (!pimpl->DeleteValue(path, name, pimpl->UserDB)) return false;
	if (pimpl->MachineDB && LocalMachineWriteable())
	{
		if (!pimpl->DeleteValue(path, name, pimpl->MachineDB)) return false;
	}
	else if (pimpl->MachineDB && ReadValueString(path, name, value))
	{
		return false;
	}
	return true;

}

SettingsStore* SettingsStore::GetInstance(std::wstring product)
{
	if (m_cInstance == nullptr)
	{
		m_cInstance = new SettingsStore(product);
	}
	return m_cInstance;
}

SettingsStore* SettingsStore::GetInstance()
{
	return m_cInstance;
}
void SettingsStore::DestroyInstance()
{
	delete m_cInstance;
	m_cInstance = nullptr;
}

bool SettingsStore::LocalMachineWriteable()
{
	return CUtilities::IsCurrentProcessElevated();
}

void SettingsStore::MakeEverythingSystemDefault()
{
	if (!LocalMachineWriteable()) return;

	sqlite3_stmt* stmt;
	sqlite3_prepare_v2(pimpl->UserDB, "SELECT path, name, value FROM  parameter ORDER BY param_id ASC", -1, &stmt, nullptr);
	while (sqlite3_step(stmt) == SQLITE_ROW)
	{

		std::wstring path = CStringUtilities::ConvertToStdWstring(std::string((char*)sqlite3_column_text(stmt, 0)));
		std::wstring name = CStringUtilities::ConvertToStdWstring(std::string((char*)sqlite3_column_text(stmt, 1)));
		std::wstring value = CStringUtilities::ConvertToStdWstring(std::string((char*)sqlite3_column_text(stmt, 2)));
		WriteValueLocalMachineString(path, name, value);
	}
	sqlite3_reset(stmt);
	sqlite3_finalize(stmt);

}

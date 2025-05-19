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
#include <vector>
#include "SQLiteTable.h"

struct sqlite3;
struct sqlite3_stmt;


class CSQLiteDatabase
{
public:
	CSQLiteDatabase() = default;
	explicit CSQLiteDatabase(const std::wstring& sFilePath);
	CSQLiteDatabase(const CSQLiteDatabase&) = delete;
	CSQLiteDatabase(CSQLiteDatabase&& toMove);
	virtual ~CSQLiteDatabase();

	void Open(const std::wstring& sFilePath);
	void Close();

	std::wstring GetName() const;
	std::wstring GetPath() const;

	void CreateTable(std::wstring sTableName, std::vector<std::wstring> Columns) const;
	void CreateTableIfNotExists(std::wstring sTableName, std::vector<std::wstring> Columns) const;
	void DropTable(std::wstring sTableName) const;
	void DropTableIfExists(std::wstring sTableName) const;
	bool TableExists(std::wstring sTableName) const;

	void ClearTable(std::wstring sTableName) const;

	void Execute(std::wstring sQuery) const;
	CSQLiteTable Query(std::wstring sQuery) const;

	CSQLiteTableRow SelectFirst(std::wstring table, std::vector<std::wstring> columns = { L"*" }, std::wstring condition = L"", std::wstring orderBy = L"") const;
	CSQLiteTable SelectAll(std::wstring table, std::vector<std::wstring> columns = { L"*" }, std::wstring condition = L"", std::wstring orderBy = L"") const;
	CSQLiteTable SelectDistinct(std::wstring table, std::vector<std::wstring> columns = { L"*" }, std::wstring condition = L"", std::wstring orderBy = L"") const;

	void InsertInto(std::wstring sTable, std::vector<std::pair<std::wstring, std::wstring>> Row) const;
	void InsertIntoBatch(std::wstring sTable, const std::vector<std::vector<std::pair<std::wstring, std::wstring>>>& Rows);
	void InsertInto(std::wstring sTable, const CSQLiteTableRow& Row) const;
	void InsertIntoBatch(std::wstring sTable, const std::vector<CSQLiteTableRow>& Rows);

	void InsertTable(const CSQLiteTable& Table);

	void BeginTransaction() const;
	void EndTransaction() const;

private:
	static void BindParameter(sqlite3_stmt* Statement, size_t index, std::wstring value);
	std::wstring GenerateSelectStatement(std::vector<std::wstring> columns, std::wstring table, std::wstring condition = L"", std::wstring orderBy = L"", size_t limit = 0) const;
	static std::wstring GenerateInsertIntoWildcardStatement(std::wstring sTable, std::vector<std::pair<std::wstring, std::wstring>> Values);
	sqlite3_stmt* PrepareStatement(const std::wstring& Statement) const;
	static std::wstring GetCommaSeparated(std::vector<std::wstring>);

	static void BindAndSend(sqlite3_stmt* PreparedStatement, const CSQLiteTableRow& row);

private:
	bool m_bIsOpen = false;
	std::wstring m_sDatabaseName = L"";
	std::wstring m_sOpenFileName = L"";
	sqlite3* m_pConnection = nullptr;
};
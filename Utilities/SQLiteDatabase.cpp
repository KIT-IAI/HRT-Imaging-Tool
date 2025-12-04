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
#include "SQLiteDatabase.h"

#include "FileUtilities.h"
#include "SQLiteTable.h"
#include "StringUtilities.h"

#pragma comment(lib,"sqlite3.lib")



CSQLiteDatabase::~CSQLiteDatabase()
{
	if (m_bIsOpen)
		Close();
}

CSQLiteDatabase::CSQLiteDatabase(const std::wstring& sFilePath)
{
	Open(sFilePath);
}

CSQLiteDatabase::CSQLiteDatabase(CSQLiteDatabase&& toMove)
	:CSQLiteDatabase(toMove.m_sOpenFileName)
{
}

/*
Returns the name of the database. This is usually the name of the opened database file or empty if no database is open.
*/
std::wstring CSQLiteDatabase::GetName() const
{
	return m_sDatabaseName;
}

/*
Returns the path to the database-file. It is empty if no database is open.
*/
std::wstring CSQLiteDatabase::GetPath() const
{
	return m_sOpenFileName;
}

/*
Opens a new database connection.
Does nothing if a connection is alread open.
Creates the database-file if it does not exist.
*/
void CSQLiteDatabase::Open(const std::wstring& sFilePath)
{
	if (m_bIsOpen)
		return;

	auto unicodePath = CStringUtilities::ConvertToUnicode(sFilePath);
	int sqliteError = sqlite3_open(unicodePath.c_str(), &m_pConnection);

	if (sqliteError)
		throw CSQLException(L"Failed to open sqlite database " + GetName());

	sqliteError = sqlite3_exec(m_pConnection, "PRAGMA journal_mode = DELETE", nullptr, nullptr, nullptr);
	if (sqliteError)
		throw CSQLException(L"Failed to open sqlite database " + GetName());
	sqliteError = sqlite3_exec(m_pConnection, "PRAGMA synchronous = NORMAL", nullptr, nullptr, nullptr);
	if (sqliteError)
		throw CSQLException(L"Failed to open sqlite database " + GetName());

	m_bIsOpen = true;
	m_sDatabaseName = CFileUtilities::GetFileName(sFilePath);
	m_sOpenFileName = sFilePath;
}

void CSQLiteDatabase::Close()
{
	if (!m_bIsOpen)
		throw CSQLException(L"Attempting to close not open database connection.");

	int sqliteError = SQLITE_OK;

	sqlite3_close(m_pConnection);

	if (sqliteError)
		throw CSQLException(L"Failed to close database " + GetName());

	m_sDatabaseName = L"";
	m_sOpenFileName = L"";
	m_bIsOpen = false;
}

void CSQLiteDatabase::CreateTable(std::wstring sTableName, std::vector<std::wstring> Columns) const
{
	std::wstring query = L"CREATE TABLE " + sTableName + L" (" + GetCommaSeparated(Columns) + L")";

	Execute(query);
}
void CSQLiteDatabase::CreateTableIfNotExists(std::wstring sTableName, std::vector<std::wstring> Columns) const
{
	std::wstring query = L"CREATE TABLE IF NOT EXISTS " + sTableName + L" (" + GetCommaSeparated(Columns) + L")";

	Execute(query);
}

void CSQLiteDatabase::DropTable(std::wstring sTableName) const
{
	std::wstring query = L"DROP TABLE " + sTableName;
	Execute(query);
}

void CSQLiteDatabase::DropTableIfExists(std::wstring sTableName) const
{
	std::wstring query = L"DROP TABLE IF EXISTS " + sTableName;
	Execute(query);
}


void CSQLiteDatabase::ClearTable(std::wstring sTableName) const
{
	std::wstring query = L"DELETE FROM TABLE " + sTableName;
	Execute(query);
}

void CSQLiteDatabase::Execute(std::wstring sQuery) const
{
	if (!m_bIsOpen)
		throw CSQLException(L"Database " + GetName() + L" is not open.");

	int sqlError = sqlite3_exec(m_pConnection, CStringUtilities::ConvertToStdString(sQuery).c_str(), nullptr, nullptr, nullptr);

	if (sqlError)
		throw CSQLException(L"SQlite query: '" + sQuery + L"' Failed with errorcode: " + std::to_wstring(sqlError));
}

CSQLiteTable CSQLiteDatabase::Query(std::wstring sQuery) const
{
	auto pStatement = PrepareStatement(sQuery);
	CSQLiteTable ResultTable(L"Result");

	int cols = sqlite3_column_count(pStatement);
	int resultCode = SQLITE_ROW;

	while (resultCode == SQLITE_ROW)
	{
		resultCode = sqlite3_step(pStatement);


		CSQLiteTableRow row;
		bool bIsRowEmpty = true;
		for (int col = 0; col < cols; col++)
		{
			auto colValue = (char*)sqlite3_column_text(pStatement, col);
			auto colName = (char*)sqlite3_column_name(pStatement, col);

			std::string sColValue;
			if (colValue == nullptr)
			{
				sColValue = std::string("");
			}
			else
			{
				sColValue = std::string(colValue);
				bIsRowEmpty = false;
			}

			auto KeyString = CStringUtilities::ConvertToStdWstring(std::string(colName));
			auto ValueString = CStringUtilities::ConvertToStdWstring(sColValue);

			row.AddKeyValuePair(KeyString, ValueString);
		}

		if (!bIsRowEmpty)
			ResultTable.AddRow(row);
	}
	sqlite3_finalize(pStatement);

	return ResultTable;
}

CSQLiteTableRow CSQLiteDatabase::SelectFirst(std::wstring table, std::vector<std::wstring> columns, std::wstring condition, std::wstring orderBy) const
{
	if (!TableExists(table))
		throw CSQLException(L"No such table:" + table);

	std::wstring sQuery = GenerateSelectStatement(columns, table, condition, orderBy, 1);

	auto Results = Query(sQuery);

	if (Results.size() == 0)
		return CSQLiteTableRow();

	assert(Results.size() == 1);

	return Results[0];
}

CSQLiteTable CSQLiteDatabase::SelectAll(std::wstring table, std::vector<std::wstring> columns, std::wstring condition, std::wstring orderBy) const
{
	if (!TableExists(table))
		throw CSQLException(L"No such table:" + table);

	std::wstring sQuery = GenerateSelectStatement(columns, table, condition, orderBy);

	return Query(sQuery);
}
CSQLiteTable CSQLiteDatabase::SelectDistinct(std::wstring table, std::vector<std::wstring> columns, std::wstring condition, std::wstring orderBy) const
{
	if (!TableExists(table))
		throw CSQLException(L"No such table:" + table);

	std::wstring sQuery = GenerateSelectStatement(columns, table, condition, orderBy);
	sQuery = CStringUtilities::ReplaceFirst(sQuery, L"SELECT ", L"SELECT DISTINCT ");
	return Query(sQuery);
}

bool CSQLiteDatabase::TableExists(std::wstring sTableName) const
{
	std::wstring sQuery = L"SELECT name FROM sqlite_master WHERE type='table' AND name='" + sTableName + L"'";
	auto Results = Query(sQuery);

	assert(Results.size() < 2);

	return Results.size() == 1;
}

void CSQLiteDatabase::InsertTable(const CSQLiteTable& Table)
{
	Table.InsertIntoDatabase(*this);
}

/*
Inserts a new row into the specified table.
\param[in] Pairs Contains a vector of key-value-pairs to insert into the table
*/
void CSQLiteDatabase::InsertInto(std::wstring sTable, std::vector<std::pair<std::wstring, std::wstring>> Pairs) const
{
	std::wstring Statement = GenerateInsertIntoWildcardStatement(sTable, Pairs);
	auto PreparedStatement = PrepareStatement(Statement);

	CSQLiteTableRow Row = CSQLiteTableRow::From(Pairs);

	BindAndSend(PreparedStatement, Row);
	sqlite3_finalize(PreparedStatement);
}
/*
Inserts multiple rows into the specified table. This method is significantly faster than inserting rows one by one.
\param[in] Rows Contains a vector (for each row) of vectors of key-value-pairs to insert into the table
*/
void CSQLiteDatabase::InsertIntoBatch(std::wstring sTable, const std::vector<std::vector<std::pair<std::wstring, std::wstring>>>& Rows)
{
	if (Rows.size() == 0)
		return;
	// All rows have the same number of Key-Value-Pairs
	assert(std::all_of(Rows.begin(), Rows.end(), [&](const auto& Row) {return Row.size() == Rows[0].size(); }));

	std::wstring Statement = GenerateInsertIntoWildcardStatement(sTable, Rows[0]);

	auto PreparedStatement = PrepareStatement(Statement);

	BeginTransaction();
	for (const auto& Row : Rows)
	{
		BindAndSend(PreparedStatement, CSQLiteTableRow::From(Row));
	}
	sqlite3_finalize(PreparedStatement);
	EndTransaction();
}

/*
When using this method you have to make sure CSQLiteTableRow contains as many values as the table has columns.
*/
void CSQLiteDatabase::InsertInto(std::wstring sTable, const CSQLiteTableRow& Row) const
{
	if (Row.size() == 0)
		return;
	auto Statement = Row.GenerateWildcardStatement(sTable);
	auto PreparedStatement = PrepareStatement(Statement);
	BindAndSend(PreparedStatement, Row);
	sqlite3_finalize(PreparedStatement);
}
/*
When using this method you have to make sure CSQLiteTableRow contains as many values as the table has columns.
*/
void CSQLiteDatabase::InsertIntoBatch(std::wstring sTable, const std::vector<CSQLiteTableRow>& Rows)
{
	if (Rows.size() == 0)
		return;
	// All rows have the same number of Values
	assert(std::all_of(Rows.begin(), Rows.end(), [&](const auto& Row) {return Row.size() == Rows[0].size(); }));

	auto Statement = Rows[0].GenerateWildcardStatement(sTable);
	auto PreparedStatement = PrepareStatement(Statement);

	BeginTransaction();

	for (const auto& Row : Rows)
	{
		BindAndSend(PreparedStatement, Row);
	}
	sqlite3_finalize(PreparedStatement);
	EndTransaction();
}
void CSQLiteDatabase::BindAndSend(sqlite3_stmt* PreparedStatement, const CSQLiteTableRow& Row)
{
	size_t i = 1;
	for (const auto& Value : Row)
	{
		BindParameter(PreparedStatement, i++, Value);
	}
	int resultCode = sqlite3_step(PreparedStatement);

	if (resultCode != SQLITE_DONE)
		throw CSQLException(L"Failed to insert element into database. Errorcode: " + resultCode);

	sqlite3_reset(PreparedStatement);
	sqlite3_clear_bindings(PreparedStatement);
}


/*
* Begins a SQL-transaction. Definition of transaction: http://www.tutorialspoint.com/sql/sql-transactions.htm
* Short version: Either all statements in a transaction get executed, or none.
* Additionally it is much faster, especially when doing a batch-statement.
*/
void CSQLiteDatabase::BeginTransaction() const
{
	int resultCode = sqlite3_exec(m_pConnection, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
	if (resultCode != SQLITE_OK)
		throw CSQLException(L"Failed to begin transaction. Errorcode: " + resultCode);
}
void CSQLiteDatabase::EndTransaction() const
{
	int resultCode = sqlite3_exec(m_pConnection, "END TRANSACTION", nullptr, nullptr, nullptr);
	if (resultCode != SQLITE_OK)
		throw CSQLException(L"Failed to commit transaction. Errorcode: " + resultCode);
}


void CSQLiteDatabase::BindParameter(sqlite3_stmt* Statement, size_t index, std::wstring value)
{
	if (value == L"NULL")
		sqlite3_bind_text(Statement, static_cast<int>(index), nullptr, -1, SQLITE_TRANSIENT);
	else
		sqlite3_bind_text(Statement, static_cast<int>(index), CStringUtilities::ConvertToStdString(value).c_str(), -1, SQLITE_TRANSIENT);
}

std::wstring CSQLiteDatabase::GenerateSelectStatement(std::vector<std::wstring> columns, std::wstring table, std::wstring condition, std::wstring orderBy, size_t limit) const
{
	std::wstring sQuery = L"SELECT " + GetCommaSeparated(columns);

	sQuery += L" FROM " + table;

	if (!condition.empty())
		sQuery += L" WHERE " + condition;

	if (!orderBy.empty())
		sQuery += L" ORDER BY " + orderBy;

	if (limit == 0)
		return sQuery;

	sQuery += L" LIMIT " + std::to_wstring(limit);

	return sQuery;
}
std::wstring CSQLiteDatabase::GenerateInsertIntoWildcardStatement(std::wstring sTable, std::vector<std::pair<std::wstring, std::wstring>> Pairs)
{
	std::wstring Statement = L"INSERT INTO " + sTable + L" (";

	std::vector<std::wstring> Columns;
	std::vector<std::wstring> Values;
	std::vector<std::wstring> QuestionMarks;
	for (const auto& pair : Pairs)
	{
		Columns.push_back(pair.first);
		Values.push_back(pair.second);
		QuestionMarks.push_back(L"?");
	}

	Statement += GetCommaSeparated(Columns) + L") VALUES (" + GetCommaSeparated(QuestionMarks) + L")";
	return Statement;
}
sqlite3_stmt* CSQLiteDatabase::PrepareStatement(const std::wstring& Statement) const
{
	if (!m_bIsOpen)
		throw CSQLException(L"Database connection not open.");

	sqlite3_stmt* sqlStatement = nullptr;
	auto stringStatement = CStringUtilities::ConvertToStdString(Statement);
	int sqliteError = sqlite3_prepare_v2(m_pConnection, stringStatement.c_str(), -1, &sqlStatement, nullptr);

	if (sqliteError)
	{
		auto sErrorMessage = sqlite3_errmsg(m_pConnection);
		sqlite3_finalize(sqlStatement);
		throw CSQLException(L"Error preparing statement. Error message: " + CStringUtilities::ConvertToStdWstring(std::string(sErrorMessage)));
	}
	return sqlStatement;
}

std::wstring CSQLiteDatabase::GetCommaSeparated(std::vector<std::wstring> in)
{
	std::wstring out;

	for (const auto& s : in)
		out += s + L",";
	out = out.substr(0, out.size() - 1);

	return out;
}

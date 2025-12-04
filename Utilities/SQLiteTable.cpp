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
#include "SQLiteTable.h"

#include "SQLiteDatabase.h"

using namespace std;



const std::wstring CSQLiteTable::DefaultColumnName = L"col";

CSQLiteTable::CSQLiteTable(std::wstring Name)
	: m_TableName(Name)
{
}

CSQLiteTable CSQLiteTable::From(const std::wstring& Name, const std::vector<CSQLiteTableRow>& Rows)
{
	CSQLiteTable Table(Name);

	for (const auto& Row : Rows)
		Table.AddRow(Row);

	return Table;
}
CSQLiteTable CSQLiteTable::From(const wstring& Name, const vector<wstring>& Values)
{
	CSQLiteTable Table(Name);

	Table.AddColumn(L"Value", L"TEXT");

	for (const auto& Val : Values)
		Table.AddRow(CSQLiteTableRow::From(vector<wstring>{Val}));

	return Table;
}
CSQLiteTable CSQLiteTable::From(const std::wstring& Name, const CDenseMatrix& Matrix)
{
	CSQLiteTable Table(Name);

	for (const auto& Row : Matrix)
		Table.AddRow(CSQLiteTableRow::From(Row));

	for (size_t col = 0; col < Matrix.Cols(); col++)
		Table.AddColumn(GetDefaultColumnName(col), L"REAL");

	return Table;
}
template <> CDenseMatrix CSQLiteTable::Convert() const
{
	if (size() == 0)
		return CDenseMatrix();

	if (!std::all_of(begin(), end(), [this](const CSQLiteTableRow& Row) {return Row.size() == GetRow(0).size(); }))
		throw std::runtime_error("Non-uniform table cannot be converted into a matrix.");

	CDenseMatrix Matrix(size(), GetRow(0).size());

	size_t i = 0;
	for (const auto& TableRow : *this)
		Matrix[i++] = TableRow.Convert<CDenseVector>();

	return Matrix;
}
template <> vector<wstring> CSQLiteTable::Convert() const
{
	if (size() == 0)
		return{};

	vector<wstring> Result;

	for (const auto& TableRow : *this)
		Result.push_back(TableRow.Get<wstring>(0));

	return Result;
}



void CSQLiteTable::AddColumn(const std::wstring& Name, const std::wstring& Type)
{
	m_ColumnNames.push_back(Name);
	m_ColumnTypes.push_back(Type);
}

void CSQLiteTable::AddRow(const CSQLiteTableRow& Row)
{
	m_Rows.push_back(Row);
}

const CSQLiteTableRow& CSQLiteTable::GetRow(size_t nIndex) const
{
	return m_Rows.at(nIndex);
}
const CSQLiteTableRow& CSQLiteTable::operator[](size_t nIndex) const
{
	return GetRow(nIndex);
}

std::wstring CSQLiteTable::GetName() const
{
	return m_TableName;
}

size_t CSQLiteTable::size() const
{
	return m_Rows.size();
}

void CSQLiteTable::InsertIntoDatabaseWithoutContent(CSQLiteDatabase& Database, bool bDropIfExists) const
{
	if (bDropIfExists)
		Database.DropTableIfExists(GetName());
	Database.CreateTableIfNotExists(GetName(), GenerateColumns());
}

void CSQLiteTable::InsertIntoDatabase(CSQLiteDatabase& Database, bool bDropIfExists) const
{
	InsertIntoDatabaseWithoutContent(Database, bDropIfExists);
	InsertContentIntoDatabase(Database);
}

void CSQLiteTable::InsertContentIntoDatabase(CSQLiteDatabase& Database) const
{
	Database.InsertIntoBatch(GetName(), m_Rows);
}

std::vector<std::wstring> CSQLiteTable::GenerateColumns() const
{
	std::vector<std::wstring> Columns;
	for (size_t i = 0; i < m_ColumnNames.size(); i++)
	{
		Columns.push_back(L"'" + m_ColumnNames[i] + L"' " + m_ColumnTypes[i]);
	}
	return Columns;
}

CSQLiteTable::iterator CSQLiteTable::begin() const
{
	return iterator(*this, 0);
}

CSQLiteTable::iterator CSQLiteTable::end() const
{
	return iterator(*this, size());
}

CSQLiteTable::iterator::iterator(const CSQLiteTable& Table, size_t nIndex)
	:Table(Table)
	, nIndex(nIndex)
{
}

const CSQLiteTableRow& CSQLiteTable::iterator::operator*() const
{
	return Table.GetRow(nIndex);
}

CSQLiteTable::iterator& CSQLiteTable::iterator::operator++()
{
	nIndex++;
	return *this;
}

CSQLiteTable::iterator& CSQLiteTable::iterator::operator++(int)
{
	nIndex++;
	return *this;
}

bool CSQLiteTable::iterator::operator!=(const iterator& rhs) const
{
	return nIndex != rhs.nIndex;
}
bool CSQLiteTable::iterator::operator==(const iterator& rhs) const
{
	return nIndex == rhs.nIndex;
}
ptrdiff_t CSQLiteTable::iterator::operator-(iterator rhs) const
{
	return static_cast<ptrdiff_t>(nIndex) - static_cast<ptrdiff_t>(rhs.nIndex);
}

CSQLiteTableRow CSQLiteTable::GetColumn(size_t nIndex) const
{
	CSQLiteTableRow column;
	for (const auto& row : *this)
		column.AddUnnamed(row.Get<std::wstring>(nIndex));
	return column;
}
std::wstring CSQLiteTable::GetDefaultColumnName(size_t nColumn)
{
	return DefaultColumnName + std::to_wstring(nColumn);
}

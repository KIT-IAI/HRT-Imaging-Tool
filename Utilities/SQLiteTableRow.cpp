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
#include "SQLiteTableRow.h"
#include "StringUtilities.h"
#include "DenseVector.h"
#include "SQLiteTable.h"

using namespace std;

CSQLiteTableRow::CSQLiteTableRow(size_t size)
{
	Values.resize(size);
}

CSQLiteTableRow CSQLiteTableRow::From(const std::vector<std::pair<std::wstring, std::wstring>>& KeyValuePairs)
{
	CSQLiteTableRow row;
	for (const auto& p : KeyValuePairs)
		row.AddKeyValuePair(p.first, p.second);
	return row;
}
CSQLiteTableRow CSQLiteTableRow::From(const vector<wstring>& Values)
{
	CSQLiteTableRow row;
	for (const auto& val : Values)
		row.AddUnnamed(val);
	return row;
}
CSQLiteTableRow CSQLiteTableRow::From(const CAbstractVector& Values)
{
	CSQLiteTableRow row;
	for (const auto& val : Values)
		row.AddUnnamed(val);
	return row;
}

template <> CDenseVector CSQLiteTableRow::Convert() const
{
	CDenseVector vec(Values.size());

	for (size_t i = 0; i < Values.size(); i++)
		vec[i] = Get<double>(i);
	return vec;
}
template <> vector<wstring> CSQLiteTableRow::Convert() const
{
	return Values;
}


size_t CSQLiteTableRow::size() const
{
	return Values.size();
}
std::vector<std::wstring>::const_iterator CSQLiteTableRow::begin() const
{
	return Values.begin();
}
std::vector<std::wstring>::const_iterator CSQLiteTableRow::end() const
{
	return Values.end();
}
std::wstring CSQLiteTableRow::GenerateWildcardStatement(const std::wstring& sTableName) const
{
	if (Values.empty())
		return L"";

	std::wstring sQuery = L"INSERT INTO " + sTableName + L"(!KEYS!) VALUES (!VALUES!)";

	std::wstring QuestionMarks;
	for (size_t i = 0; i < Values.size(); i++)
		QuestionMarks += L", ?";
	QuestionMarks = QuestionMarks.substr(2);

	sQuery = CStringUtilities::ReplaceAll(sQuery, L"!VALUES!", QuestionMarks);

	if (!bUsesKeyValuePairs)
		return CStringUtilities::ReplaceAll(sQuery, L"(!KEYS!)", L"");

	std::wstring KeyString;
	for (size_t i = 0; i < Keys.size(); i++)
		KeyString += L", '" + Keys[i] + L"'";
	KeyString = KeyString.substr(2);

	return CStringUtilities::ReplaceAll(sQuery, L"!KEYS!", KeyString);
}
void CSQLiteTableRow::InsertInto(CSQLiteTable& Table) const
{
	Table.AddRow(*this);
}

